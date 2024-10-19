#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Servo.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid = "Haha";
const char* password = "@12345678";

// Define the Servo object and pin
Servo myServo;
const int ServoPin = 2; // The GPIO pin the servo is connected to

// Initialize UDP
WiFiUDP Udp;
const unsigned int localUdpPort = 4210;  // Local port to listen on
char incomingPacket[255];  // Buffer for incoming packets

// Global variable to store the desired servo angle
int servoangle = 90;  // Start at the neutral position (90 degrees)
String Direction = "n";  // Initialize to neutral

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

  // Attach the servo to the specified pin
  myServo.attach(ServoPin);
  myServo.write(servoangle);  // Start the servo at initial position

  // Start listening for UDP packets
  Udp.begin(localUdpPort);
  Serial.print("Listening for UDP packets on port ");
  Serial.println(localUdpPort);
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

  // Adjust the servo based on the received direction
  if (Direction.equals("r")) {
    servoangle = servoangle + 1;  // Move right
  } else if (Direction.equals("l")) {
    servoangle = servoangle - 1;  // Move left
  }

  // Constrain servo angle to valid range (0 to 180 degrees)
  servoangle = constrain(servoangle, 0, 180);
  myServo.write(servoangle);  // Update the servo position

  // Print the current servo angle for debugging
  // Serial.println(servoangle);

  // Optional: Add a delay to smooth out servo movements
  delay(500);  // Adjust this delay as necessary
}