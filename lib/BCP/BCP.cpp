#include "BCP.h"

BCP::BCP(uint16_t timeout, uint8_t num_retries, LoRa* lora, void (*data_stream_func)(const char* data, size_t data_size), Encryption* encryption) {
    this->timeout = timeout;
    this->num_retries = num_retries;
    this->lora = lora;
    this->data_stream_func = data_stream_func;
    this->encryption = encryption;
}

PacketStatus BCP::send_packet(uint8_t history_index) {
    if (history_index >= BCP_SENT_HISTORY_SIZE) {
        return PacketStatus::INVALID;
    }

    Packet& packet = this->sent_packet_history[history_index];
    packet.generate_raw();
    const char* packet_data = packet.get_packet();
    const uint8_t packet_size = packet.get_packet_size();
    this->current_index = packet.get_index() + 1; // in some cases on retransmission this is necessary
    DEBUG_BCP_PRINT(F("Sending: "));
    DEBUG_BCP_PRINT((uint8_t)packet.get_type());
    DEBUG_BCP_PRINT(F(", Index: "));
    DEBUG_BCP_PRINTLN(packet.get_index());
    const bool packet_send_status = this->lora->send(packet_data, packet_size);
    DEBUG_BCP_PRINT(F("Packet Send Status: "));
    DEBUG_BCP_PRINTLN(packet_send_status);
    return packet_send_status ? PacketStatus::SUCCESS : PacketStatus::TIMEOUT;
}

Packet* BCP::new_send_packet() {
    for (size_t i = BCP_SENT_HISTORY_SIZE - 1; i > 0; --i) { // we don't want to include 0
        this->sent_packet_history[i] = this->sent_packet_history[i - 1];
    }
    Packet& new_packet = this->sent_packet_history[0];
    new_packet = Packet(this->encryption);
    new_packet.set_index(this->current_index);
    this->current_index++;
    return &new_packet;
}

Packet* BCP::new_recv_packet() {
    for (size_t i = BCP_RECVD_HISTORY_SIZE - 1; i > 0; --i) { // we don't want to include 0
        this->recvd_packet_history[i] = this->recvd_packet_history[i - 1];
    }
    Packet& new_packet = this->recvd_packet_history[0];
    new_packet = Packet(this->encryption);
    this->current_index++;
    return &new_packet;
}

PacketStatus BCP::send_recv(uint8_t num_packets, PacketType expected_packet_type) {
    if (num_packets > BCP_SENT_HISTORY_SIZE) {
        return PacketStatus::INVALID;
    }

    for (uint8_t i = 0; i < this->num_retries; ++i) {
        bool send_failed = false;
        for (int8_t packet_index = num_packets - 1; packet_index >= 0; --packet_index) {
            if (this->send_packet(packet_index) != PacketStatus::SUCCESS) {
                send_failed = true;
                break;
            }
        }
        if (send_failed) {
            continue;
        }
        unsigned long start_time = PAL_MILLISECONDS();
        while (PAL_MILLISECONDS() - start_time < this->timeout) {
            if (this->recv_packet() == PacketStatus::SUCCESS) { // retransmission is not possible, so only check for success
                if (this->recvd_packet_history[0].get_type() != expected_packet_type) {
                    this->reject_recvd_packet();
                    continue;
                }
                return PacketStatus::SUCCESS;
            }
            // don't do anything if it is invalid (the packet is already rejected in `recv_packet()`) or timed-out
        }
    }

    return PacketStatus::TIMEOUT;
}

bool BCP::synchronise() {
    this->current_index = 0;
    Packet* new_packet = this->new_send_packet();
    new_packet->set(PacketType::SYN, -1, 0, "");
    if (this->send_recv(1, PacketType::SYNACK) != PacketStatus::SUCCESS) {
        return false;
    }

    new_packet = this->new_send_packet();
    new_packet->set(PacketType::ACK, -1, 0, "");
    // we don't send this packet because it is sent in `send_data_desc(size_t)` since if we timeout, we must send both, which is easier to implement in that method

    return true;
}

