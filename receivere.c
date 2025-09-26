#include <WiFi.h>
#include <esp_now.h>
#include <FastLED.h>

// ----- LED setup -----
#define LED_PIN     12 //Change this to the corresponding pin
#define NUM_LEDS    24 //Change this to the total amount of LED's
#define BRIGHTNESS  255 //Change this to adjust the brightness
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
int currentLed = 0; // Index of next LED to turn off

// ----- Timeout setup -----
unsigned long lastTrueTime = 0;
const unsigned long TIMEOUT_MS = 10000; // Reset time of 10 seconds (can be changed)

// ----- Function to update LED colors based on remaining LEDs -----
void updateLEDs() {
  int remaining = NUM_LEDS - currentLed;

  int oneThird = NUM_LEDS / 3;
  int twoThirds = (2 * NUM_LEDS) / 3;

  CRGB color;

  if (remaining == 0) {
    // all off, handled in blink
    return;
  } else if (remaining < oneThird) {
    color = CRGB::Red;      // less than 1/3 → red
  } else if (remaining < twoThirds) {
    color = CRGB::Orange;   // between 1/3 and 2/3 → orange
  } else {
    color = CRGB::Green;    // more than 2/3 → green
  }

  for (int i = currentLed; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

// ----- Function to blink the entire strip -----
void blinkStrip(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(delayMs);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(delayMs);
  }
}

// ----- Reset LEDs -----
void resetLEDs() {
  currentLed = 0;
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  lastTrueTime = millis(); // reset timeout timer
}

// ----- ESP-NOW callback -----
void OnDataRecv(const esp_now_recv_info* info, const uint8_t* data, int len) {
  if (len > 0 && data[0]) { // vibration detected
    Serial.println("Vibration detected: TRUE");
    lastTrueTime = millis(); // reset timeout

    // Turn off the next LED
    if (currentLed < NUM_LEDS) {
      leds[currentLed] = CRGB::Black;
      currentLed++;
      updateLEDs();
    }

    // If all LEDs are off → blink and restart
    if (currentLed >= NUM_LEDS) {
      blinkStrip(3, 300); // blink 3 times, 300ms each
      resetLEDs();
    }
  }
}

// ----- Setup -----
void setup() {
  Serial.begin(115200);

  // LED init
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  resetLEDs(); // start green

  // ESP-NOW init
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW Receiver ready...");
}

// ----- Loop -----
void loop() {
  // Check timeout for no TRUE received
  if (millis() - lastTrueTime > TIMEOUT_MS) {
    Serial.println("Timeout reached, resetting LEDs");
    resetLEDs();
  }
}
