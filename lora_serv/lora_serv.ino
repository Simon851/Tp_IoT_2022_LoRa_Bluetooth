#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>
#include <SPI.h>
// #include <U8x8lib.h>

#define ssid "nope"
#define pass "jejejejeje"
#define SCK 5
#define MISO  19
#define MOSI  27
#define SS  18
#define RST 14
#define DI0 26

// U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15,4,16); // clock, data, reset

WiFiClient espClient;
PubSubClient client(espClient);

// MQTT Stuff
const byte LIGHT_PIN = 25;
const char *ID = "simcom_serv";
const char *TOPIC = "/info/youpi";
const char *STATE_TOPIC = "/info/youpi/status";
// const char* mqttServer = "5.196.95.208"; // test.mosquitto.org
const char* mqttServer = "test.mosquitto.org";

// LoRa Stuff
const int loRaFrequency = 870E6;
const int spreadingFactor = 8;
const int signalBandwidth = 100E3;

char values[100];

/* Not functional
void dispData(char* td)
{
  u8x8.clear();
  u8x8.drawString(0,2,td);
}
*/

void setup() {
    Serial.begin(9600) ;
    while (!Serial);

    // Connecting to WiFi
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to the WiFi network");

    // Connecting to MQTT
    client.setServer(mqttServer, 1883);
    while (!client.connected()) {
        Serial.println("Connecting to MQTT...");
        if (client.connect(ID)) {
            Serial.println("connected");
        } else {
            Serial.print("failed with state");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // client.publish("esp/test", "Hello from ESP32");
    // Serial.println("published");

    // Setting up LoRa
    LoRa.setFrequency(loRaFrequency);
    LoRa.setSpreadingFactor(spreadingFactor);
    LoRa.setSignalBandwidth(signalBandwidth);
    pinMode(DI0, INPUT);
    SPI.begin(SCK,MISO,MOSI,SS);
    LoRa.setPins(SS,RST,DI0);
    LoRa.onReceive(onReceive);

    // Dans l'ordre : Fréquence, Facteur d'étalement du spectre, nombre de modulation puis bande passante
    sprintf(values, "%u,%u,%u", loRaFrequency, spreadingFactor, signalBandwidth);

    if (!LoRa.begin(loRaFrequency)) {
      Serial.println("Starting LoRa failed :(");
      exit(1);
    }

    LoRa.receive();
}

// Paquet d'émission
union pack
{
  uint8_t frame[16];
  float data[4];
} sdp;

void onReceive(int packetSize) {
  // Received a packet
   Serial.print("Received packet '");

  // read packet
  char endc[500];
  for (int i = 0; i < packetSize; i++) {
    sprintf(endc,"%s%c",endc,(char)LoRa.read());
  }
  Serial.print(endc);
//  dispData(endc);

  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}

void loop() {
  client.publish(TOPIC, values);
  delay(10000);
}
