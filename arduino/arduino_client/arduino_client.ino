#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

const int Trigger = 12;// connecting to port 12, in board is D6
const int Echo = 14;  // connecting to port 14, in board is D5

const char *ssid = "casa internet"; // Enter your WiFi name
const char *password = "J61H50R2"; // Enter WiFi password


// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port);


WiFiUDP udp;
Coap coap(udp);

//define sound velocity in cm/uS
double duration;
double distanceCm;


void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  Serial.println(p);
}

void setup() {
  // Set software serial baud to 115200;
  pinMode(Trigger, OUTPUT); // Sets the trigPin as an Output
  pinMode(Echo, INPUT); // Sets the echoPin as an Input

  Serial.begin(115200);
  
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  coap.response(callback_response);
  coap.start();
}


void loop() {
    delay(2000);
    
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(Trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigger, LOW);
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(Echo, HIGH);
    
    // Calculate the distance
    distanceCm = duration/58.4;
    Serial.println("La distancia medida es = " + String(distanceCm, 4));
    int msgid = coap.put(IPAddress(192, 168, 100, 6), 5683, "alarm", String(distanceCm, 4).c_str());
    coap.loop();
}