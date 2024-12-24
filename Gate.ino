#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <esp_now.h>

// RFID Pins
#define BOARD_ID 2
#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
Servo myServo;                      // Define servo

// Wi-Fi credentials
constexpr char WIFI_SSID[] = "Pixx";
constexpr char WIFI_PASSWORD[] = "44440000";

// MAC Address of the receiver
uint8_t broadcastAddress[] = {0xcc, 0x7b, 0x5c, 0x28, 0x95, 0x2c};

// Structure to send data
typedef struct struct_message2 {
    int id;
    char gateStatus[6];
    int readingId;
} struct_message2;

esp_now_peer_info_t peerInfo;

// Create a struct_message2 instance
struct_message2 myData2;

int readingId = 0;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    Serial.begin(115200);           // Start serial communication
    SPI.begin();                    // Start SPI bus
    mfrc522.PCD_Init();             // Initialize RFID module
    myServo.attach(27);             // Attach servo
    myServo.write(0);              // Servo start position

    Serial.println("Put your card to the reader...");

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
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Show UID on serial monitor
    Serial.print("UID tag : ");
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    content.toUpperCase();

    // Check for authorized UID
    if (content.substring(1) == "05 76 A4 89") { // Replace with your authorized UID
        Serial.println("Authorized access");
        myServo.write(140);  // Open gate
        delay(500);

        // Set and send "open" status
        strcpy(myData2.gateStatus, "Open");
        myData2.id = BOARD_ID;
        myData2.readingId = readingId++;
        esp_now_send(broadcastAddress, (uint8_t *)&myData2, sizeof(myData2));

        delay(5000);          // Keep gate open for 5 seconds
        myServo.write(0);     // Close gate

        // Set and send "close" status
        strcpy(myData2.gateStatus, "Closed");
        myData2.id = BOARD_ID;
        myData2.readingId = readingId++;
        esp_now_send(broadcastAddress, (uint8_t *)&myData2, sizeof(myData2));
    } else {
        Serial.println("Access denied");

        // Set and send "close" status
        strcpy(myData2.gateStatus, "Closed");
        myData2.id = BOARD_ID;
        myData2.readingId = readingId++;
        esp_now_send(broadcastAddress, (uint8_t *)&myData2, sizeof(myData2));
    }
}
