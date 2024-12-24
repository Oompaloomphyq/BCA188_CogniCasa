#include <WiFi.h>
#include <esp_now.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Pixx";
const char* password = "44440000";

AsyncWebServer server(80);

// Structure for temperature
typedef struct struct_message {
    int id;
    float temp;
    int readingId;
} struct_message;

// Structure for gate status
typedef struct struct_message2 {
    int id;
    char gateStatus[6];
    int readingId;
} struct_message2;

typedef struct struct_message3 {
    int id;
    char lightStatus[4];
} struct_message3;

typedef struct struct_message4 {
    int id;
    char parkStatus[20];
} struct_message4;

// Variables to store received data
struct_message receivedData;
struct_message2 receivedData2;
struct_message3 receivedData3;
struct_message4 receivedData4;

// Callback when data is received
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
    if (len == sizeof(receivedData)) {
        // Handle temperature data
        memcpy(&receivedData, incomingData, sizeof(receivedData));
        Serial.println("<<<<< Received Temperature Data >>>>>");
        Serial.print("ID: ");
        Serial.println(receivedData.id);
        Serial.print("Temperature (°C): ");
        Serial.println(receivedData.temp);
        Serial.print("Reading ID: ");
        Serial.println(receivedData.readingId);
    } else if (len == sizeof(receivedData2)) {
        // Handle gate status data
        memcpy(&receivedData2, incomingData, sizeof(receivedData2));
        Serial.println("<<<<< Received Gate Status Data >>>>>");
        Serial.print("ID: ");
        Serial.println(receivedData2.id);
        Serial.print("Gate Status: ");
        Serial.println(receivedData2.gateStatus);
        Serial.print("Reading ID: ");
        Serial.println(receivedData2.readingId);
    } else if (len == sizeof(receivedData3)) {
        // Handle sound sensor (light status) data
        memcpy(&receivedData3, incomingData, sizeof(receivedData3));
        Serial.println("<<<<< Received Light Status Data >>>>>");
        Serial.print("ID: ");
        Serial.println(receivedData3.id);
        Serial.print("Light Status: ");
        Serial.println(receivedData3.lightStatus);
    } else if (len == sizeof(receivedData4)){
        memcpy(&receivedData4, incomingData, sizeof(receivedData4));
        Serial.println("<<<<< Received Park Status Data >>>>>");
        Serial.print("ID: ");
        Serial.println(receivedData4.id);
        Serial.print("Park Status: ");
        Serial.println(receivedData4.parkStatus);
    } else {
        Serial.println("Unknown data received!");
    }
}

// HTTP handler for gate status
void handleGetGateStatus(AsyncWebServerRequest *request) {
    String jsonResponse = "{ \"gateStatus\": \"" + String(receivedData2.gateStatus) + "\" }";
    request->send(200, "application/json", jsonResponse);
}

// HTTP handler for temperature
void handleGetTemperature(AsyncWebServerRequest *request) {
    String jsonResponse = "{ \"temperature\": " + String(receivedData.temp) + " }";
    request->send(200, "application/json", jsonResponse);
}

void handleGetLightStatus(AsyncWebServerRequest *request) {
    String jsonResponse = "{ \"lightStatus\": \"" + String(receivedData3.lightStatus) + "\" }";
    request->send(200, "application/json", jsonResponse);
}

void handleGetParkStatus(AsyncWebServerRequest *request) {
    String jsonResponse = "{ \"parkStatus\": \"" + String(receivedData4.parkStatus) + "\" }";
    request->send(200, "application/json", jsonResponse);
}


void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.println(WiFi.localIP());

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);

    strcpy(receivedData2.gateStatus, "Closed");
    strcpy(receivedData3.lightStatus, "Off");
    strcpy(receivedData4.parkStatus, "No Car Parked");

    // Define server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String response = generateHTML();
        request->send(200, "text/html", response);
    });

    server.on("/temperature", HTTP_GET, handleGetTemperature);
    server.on("/getGateStatus", HTTP_GET, handleGetGateStatus);
    server.on("/getLightStatus", HTTP_GET, handleGetLightStatus);
    server.on("/park-status", HTTP_GET, handleGetParkStatus);

    server.begin();
}

