#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "LAPTOP-621";
const char *password = "Tni54321";
const char *mqtt_server = "192.168.137.167";
const int mqtt_port = 1883; // 1883 for TCP
const int soilpin = 4;
// Topic for Publish &Subscribe
const char *mqtt_publish_topic = "esp32/output";
const char *mqtt_subscribe_topic = "esp32/input";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback function when MQTT message received
void callback(char *topic, byte *payload,
              unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // display payload on serial monitor
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe(mqtt_subscribe_topic);
      Serial.print("Subscribed to: ");
      Serial.println(mqtt_subscribe_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  setup_wifi();
  pinMode(soilpin, INPUT);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000)
  { // ทุกๆ 10 วินาที
    lastMsg = now;
    ++value;

    // สุ่มค่าจำลองจากหลายๆ sensor
    int humid = analogRead(soilpin); // ความชื้นสัมพัทธ์

    // สร้าง JSON payload อย่างถูกต้อง
    sprintf(msg, "{\"HUMID\": %d}", humid);

    Serial.print("Publishing JSON to topic '");
    Serial.print(mqtt_publish_topic);
    Serial.print("': ");
    Serial.println(msg);

    client.publish(mqtt_publish_topic, msg);
  }
}