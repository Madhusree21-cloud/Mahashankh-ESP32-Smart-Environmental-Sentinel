#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Preferences.h>
#include "esp_sleep.h"

#define DHTPIN 4
#define DHTTYPE DHT22
#define LED_PIN 18
#define BUZZER_PIN 19

#define SDA_PIN 21
#define SCL_PIN 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const float TEMP_THRESHOLD = 35.0;
const float HUM_THRESHOLD = 80.0;
const unsigned long READ_INTERVAL = 100;
const uint64_t SLEEP_TIME_US = 10ULL * 1000000ULL;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);
Preferences prefs;

unsigned long lastRead = 0;
float tempOffset = 0.0;
float humOffset = 0.0;

void readAndDisplay() {
  float t = dht.readTemperature() + tempOffset;
  float h = dht.readHumidity() + humOffset;

  if (isnan(t) || isnan(h)) {
    Serial.println("Sensor read failed");
    return;
  }

  Serial.printf("Temperature: %.2f C\n", t);
  Serial.printf("Humidity   : %.2f %%\n", h);

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Smart Environment");
  display.printf("Temp: %.1f C\n", t);
  display.printf("Hum : %.1f %%\n", h);
  display.display();

  if (t > TEMP_THRESHOLD) {
  Serial.println("Temperature Alert!");
}

if (h > HUM_THRESHOLD) {
  Serial.println("Humidity Alert!");
}

if (t > TEMP_THRESHOLD || h > HUM_THRESHOLD) {
  digitalWrite(LED_PIN, HIGH);
 ledcWriteTone(BUZZER_PIN, 1000);   // 1 kHz tone
} else {
  digitalWrite(LED_PIN, LOW);
  ledcWriteTone(BUZZER_PIN, 0);      // Stop the tone
}

  prefs.putFloat("lastTemp", t);
  prefs.putFloat("lastHum", h);

  display.display(); // only to allow OLED update
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
  Serial.println("Entering Deep Sleep...");
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  dht.begin();

  prefs.begin("env", false);
  tempOffset = prefs.getFloat("tempOffset", 0.0);
  humOffset = prefs.getFloat("humOffset", 0.0);

  Serial.println("=== Smart Environmental Sentinel ===");
  Serial.printf("Wakeup reason: %d\n", esp_sleep_get_wakeup_cause());

  lastRead = millis();
  ledcAttach(BUZZER_PIN, 1000, 8);   // GPIO 19, 1000 Hz, 8-bit resolution
}

void loop() {
  if (millis() - lastRead >= READ_INTERVAL) {
    lastRead = millis();
    readAndDisplay();
  }
}
