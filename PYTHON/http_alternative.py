# esp_utils.py
import requests

def send_data_to_esp8266(esp_address, data):
    """
    Sends data to the ESP8266 via HTTP POST request.

    Parameters:
    esp_address (str): The mDNS or IP address of the ESP8266 (e.g., 'http://espmine.local/data').
    data (str): The data to be sent (e.g., '60').

    Returns:
    response_text (str): The response text from ESP8266.
    """
    try:
        # Send POST request to ESP8266
        response = requests.post(esp_address, data)

        # Check if the request was successful
        if response.status_code == 200:
            print("Data sent successfully!")
            print("ESP Response:", response.text)
            return response.text
        else:
            print(f"Failed to send data. Status code: {response.status_code}")
            return None

    except requests.exceptions.RequestException as e:
        print(f"Error connecting to ESP8266: {e}")
        return None
send_data_to_esp8266('http://espmine.local/data', "hello world")