import serial
import requests
import time
import sys
from datetime import datetime

def get_current_timestamp():
    """
    Returns the current time formatted as YYYY-MM-DD HH:MM:SS.mmm.
    Example: '2024-04-27 15:45:30.123'
    """
    return datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]  # Truncate to milliseconds

def main():
    # Configuration Parameters
    SERIAL_PORT = 'COM7'          # Replace with your serial port (e.g., 'COM3' on Windows or '/dev/ttyUSB0' on Linux)
    BAUD_RATE = 9600            # Replace with your baud rate
    PHP_URL = 'https://yeahbuoy.co.nz/hidden/append_to_file.php'  # URL of the PHP server

    # Initialize Serial Connection
    try:
        ser = serial.Serial(port=SERIAL_PORT, baudrate=BAUD_RATE, timeout=1)
        print(f"{get_current_timestamp()} - Connected to {SERIAL_PORT} at {BAUD_RATE} baud.")
    except serial.SerialException as e:
        print(f"{get_current_timestamp()} - Error opening serial port {SERIAL_PORT}: {e}")
        sys.exit(1)

    # Allow some time for the serial connection to initialize
    time.sleep(2)

    # Send the command "AT+CRX\r\n" to the serial device
    try:
        command = 'AT+CRX\r\n'
        ser.write(command.encode('utf-8'))
        print(f"{get_current_timestamp()} - Sent command: {command.strip()}")
    except serial.SerialException as e:
        print(f"{get_current_timestamp()} - Error writing to serial port: {e}")
        ser.close()
        sys.exit(1)

    # Create a session for HTTP requests to improve performance
    session = requests.Session()

    print(f"{get_current_timestamp()} - Starting to log incoming data... Press Ctrl+C to exit.")

    try:
        while True:
            try:
                # Read a line from the serial port
                line = ser.readline()

                if line:
                    # Decode the byte data to a string
                    raw_output = line.decode('utf-8', errors='replace').strip()
                    
                    # Get the current timestamp with milliseconds
                    timestamp = get_current_timestamp()
                    
                    # Combine timestamp with the raw output
                    output = f"{timestamp} - {raw_output}"
                    
                    # Print the output to the console
                    print(output)
                    
                    # Send the output to the PHP server via a POST request
                    response = session.post(PHP_URL, data={'data': output})
                    
                    # Optional: Check the response status
                    if response.status_code != 200:
                        print(f"{timestamp} - Failed to send data to server. Status Code: {response.status_code}")
                    
                else:
                    # No data received; you can choose to sleep briefly
                    time.sleep(0.1)

            except serial.SerialException as e:
                timestamp = get_current_timestamp()
                print(f"{timestamp} - Serial communication error: {e}")
                break
            except requests.RequestException as e:
                timestamp = get_current_timestamp()
                print(f"{timestamp} - HTTP request error: {e}")
                # Depending on requirements, you might want to continue or break
                continue

    except KeyboardInterrupt:
        print(f"\n{get_current_timestamp()} - Exiting program...")

    finally:
        # Close the serial connection
        ser.close()
        print(f"{get_current_timestamp()} - Serial connection closed.")

if __name__ == "__main__":
    main()