void loop() {
    // Nothing needed in the loop for this implementation
}

String generateHTML() {
    String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>EcoVilla</title>
      <style>
          * { margin: 0; padding: 0; box-sizing: border-box; }
          body {
              background-color: #35BBCA;
              padding: 0;
              height: 100vh;
              width: 100%;
              background-image: url('https://raw.githubusercontent.com/Oompaloomphyq/BCA188-Final-Project/main/Bg2.png');
              background-size: cover;
              background-repeat: no-repeat;
              background-position: center;
          }
          header { background: #6200ea; color: white; padding: 0; }
          footer { color: white; padding: 0; position: fixed; width: 100%; bottom: 0; }
          .container { position: relative; overflow: hidden; width: 100%; min-height: 100%; display: flex; justify-content: center; align-items: center; padding: 20px; gap: 120px; padding-top: 200px; }
          .info-box { border-radius: 10px; padding: 20px; width: 200px; height: 250px; }
          .info-box h3 { margin: 0 0 10px; }
          .info-box img { display: block; margin: 0 auto; max-width: 100%; height: auto; }
          .box1 { background-color: #8FD5D5; }
          .box2 { background-color: #EDB700; }
          .box3 { background-color: #EEA36B; }
          .box4 { background-color: #F3906B; }
      </style>
      <script>
          function fetchTemperature() {
              fetch('/temperature')
                  .then(response => response.json())
                  .then(data => {
                      document.getElementById('temperature-value').innerText = data.temperature + " °C";
                  })
                  .catch(error => console.error('Error fetching temperature:', error));
          }

          function fetchGateStatus() {
              fetch('/getGateStatus')
                  .then(response => response.json())
                  .then(data => {
                      document.getElementById('gate-status').innerText = data.gateStatus;
                  })
                  .catch(error => console.error('Error fetching gate status:', error));
          }

          function fetchLightStatus() {
            fetch('/getLightStatus')
              .then(response => response.json())
              .then(data => {
                  document.getElementById('light-status').innerText = data.lightStatus;
              })
              .catch(error => console.error('Error fetching light status:', error));
          }
          function fetchParkStatus() {
              fetch('/park-status')
                  .then(response => response.json())
                  .then(data => {
                      document.getElementById('park-status').innerText = data.parkStatus;
                  })
                  .catch(error => console.error('Error fetching park status:', error));
          }


          setInterval(fetchTemperature, 1000);
          setInterval(fetchGateStatus, 100);
          setInterval(fetchLightStatus, 1000);
          setInterval(fetchParkStatus, 1000);
      </script>
  </head>
  <body>
      <div class="container">
          <div class="info-box box1">
              <h3 style="text-align: center;">Gate</h3>
              <img src="https://raw.githubusercontent.com/Oompaloomphyq/BCA188-Final-Project/main/Gate2.png" alt="Gate Image">
              <p style="text-align: center;"><strong> Gate: </strong> <span id="gate-status">Loading...</span></p>
          </div>
          <div class="info-box box2">
              <h3 style="text-align: center;">Parking Space</h3>
              <img src="https://raw.githubusercontent.com/Oompaloomphyq/BCA188-Final-Project/main/Park.png" alt="Parking Image">
              <p style="text-align: center;">Parking: <span id="park-status">Loading...</span></p>
          </div>
          <div class="info-box box3">
              <h3 style="text-align: center;">Room Temperature</h3>
              <img src="https://raw.githubusercontent.com/Oompaloomphyq/BCA188-Final-Project/main/Temp2.png" alt="Temperature Image">
              <p style="text-align: center;"><strong>Temperature:</strong> <span id="temperature-value">Loading...</span></p>
          </div>
          <div class="info-box box4">
              <h3 style="text-align: center;">House Light & Fan</h3>
              <img src="https://raw.githubusercontent.com/Oompaloomphyq/BCA188-Final-Project/main/Villa.png" alt="Villa Image">
              <p style="text-align: center;"><strong>Light: </strong> <span id="light-status">Loading...</span></p>
          </div>
      </div>
  </body>
  </html>
  )rawliteral";
    return html;
}
