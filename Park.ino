#include <esp_now.h>
#include <WiFi.h>
#include <NewPing.h>

#define TRIG_PIN 4    // GPIO pin for Ultrasonic Trigger
#define ECHO_PIN 2    // GPIO pin for Ultrasonic Echo
#define MAX_DISTANCE 50 // Maximum distance to measure (in cm)

#define RED_PIN   18 // GPIO pin for Red channel (Base of NPN Transistor)
#define GREEN_PIN 19 // GPIO pin for Green channel (Base of NPN Transistor)
#define BLUE_PIN  21 // GPIO pin for Blue channel (Base of NPN Transistor)

constexpr char WIFI_SSID[] = "Pixx";
constexpr char WIFI_PASSWORD[] = "44440000";

// MAC Address of the receiver
uint8_t broadcastAddress[] = {0xcc, 0x7b, 0x5c, 0x28, 0x95, 0x2c};

// Structure to send data
typedef struct struct_message4 {
    int id;
    char parkStatus[20];
} struct_message4;

esp_now_peer_info_t peerInfo;

// Create a struct_message4 instance
struct_message4 myData4;
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);  // Create NewPing object

// Function to smooth distance readings
float smoothDistance(int numReadings) {
    float total = 0;
    for (int i = 0; i < numReadings; i++) {
        total += sonar.ping_cm();  // Get the distance in cm
        delay(50);  // Small delay between readings for better stability
    }
    return total / numReadings;  // Return the average distance
}

void setLED(int red, int green, int blue) {
  digitalWrite(RED_PIN, red);     // HIGH for ON, LOW for OFF
  digitalWrite(GREEN_PIN, green); // HIGH for ON, LOW for OFF
  digitalWrite(BLUE_PIN, blue);   // HIGH for ON, LOW for OFF
}



void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void turnOffLEDs() {
  setLED(LOW, LOW, LOW);  // Turn off all LEDs
}




void setup() {
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

      pinMode(RED_PIN, OUTPUT);
      pinMode(GREEN_PIN, OUTPUT);
      pinMode(BLUE_PIN, OUTPUT);

  turnOffLEDs(); // Turn off all LEDs initially

    myData4.id = 4;  // Unique identifier for this sender
}

void loop() {
    // Smooth the distance measurement by averaging multiple readings
    float distance = smoothDistance(10);  // Averaging 10 readings for stability

    // Determine parking status based on distance
    if (distance >= 1 && distance <= 15) {
        setLED(HIGH, HIGH, HIGH);
        strcpy(myData4.parkStatus, "Car Parked");
        Serial.println("Car Parked");
    } else if (distance >= 16 && distance <= 22) {
        setLED(LOW, HIGH, HIGH); 
        strcpy(myData4.parkStatus, "Car Approaching");
        Serial.println("Car Approaching");
    } else if (distance >= 23 && distance <=50) {
       setLED(LOW, LOW, LOW);
        strcpy(myData4.parkStatus, "No Car Parked");
        Serial.println("No Car Parked");
    } else {
        turnOffLEDs();
        strcpy(myData4.parkStatus, "No Car Parked");
        Serial.println("No Car Parked");
    }

    // Send parking status via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&myData4, sizeof(myData4));

    delay(2000);  // Delay for better sensor response and stability
}
