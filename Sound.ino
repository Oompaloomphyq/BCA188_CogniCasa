#include <esp_now.h>
#include <WiFi.h>

// Sound Sensor and LED Pins
int SoundSensor = 27;  // LM393 Sound Sensor Digital Pin D0 connected to pin 4
int LED = 33;          // LED connected to pin 2
boolean LEDStatus = false; 
int clapCount = 0; 
unsigned long lastClapTime = 0; 
unsigned long debounceDelay = 500; 

constexpr char WIFI_SSID[] = "Pixx";
constexpr char WIFI_PASSWORD[] = "44440000";
// MAC Address of the receiver
uint8_t broadcastAddress[] = {0xcc, 0x7b, 0x5c, 0x28, 0x95, 0x2c};

// Structure to send data
typedef struct struct_message3 {
    int id;
    char lightStatus[4];
} struct_message3;

esp_now_peer_info_t peerInfo;

// Create a struct_message3 instance
struct_message3 myData3;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    pinMode(SoundSensor, INPUT);
    pinMode(LED, OUTPUT);

    Serial.begin(115200);

    // Initialize Wi-Fi and ESP-NOW
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
    int SensorData = digitalRead(SoundSensor);

    if (SensorData == HIGH) {
        unsigned long currentTime = millis();

        if (currentTime - lastClapTime > debounceDelay) {
            clapCount = 0;
        }

        clapCount++;
        lastClapTime = currentTime;
        delay(100);

        if (clapCount == 2) {
            if (!LEDStatus) {
                LEDStatus = true;
                digitalWrite(LED, HIGH);
                strcpy(myData3.lightStatus, "Off");
                Serial.println("LED turned OFF");
            } else {
                LEDStatus = false;
                digitalWrite(LED, LOW);
                strcpy(myData3.lightStatus, "On");
                Serial.println("LED turned ON");
            }

            Serial.print("LED Pin State: ");
            Serial.println(digitalRead(LED));

            // Send light status
            myData3.id = 3;  // Unique identifier for this sender
            esp_now_send(broadcastAddress, (uint8_t *)&myData3, sizeof(myData3));

            clapCount = 0;
        }
    }
}
