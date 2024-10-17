#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Servo.h>
#include <ESP8266WebServer.h>
const char* ssid = "Haha";
const char* password = "@12345678";
const int ServoPin = 1;
const int relaypin = 5;
ESP8266WebServer server (80);
void setup() {
  // put your setup code here, to run once:
  WiFi.begin(ssid , password);
  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("wait connecting");
    
  }
  if (MDNS.begin("espmine")){
    Serial.println("MDNS responder started");
  }
  else{
    Serial.println("MDNS failed");
  }
  Serial.println("");
  Serial.println("wifi connected");
  Serial.print("Ip: ");
  Serial.println(WiFi.localIP());

  
  server.on("/",HTTP_GET,[](){
    server.send(200,"text/plain","hello u met the esp8266");
  });
  server.on("/data",HTTP_POST,[](){ 
    String data = server.arg("plain");
    Serial.println("the angel recieved is "+data);
    server.send(200,"text/plain","data is recieved and it is "+ data);
  });
    server.on("/relay",HTTP_POST,[](){ 
    String relay_state = server.arg("plain");
    Serial.println("the angel recieved is "+relay_state);
    server.send(200,"text/plain","data is recieved and it is "+ relay_state);
    if(relay_state = 'on'){
      digitalWrite(relaypin , LOW);
    }
    else if(relay_state = 'off'){
      digitalWrite(relaypin , HIGH);
    }
  });
  // if (server.begin()){
  //   Serial.println("http server is up and running baby!");
  // };
  server.begin();
  Serial.println("server is up baby");
  // here we can send on off to control relay if its connected
  
}

void loop() {
  MDNS.update();
  server.handleClient();

}
