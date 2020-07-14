#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include<WiFiUDP.h>
#define DHTPIN1 D1 
#define DHTPIN2 D2 
#define PUMPPIN D0
// Update these with values suitable for your network.
DHT dht1(DHTPIN1, DHT11), dht2(DHTPIN2, DHT11);
const char* mqtt_server = "192.168.1.22";
WiFiUDP Udp;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  int cnt=0;
  delay(10);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(cnt++ >= 10){
      WiFi.beginSmartConfig();
      while(1){
        delay(1000);
        //Kiểm tra kết nối thành công in thông báo
        if(WiFi.smartConfigDone()){
          Serial.println("SmartConfig Success");
          break;
        }
      }
    }
 
    Serial.println("");
    Serial.println("");
 
    Serial.println("Wait");
 
    // Khởi tạo server UDP tại lắng nghe tại cổng 49999
    Udp.begin(49999);
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
      client.subscribe("/button");
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
  Serial.begin(9600);
  pinMode(PUMPPIN,OUTPUT);
  digitalWrite(PUMPPIN,HIGH);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht1.begin();
  dht2.begin();
}

void loop() {
  char bufH1[100],bufT1[100],bufH2[100],bufT2[100];
  if (!client.connected()) {
    reconnect();
  } 
  
  float h = dht1.readHumidity();
  gcvt(h, 3, bufH1); 
  
  // Read temperature as Celsius (the default)
  float t = dht1.readTemperature();
  gcvt(t, 3, bufT1); 
  float h2 = dht2.readHumidity();
  gcvt(h2, 3, bufH2); 
  // Read temperature as Celsius (the default)
  float t2 = dht2.readTemperature();
  gcvt(t2, 3, bufT2);
  client.publish("/temperature",bufT1);
  client.publish("/humidity",bufH1);
  client.publish("/temperature2",bufT2);
  client.publish("/humidity2",bufH2);
  delay(1000);
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    value++;
    Serial.print("Độ ẩm hiện tại là:");
    Serial.println(h);
    Serial.println(h2);
    Serial.print("Nhiệt độ hiện tại là:");
    Serial.println(t);
    Serial.println(t2);
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  static char message[4];
  strncpy(message, (char *)payload, length);
  message[length] = '\0';
  Serial.printf("topic %s, message received: %s\n", topic, message);

  // decode message
  if (strcmp(message, "off") == 0) {
    digitalWrite(PUMPPIN,HIGH);
  } else if (strcmp(message, "on") == 0) {
    digitalWrite(PUMPPIN,LOW);
  }
 
}
