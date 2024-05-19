#define MQTT_SERVER "mqtt.example.com" 
#define MQTT_PORT 1883              
#define MQTT_USER "yourMQTTUsername"    
#define MQTT_PASSWORD "yourMQTTPassword"
#define MQTT_CLIENT_ID "IrrigationSystem"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Define the Wi-Fi credentials
char ssid[] = "ERROR 404";
char pass[] = "asenhanaotemsenha";

// MQTT topics
#define SOIL_MOISTURE_TOPIC "home/garden/soil_moisture"
#define PUMP_CONTROL_TOPIC "home/garden/pump_control"
#define PUMP_STATUS_TOPIC "home/garden/pump_status"

// Define component pins
#define sensor A0
#define waterPump D3

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
bool Relay = 0;

void setup() {
  Serial.begin(9600);
  pinMode(waterPump, OUTPUT);
  digitalWrite(waterPump, HIGH);

  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    client.subscribe(PUMP_CONTROL_TOPIC);
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == PUMP_CONTROL_TOPIC) {
    if (message == "1") {
      Relay = 1;
      digitalWrite(waterPump, LOW);
    } else {
      Relay = 0;
      digitalWrite(waterPump, HIGH);
    }
    publishPumpStatus();
  }
}

void publishPumpStatus() {
  String status = Relay ? "ON" : "OFF";
  client.publish(PUMP_STATUS_TOPIC, status.c_str());
}

void soilMoistureSensor() {
  int sensorValue = analogRead(sensor);
  sensorValue = map(sensorValue, 0, 1024, 0, 100);
  sensorValue = (sensorValue - 100) * -1;
  snprintf(msg, 50, "%d", sensorValue);
  client.publish(SOIL_MOISTURE_TOPIC, msg);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe(PUMP_CONTROL_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    soilMoistureSensor();
  }
}
