import subprocess
import requests
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
    script_to_run = 'BCP.py'  # Replace with the path to your script if necessary
    php_url = 'https://yeahbuoy.co.nz/hidden/append_to_file.php'  # URL of the PHP server

    try:
        # Initialize Subprocess
        process = subprocess.Popen(
            ['python', script_to_run],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,  # Automatically decode bytes to string
            bufsize=1,  # Line-buffered
            universal_newlines=True  # Handle universal newlines
        )
        print(f"{get_current_timestamp()} - Started script '{script_to_run}'.")
    except FileNotFoundError:
        print(f"{get_current_timestamp()} - Error: The script '{script_to_run}' was not found.")
        sys.exit(1)
    except Exception as e:
        print(f"{get_current_timestamp()} - Failed to start script '{script_to_run}': {e}")
        sys.exit(1)

    try:
        # Continuously read the output from the script
        while True:
            # Read line from stdout
            output_line = process.stdout.readline()
            if output_line:
                # Remove any trailing newline characters
                output_line = output_line.rstrip()

                # Get current timestamp
                timestamp = get_current_timestamp()

                # Combine timestamp with the output
                timestamped_output = f"{timestamp} - {output_line}"

                # Print the timestamped output to the console
                print(timestamped_output)

                # Send the timestamped output to the PHP server via a POST request
                try:
                    response = requests.post(php_url, data={'data': timestamped_output})
                    if response.status_code != 200:
                        print(f"{timestamp} - Failed to send data to server. Status Code: {response.status_code}")
                except requests.RequestException as e:
                    print(f"{timestamp} - HTTP request error: {e}")

            else:
                # No more output; check if the process has terminated
                if process.poll() is not None:
                    break

        # Capture any remaining stderr output
        stderr_output = process.stderr.read()
        if stderr_output:
            # Split stderr into individual lines
            for error_line in stderr_output.splitlines():
                error_line = error_line.rstrip()

                # Get current timestamp
                timestamp = get_current_timestamp()

                # Combine timestamp with the error output
                timestamped_error = f"{timestamp} - ERROR: {error_line}"

                # Print the timestamped error to the console
                print(timestamped_error)

                # Send the timestamped error to the PHP server via a POST request
                try:
                    response = requests.post(php_url, data={'data': timestamped_error})
                    if response.status_code != 200:
                        print(f"{timestamp} - Failed to send error to server. Status Code: {response.status_code}")
                except requests.RequestException as e:
                    print(f"{timestamp} - HTTP request error while sending error: {e}")

    except KeyboardInterrupt:
        print(f"\n{get_current_timestamp()} - Received KeyboardInterrupt. Terminating subprocess...")
        process.terminate()
    except Exception as e:
        print(f"{get_current_timestamp()} - An unexpected error occurred: {e}")
        process.terminate()
    finally:
        # Ensure the subprocess is terminated
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            print(f"{get_current_timestamp()} - Subprocess did not terminate in time. Killing it...")
            process.kill()
        print(f"{get_current_timestamp()} - Subprocess terminated.")

if __name__ == "__main__":
    main()