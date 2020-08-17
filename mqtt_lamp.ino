/*
Publish - LampStat
Subscribe - LampRq
*/
#include <Adafruit_NeoPixel.h>
#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define colorSaturation 128
#define MSG_BUFFER_SIZE  (50)

// Update these with values suitable for your network.
const char* ssid = "JuanKulas";
const char* password = "wifipass0011";
const char* mqtt_server = "192.168.1.30";
const uint16_t PixelCount = 29; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PixelCount, PixelPin, NEO_GRB + NEO_KHZ800);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
long color=0;
String inString = "";

void SetNeoColor(byte* c){  
  int i;
  int n = 0;
  inString = "";
  for(i=0;i<22;i++){
      if((char)c[i]==',') {
        inString += '\n';
        n = inString.toInt();
        color = color << 8;
        color = color | n;
        inString = "";
      }
      else if ((c[i] >= 0x30) && (c[i] <= 0x39)){
        inString += (char)c[i]; 
      }
      else{inString = "";}      
  }
  for(int i=0; i<PixelCount;i++){
    strip.setPixelColor(i, color);
  }  
  Serial.print("Received color: ");
  Serial.print(color, HEX);  
  strip.show();
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if((topic[0]=='L')&&(topic[1]=='a')&&(topic[2]=='m')&&(topic[3]=='p')&&(topic[4]=='R')&&(topic[5]=='q'))
  SetNeoColor(payload);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("LampRq");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println();
  Serial.println("Running...");

  // Neo color at 50
  for(int i=0; i<PixelCount;i++){
  strip.setPixelColor(i, 0x303030);
   }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 15000) {
    lastMsg = now;
    snprintf (msg, MSG_BUFFER_SIZE, "Color hex value:0x%ld", color);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
