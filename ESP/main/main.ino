#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Servo.h>
#include <WiFiUdp.h>
#include <math.h>

// Replace with your network credentials
const char* ssid = "Haha";
const char* password = "@12345678";

// Servo definitions
Servo myServo1;  // First servo
Servo myServo2;  // Second servo
const int ServoPin1 = 2; // GPIO pin for the first servo
const int ServoPin2 = 4; // GPIO pin for the second servo

// Ultrasonic sensor pins
const int trigPin1 = 5;  // First sensor's trig pin
const int echoPin1 = 6;  // First sensor's echo pin
const int trigPin2 = 7;  // Second sensor's trig pin
const int echoPin2 = 8;  // Second sensor's echo pin

// UDP Setup
WiFiUDP Udp;
const unsigned int localUdpPort = 4210;  // Local port to listen on
char incomingPacket[255];  // Buffer for incoming packets

// Global variables for servo control
int servoangle1 = 90;  // Start angle for first servo
String Direction = "n";  // Initialize to neutral

// Parabolic trajectory variables
const float v = 10.0;  // Initial velocity (m/s), you can adjust this value
const float g = 9.8;   // Gravity (m/s^2)

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("wait connecting");
  }

  // Setup MDNS responder
  if (MDNS.begin("espmine")) {
    Serial.println("MDNS responder started");
  } else {
    Serial.println("MDNS failed");
  }

  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Attach the servos to the specified pins
  myServo1.attach(ServoPin1);
  myServo2.attach(ServoPin2);
  myServo1.write(servoangle1);  // Initialize first servo
  myServo2.write(90);           // Initialize second servo

  // Setup the ultrasonic sensors
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // Start listening for UDP packets
  Udp.begin(localUdpPort);
  Serial.print("Listening for UDP packets on port ");
  Serial.println(localUdpPort);
}

// Function to get distance from ultrasonic sensor
float getDistance(int trigPin, int echoPin) {
  // Trigger the ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo time in microseconds
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in cm
  float distance = (duration * 0.034) / 2;
  return distance;
}

// Function to calculate the required angle for the second servo
int calculateLaunchAngle(float distance) {
  // Convert distance to meters
  float R = distance / 100.0;
  
  // Check if the distance is within range for a parabolic shot
  float sin2theta = (R * g) / (v * v);
  
  // Ensure sin2theta is between -1 and 1 to avoid errors
  if (sin2theta > 1 || sin2theta < -1) {
    Serial.println("Target out of range for parabolic trajectory");
    return 90;  // Neutral position
  }
  
  // Calculate the angle (in radians) and convert to degrees
  float theta = asin(sin2theta) / 2.0;
  int angle = (int)(theta * 180.0 / PI);  // Convert from radians to degrees
  
  // Ensure the angle is between 45 and 90 degrees
  angle = constrain(angle, 45, 90);
  
  return angle;
}

void loop() {
  MDNS.update();
  
  // Check if there are incoming UDP packets
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Read the incoming packet
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Null-terminate the string
    }
    
    // Store the received direction
    Direction = String(incomingPacket);
    Serial.println("Direction received: " + Direction);
  }

  // Adjust the first servo based on the received direction
  if (Direction.equals("r")) {
    servoangle1 = servoangle1 + 1;  // Move first servo right
  } else if (Direction.equals("l")) {
    servoangle1 = servoangle1 - 1;  // Move first servo left
  }

  // Constrain first servo angle to valid range (0 to 180 degrees)
  servoangle1 = constrain(servoangle1, 0, 180);
  myServo1.write(servoangle1);  // Update the first servo position

  // Get distance from the second ultrasonic sensor (in cm)
  float distance2 = getDistance(trigPin2, echoPin2);
  Serial.print("Distance from sensor 2: ");
  Serial.print(distance2);
  Serial.println(" cm");

  // Calculate the required angle for the second servo based on distance
  int servoangle2 = calculateLaunchAngle(distance2);
  Serial.print("Calculated angle for parabolic shot: ");
  Serial.println(servoangle2);

  // Set the second servo to the calculated angle
  myServo2.write(servoangle2);

  // Optional: Add a delay to smooth out servo movements
  delay(500);  // Adjust this delay as necessary
}
