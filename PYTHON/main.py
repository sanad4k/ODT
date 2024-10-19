import cv2
import numpy as np
from collections import deque
import socket
import time
esp_address = 'espmine.local'

# Define a deque (a fast way to manage a fixed-size sliding window of values)
direction_history = deque(maxlen=5)  # Store the last 5 directions
frame_count = 0  # Global frame counter for sending data every 10 frames
stable_direction = 'n'  # Global variable to store the stable direction
last_sent_time = 0  # Track the last time data was sent

def filter_direction(new_direction):
    direction_history.append(new_direction)

    # Count occurrences of each direction in the history
    most_common = max(set(direction_history), key=direction_history.count)

    return most_common

def send_data_to_esp8266(esp_address, data):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # i added this time out to fix the mdns errors remove if not found useful
        # sock.settimeout(2)
        sock.sendto(data.encode(), (esp_address, 4210))
        print(f"Sent command: {data}")
    except Exception as e:
        print(f"Error sending data: {e}")
    finally:
        sock.close()

# Function to detect red square and measure its dimensions
def detect_red_square(frame): 
    # Convert the frame to HSV color space
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Define the range of red color in HSV
    lower_red1 = np.array([10, 100, 70])  # Dark Orange to Red (lower hue range)
    upper_red1 = np.array([20, 255, 255])  # Upper boundary for dark orange

    lower_red2 = np.array([0, 100, 70])  # Red lower bound
    upper_red2 = np.array([10, 255, 255])  # Upper boundary for red

    lower_orange_red3 = np.array([170, 100, 70])  # Upper red range (near 180° hue)
    upper_orange_red3 = np.array([180, 255, 255])  # Covering wraparound to 0° for red

    # Create masks to capture red shades
    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
    mask3 = cv2.inRange(hsv, lower_orange_red3, upper_orange_red3)
    # Combine the masks
    mask = mask1 + mask2 + mask3

    # Perform morphological operations to remove small noise
    mask = cv2.erode(mask, None, iterations=1)  # Reduce iterations for faster performance
    mask = cv2.dilate(mask, None, iterations=1)

    # Find contours in the mask
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    square_detected = None
    dimensions = None
    center_x = None

    min_size = 15  # Minimum width and height

    for contour in contours:
        # Approximate the contour
        perimeter = cv2.arcLength(contour, True)
        approx = cv2.approxPolyDP(contour, 0.02 * perimeter, True)

        # Check if the contour has 4 points (could be a square or rectangle)
        if len(approx) == 4:
            x, y, w, h = cv2.boundingRect(approx)

            # Check if the bounding box is close to a square
            aspect_ratio = float(w) / h
            if 0.5 <= aspect_ratio <= 3 and w >= min_size and h >= min_size:
                cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

                square_detected = approx
                dimensions = (w, h)

                center_x, center_y = x + w // 2, y + h // 2
                cv2.circle(frame, (center_x, center_y), 5, (255, 0, 0), -1)

                cv2.putText(frame, f"Width: {w}px, Height: {h}px", (x, y - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

    return frame, dimensions, center_x

# Function to draw center grid lines
def draw_center_grid(frame, color=(0, 255, 0), thickness=1):
    height, width, _ = frame.shape
    cv2.line(frame, (width // 2, 0), (width // 2, height), color, thickness)
    cv2.line(frame, (0, height // 2), (width, height // 2), color, thickness)
    return frame

# Movement direction based on red square position
def movement(width, center_x):
    if center_x is None:
        return 'n'
    direction = 'n'
    if center_x > (340):
        direction = 'r'
    elif (300) > center_x:
        direction = 'l'
    else:
        direction = 'n'
    return direction

def manage_esp_communication(esp_address, stable_direction, last_sent_time):
    current_time = time.time()
    # Send data every 0.5 seconds
    if current_time - last_sent_time > 0.5:
        send_data_to_esp8266(esp_address, stable_direction)
        last_sent_time = current_time
    return last_sent_time

cap = cv2.VideoCapture(0)
esp_address = 'espmine.local'

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame, dimensions, center_x = detect_red_square(frame)

    height, width, _ = frame.shape
    cv2.putText(frame, f"framesize: ({width}, {height})", (150, 150), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

    # Draw center grid lines on the frame
    frame = draw_center_grid(frame, color=(0, 255, 0), thickness=2)

    # Calculate direction and filter for stability
    direction = movement(width, center_x)
    stable_direction = filter_direction(direction)

    # Update frame count and manage communication with ESP8266
    frame_count += 1
    last_sent_time = manage_esp_communication(esp_address, stable_direction, last_sent_time)

    # Display the resulting frame
    cv2.imshow('Red Square Detection with Center Grid', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
