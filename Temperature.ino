#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define BOARD_ID 1
#define SENSOR_PIN 32

constexpr char WIFI_SSID[] = "Pixx";
constexpr char WIFI_PASSWORD[] = "44440000";

OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);

uint8_t broadcastAddress[] = {0xcc, 0x7b, 0x5c, 0x28, 0x95, 0x2c};

typedef struct struct_message {
    int id;
    float temp;
    int readingId;
} struct_message;

esp_now_peer_info_t peerInfo;

struct_message myData;

unsigned long previousMillis = 0;  
const long interval = 1000;  
int readingId =0;


float readDSTemperature() {
    DS18B20.requestTemperatures(); // Send the command to get temperatures
    float temp = DS18B20.getTempCByIndex(0); // Read temperature in °C
    return temp; // Return the temperature value
}



// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    Serial.begin(115200);
    DS18B20.begin();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;

    // Set values to send
    myData.id = BOARD_ID;
    myData.temp = readDSTemperature();
    myData.readingId = readingId++;

    Serial.print("Temperature: ");
    Serial.println(myData.temp);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
  }
}