bool BCP::send_data_desc(size_t data_size) {
    Packet* new_packet = this->new_send_packet();
    const uint32_t num_packets = data_size / MAX_DATA_SIZE + (data_size % MAX_DATA_SIZE ? 1 : 0);
    DEBUG_BCP_PRINT(F("Send DATA_DESC, Num packets: "));
    DEBUG_BCP_PRINTLN(num_packets);
    new_packet->set(PacketType::DATA_DESC, -1, sizeof(num_packets), (const char*)&num_packets);
    return this->send_recv(2, PacketType::ACK) == PacketStatus::SUCCESS; // send the ACK from `synchronise()` first, and then the DATA_DESC
}


bool BCP::send_data(const char* data, size_t data_size) {
    const uint32_t num_packets = data_size / MAX_DATA_SIZE + (data_size % MAX_DATA_SIZE ? 1 : 0);
    for (uint32_t i = 0; i < num_packets; ++i) {
        Packet* new_packet = this->new_send_packet();
        new_packet->set(PacketType::DATA, -1, (i == num_packets - 1) ? data_size % MAX_DATA_SIZE : MAX_DATA_SIZE, &data[i * MAX_DATA_SIZE]);
        if (this->send_recv(1, PacketType::ACK) != PacketStatus::SUCCESS) {
            return false;
        }
    }
    return true;
}

bool BCP::recv_data_desc() {
    unsigned long start_time = PAL_MILLISECONDS();
    while (PAL_MILLISECONDS() - start_time < this->num_retries * this->timeout) { // now, transmission control is up to the server, so timeout should be multiplied by num of retries to allow server to send packet in case of packet loss
        const PacketStatus packet_status = this->recv_packet();
        if (packet_status == PacketStatus::SUCCESS) { // retransmission is not possible, so only check for success
            if (this->recvd_packet_history[0].get_type() != PacketType::DATA_DESC) {
                this->reject_recvd_packet();
                continue;
            }
            this->total_msg_packets = ((const uint32_t*)this->recvd_packet_history[0].get_data())[0];
            DEBUG_BCP_PRINT(F("Recv DATA_DESC, Num packets: "));
            DEBUG_BCP_PRINTLN(this->total_msg_packets);

            Packet* new_packet = this->new_send_packet();
            new_packet->set(PacketType::ACK, -1, 0, "");
            return this->send_packet(0) == PacketStatus::SUCCESS;
        }
        // don't do anything if it is invalid (the packet is already rejected in `recv_packet()`) or timed-out
    }
    return false;
}

bool BCP::recv_data() {
    uint32_t i = 0;
    while (i < this->total_msg_packets) {
        DEBUG_BCP_PRINT("Receiving DATA: "); DEBUG_BCP_PRINT(i + 1); DEBUG_BCP_PRINT(" / "); DEBUG_BCP_PRINTLN(this->total_msg_packets);

        bool timed_out = true;
        unsigned long start_time = PAL_MILLISECONDS();
        while (PAL_MILLISECONDS() - start_time < this->num_retries * this->timeout) { // now, transmission control is up to the server, so timeout should be multiplied by num of retries to allow server to send packet in case of packet loss
            const PacketStatus packet_status = this->recv_packet();
            if (packet_status == PacketStatus::RETRANSMISSION) { // if this is a retransmission just resend last sent packet, but we don't break, as we need to keep checking for the expected next packet
                this->send_packet(0); // doesn't matter if this fails
                // do not increment `i` and continue waiting for the expected packet
                continue;
            }

            if (packet_status == PacketStatus::SUCCESS) {
                if (this->recvd_packet_history[0].get_type() == PacketType::DATA) {
                    this->data_stream_func(this->recvd_packet_history[0].get_data(), this->recvd_packet_history[0].get_data_size());

                    Packet* new_packet = this->new_send_packet();
                    new_packet->set(PacketType::ACK, -1, 0, "");

                    if (i == this->total_msg_packets - 1) { // if this is the last data packet, don't send the ACK, as we will do this in `finish()` (similar to how we send the ACK and DATA_DESC in `send_data_desc(size_t)`)
                        return true;
                    }
                    if (this->send_packet(0) != PacketStatus::SUCCESS) {
                        continue;
                    }

                    timed_out = false;
                    i++; // move to the next packet
                    break;
                }

                this->reject_recvd_packet(); // else, reject, and don't increment `i`
            }
            // don't do anything if it is invalid (the packet is already rejected in `recv_packet()`) or timed-out
        }
        if (timed_out) {
            return false;
        }
    }
    return true;
}

