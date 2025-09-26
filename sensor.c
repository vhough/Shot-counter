#include <esp_now.h>
#include <WiFi.h>

#define SENSOR_PIN 16 //Channge this to the right sensor pin



// Replace with receiver's MAC address
uint8_t receiverMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct struct_message {
  bool vibrationDetected;
} struct_message;

struct_message myData;

// Callback for data send
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);

  // Init Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
    int sensorState = digitalRead(SENSOR_PIN); // Read the sensor's state

  // Simple vibration detection threshold
  if (sensorState == HIGH) {
    myData.vibrationDetected = true;
    esp_now_send(receiverMac, (uint8_t *) &myData, sizeof(myData));
    Serial.println("Vibration detected, message sent!");
    delay(200); // debounce
  }
}
