import serial
import serial.tools.list_ports
import threading
import sys

def list_serial_ports():
    """
    Lists all available serial ports with their descriptions.
    """
    ports = serial.tools.list_ports.comports()
    port_list = list(ports)

    if not port_list:
        print("No serial ports found.")
    else:
        print("Available Serial Ports:")
        for idx, port in enumerate(port_list, start=1):
            print(f"  {idx}. {port.device} - {port.description}")
    return port_list

def read_from_serial(ser):
    """
    Continuously read data from the serial port and print to terminal.
    """
    try:
        while True:
            if ser.in_waiting:
                data_read = ser.read(ser.in_waiting)
                try:
                    data = data_read.decode('utf-8')
                except UnicodeDecodeError as e:
                    data = data_read
                print(data, end='', flush=True)
    except serial.SerialException:
        print("\nSerial port error. Exiting read thread.")
    except Exception as e:
        print(f"\nUnexpected error in read thread: {e}")

def write_to_serial(ser):
    """
    Continuously read user input from terminal and send to serial port.
    """
    try:
        while True:
            user_input = input()
            if user_input.lower() == 'exit':
                print("Exiting communication...")
                ser.close()
                sys.exit()
            ser.write((user_input + '\n').encode('utf-8'))
    except serial.SerialException:
        print("\nSerial port error. Exiting write thread.")
    except EOFError:
        # Handle end of file (e.g., Ctrl+D)
        print("\nEOF detected. Exiting write thread.")
    except KeyboardInterrupt:
        print("\nKeyboard interrupt detected. Exiting write thread.")
    except Exception as e:
        print(f"\nUnexpected error in write thread: {e}")

def main():
    # List available serial ports
    ports = list_serial_ports()

    if not ports:
        sys.exit(1)

    # Prompt user to select a port
    while True:
        port_input = input("\nEnter the number of the serial port to use (or type the port name directly): ").strip()
        if port_input.isdigit():
            port_index = int(port_input) - 1
            if 0 <= port_index < len(ports):
                selected_port = ports[port_index].device
                break
            else:
                print("Invalid selection. Please enter a valid number from the list.")
        else:
            # Check if the entered port exists
            matching_ports = [port.device for port in ports if port.device.lower() == port_input.lower()]
            if matching_ports:
                selected_port = matching_ports[0]
                break
            else:
                print("Invalid port name. Please choose from the listed ports or enter a valid port name.")

    # Prompt user to enter baud rate
    while True:
        baud_input = input("Enter the baud rate (e.g., 9600, 115200): ").strip()
        if baud_input.isdigit():
            baud_rate = int(baud_input)
            break
        else:
            print("Invalid baud rate. Please enter a numeric value.")

    # Attempt to open the serial port
    try:
        ser = serial.Serial(selected_port, baud_rate, timeout=0)
        print(f"\nConnected to {selected_port} at {baud_rate} baud.")
        print("Type 'exit' to quit.\n")
    except serial.SerialException as e:
        print(f"Failed to connect to {selected_port}: {e}")
        sys.exit(1)

    # Start threads for reading and writing
    read_thread = threading.Thread(target=read_from_serial, args=(ser,), daemon=True)
    write_thread = threading.Thread(target=write_to_serial, args=(ser,), daemon=True)

    read_thread.start()
    write_thread.start()

    # Keep the main thread alive to allow threads to run
    try:
        while True:
            pass
    except KeyboardInterrupt:
        print("\nKeyboard interrupt detected. Exiting...")
    finally:
        if ser.is_open:
            ser.close()
        sys.exit()

if __name__ == "__main__":
    main()