bool BCP::finish() {
    Packet* new_packet = this->new_send_packet();
    new_packet->set(PacketType::FIN, -1, 0, "");
    if (this->send_recv(2, PacketType::ACK) != PacketStatus::SUCCESS) { // send the ACK first from `recv_data()` and then the FIN
        return false;
    }

    unsigned long start_time = PAL_MILLISECONDS();
    while (PAL_MILLISECONDS() - start_time < this->num_retries * this->timeout) { // now, transmission control is up to the server, so timeout should be multiplied by num of retries to allow server to send packet in case of packet loss
        if (this->recv_packet() == PacketStatus::SUCCESS) { // retransmission is not possible, so only check for success
            if (this->recvd_packet_history[0].get_type() != PacketType::FIN) {
                this->reject_recvd_packet();
                continue;
            }

            Packet* new_packet = this->new_send_packet();
            new_packet->set(PacketType::ACK, -1, 0, "");
            return this->send_packet(0) == PacketStatus::SUCCESS; // after this ACK, we close the connection, but if this ACK packet is lost, the server will continue to submit FIN in case we didn't receive it, and it will have to timeout (there is nothing we can do to solve this problem)
        }
        // don't do anything if it is invalid (the packet is already rejected in `recv_packet()`) or timed-out
    }
    return false;
}

PacketStatus BCP::recv_packet() {
    unsigned long start_time = PAL_MILLISECONDS();
    while (PAL_MILLISECONDS() - start_time < this->timeout) {
        if (this->lora->recv()) {
            const char* data = this->lora->get_buffer();
            Packet* new_packet = this->new_recv_packet();
            new_packet->set_packet_size(data[PACKET_SIZE_INDEX]);
            new_packet->set_packet(data);
            if (!new_packet->from_raw()) {
                DEBUG_BCP_PRINTLN(F("Packet invalidated, error while decoding from raw bytes"));
                this->reject_recvd_packet();
                continue;
            }
            DEBUG_BCP_PRINT(F("Recvd packet: "));
            DEBUG_BCP_PRINT((uint8_t)new_packet->get_type());
            DEBUG_BCP_PRINT(F(", Index: "));
            DEBUG_BCP_PRINTLN(new_packet->get_index());
            const PacketStatus packet_validation_status = this->validate_recvd_packet();
            if (packet_validation_status == PacketStatus::INVALID) {
                this->reject_recvd_packet();
            }
            return packet_validation_status;
        }
    }

    return PacketStatus::TIMEOUT;
}

static void print_hex(const char* str, size_t str_length) {
    for (size_t i = 0; i < str_length; ++i) {
        DEBUG_BCP_PRINT(F("0x"));
        DEBUG_BCP_PRINT((uint8_t)str[i], PAL_HEX);
        DEBUG_BCP_PRINT(F(" "));
    }
    DEBUG_BCP_PRINTLN();
}

