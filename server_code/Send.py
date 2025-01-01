import socket
import base64

server_password_b64 = b'E78fXsCYx1o='
server_password = base64.b64decode(server_password_b64)

def send_data_to_backend(buoy_update_id, air_temp, water_temp, uv_index, humidity, pressure, air_temp2, inner_temp, light_lux, wind_speed, gust_speed, battery_voltage, server_password=server_password):
    server_address = ('backendserver.yeahbuoy.co.nz', 3896)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        sock.connect(server_address)
        
        password_buffer = server_password + b';'

        data_str = f"{buoy_update_id},{air_temp},{water_temp},{uv_index},{humidity},{pressure},{air_temp2},{inner_temp},{light_lux},{wind_speed},{gust_speed},{battery_voltage}"

        message = password_buffer + data_str.encode()

        sock.sendall(message)
        
        time_buffer = sock.recv(7)
        
        print(f"Received time buffer: {time_buffer}")

    except Exception as e:
        print(f"Error sending data to backend: {e}")
    finally:
        sock.close()
