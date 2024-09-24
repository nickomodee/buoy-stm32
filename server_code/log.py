import subprocess
import requests

# The script to execute
script_to_run = 'BCP.py'

# URL of the PHP server
php_url = 'https://yeahbuoy.co.nz/hidden/append_to_file.php'

# Run the script and capture the output
process = subprocess.Popen(['python', script_to_run], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

# Continuously read the output from the script
for line in iter(process.stdout.readline, b''):
    # Decode the output
    output = line.decode('utf-8').strip()
    print(output)

    # Send the output to the PHP server via a POST request
    request = requests.post(php_url, data={'data': output})

# If there are any errors, capture and send them
stderr = process.communicate()[1]
if stderr:
    error_output = stderr.decode('utf-8').strip()
    requests.post(php_url, data={'data': error_output})