PacketStatus BCP::validate_recvd_packet() {
    if (!this->recvd_packet_history[0].validate_message_checksum() || !this->recvd_packet_history[0].validate_packet_checksum()) {
        DEBUG_BCP_PRINT(F("Packet invalidated, Checksum failed Message checksum success: "));
        DEBUG_BCP_PRINT(this->recvd_packet_history[0].validate_message_checksum());
        DEBUG_BCP_PRINT(F(" Packet checksum success: "));
        DEBUG_BCP_PRINTLN(this->recvd_packet_history[0].validate_packet_checksum());
        print_hex(this->recvd_packet_history[0].get_packet(), this->recvd_packet_history[0].get_packet_size());
        print_hex(this->recvd_packet_history[0].get_data(), this->recvd_packet_history[0].get_data_size());
        return PacketStatus::INVALID;
    }

    if (this->recvd_packet_history[0].get_index() == this->current_index - 1) {
        return PacketStatus::SUCCESS;
    }

    if (this->retransmission_control == RetransmissionController::SERVER && this->recvd_packet_history[0] == this->recvd_packet_history[1] && this->recvd_packet_history[0].get_iv() != this->recvd_packet_history[1].get_iv()) { // probably a duplicate NOT A RETRANSMISSION if the IVs are equal (duplicate meaning it could be reflected off a mountain but arriving at a later time). if server controls retransmission, it may miss our packet, so we may receive an already received packet
        DEBUG_BCP_PRINTLN(F("Retransmission packet received"));
        this->current_index = this->recvd_packet_history[0].get_index() + 1;
        return PacketStatus::RETRANSMISSION;
    }
    DEBUG_BCP_PRINT(F("Packet invalidated, Current index: "));
    DEBUG_BCP_PRINT(this->current_index);
    DEBUG_BCP_PRINT(F(" Packet index: "));
    DEBUG_BCP_PRINT(this->recvd_packet_history[0].get_index());
    DEBUG_BCP_PRINT(F(" Other packet index: "));
    DEBUG_BCP_PRINT(this->recvd_packet_history[1].get_index());
    DEBUG_BCP_PRINT(F(" This packet == other packet: "));
    DEBUG_BCP_PRINT(this->recvd_packet_history[0] == this->recvd_packet_history[1]);
    DEBUG_BCP_PRINT(F(" This packet type: "));
    DEBUG_BCP_PRINT((uint8_t)this->recvd_packet_history[0].get_type());
    DEBUG_BCP_PRINT(F(" Other packet type: "));
    DEBUG_BCP_PRINT((uint8_t)this->recvd_packet_history[1].get_type());
    DEBUG_BCP_PRINT(F(" This packet data size: "));
    DEBUG_BCP_PRINT(this->recvd_packet_history[0].get_data_size());
    DEBUG_BCP_PRINT(F(" Other packet data size: "));
    DEBUG_BCP_PRINT(this->recvd_packet_history[1].get_data_size());
    DEBUG_BCP_PRINT(F(" This packet IV: "));
    DEBUG_BCP_PRINT(this->recvd_packet_history[0].get_iv());
    DEBUG_BCP_PRINT(F(" Other packet IV: "));
    DEBUG_BCP_PRINT(this->recvd_packet_history[1].get_iv());
    DEBUG_BCP_PRINT(F(" This packet data: "));
    DEBUG_BCP_WRITE(this->recvd_packet_history[0].get_data(), this->recvd_packet_history[0].get_data_size());
    DEBUG_BCP_PRINT(F(" Other packet data: "));
    DEBUG_BCP_WRITE(this->recvd_packet_history[1].get_data(), this->recvd_packet_history[1].get_data_size());
    DEBUG_BCP_PRINTLN();
    return PacketStatus::INVALID;
}

void BCP::reject_recvd_packet() {
    for (size_t i = 0; i < BCP_RECVD_HISTORY_SIZE - 1; ++i) {
        this->recvd_packet_history[i] = this->recvd_packet_history[i + 1];
    }
    this->recvd_packet_history[BCP_RECVD_HISTORY_SIZE - 1] = Packet(this->encryption);
    this->current_index--;
    DEBUG_BCP_PRINTLN(F("Packet rejected"));
}

void BCP::set_retransmission_control(RetransmissionController controller) {
    this->retransmission_control = controller;
}

bool BCP::send(const char* data, size_t data_size) {
    const LoRaState saved_state = this->lora->get_state();
    this->lora->set_state(LoRaState::RX); // we do this so that after TX, we immediately revert to RX to catch packets
    this->current_index = 0;
    this->set_retransmission_control(RetransmissionController::BUOY);
    for (uint8_t i = 0; i < this->num_retries; ++i) {
        DEBUG_BCP_PRINTLN(F("Synchronising"));
        if (!this->synchronise()) {
            continue;
        }
        DEBUG_BCP_PRINTLN(F("Sending DATA_DESC"));
        if (!this->send_data_desc(data_size)) {
            continue;
        }
        DEBUG_BCP_PRINTLN(F("Sending data"));
        if (!this->send_data(data, data_size)) {
            continue;
        }
        DEBUG_BCP_PRINTLN(F("Receiving DATA_DESC"));
        if (!this->recv_data_desc()) {
            continue;
        }
        this->set_retransmission_control(RetransmissionController::SERVER);
        DEBUG_BCP_PRINTLN(F("Receiving data"));
        if (!this->recv_data()) {
            continue;
        }
        this->set_retransmission_control(RetransmissionController::BUOY);
        DEBUG_BCP_PRINTLN(F("Finishing"));
        if (!this->finish()) {
            continue;
        }
        DEBUG_BCP_PRINTLN(F("Finished"));
        this->lora->set_state(saved_state);
        return true;
    }
    this->lora->set_state(saved_state);
    return false;
}