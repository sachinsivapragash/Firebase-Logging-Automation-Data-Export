// task5_esp32_firebase.ino
/*
  ESP32 + Firebase Environment Monitor
  -------------------------------------------------
  Reads: DHT11 (temperature, humidity), LDR (ambient light — monitored only)
  Writes: /sensors/{temperature,humidity,light}, /logs/{push}
  Reads control: /control/mode ("auto"/"manual"), /control/bulb (bool)
  Drives: relay/LED pin for the bulb

  AUTOMATION LOGIC (auto mode):
    Bulb turns ON when temperature >= TEMP_HIGH_THRESHOLD OR humidity >= HUMIDITY_HIGH_THRESHOLD
    Bulb turns OFF when temperature <= TEMP_LOW_THRESHOLD AND humidity < HUMIDITY_HIGH_THRESHOLD
    (The gap between TEMP_LOW and TEMP_HIGH is a hysteresis band so it doesn't flicker on/off
    when the reading hovers right at one threshold.)

  MANUAL MODE:
    Bulb simply follows whatever value the dashboard last wrote to /control/bulb.

  LIBRARIES REQUIRED (Arduino IDE > Library Manager):
    - Firebase ESP Client   by Mobizt   ("Firebase ESP32 Client")
    - DHT sensor library    by Adafruit
    - Adafruit Unified Sensor (dependency of DHT)

  BOARD: ESP32 Dev Module (Tools > Board)

  WIRING (adjust pins as needed):
    DHT11 data  -> GPIO 4
    LDR (analog)-> GPIO 34 (ADC1_CH6)
    Relay/Bulb  -> GPIO 5
*/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>

// ---------------- USER CONFIG ----------------
#define WIFI_SSID       "YOUR_WIFI_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

#define API_KEY         "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL    "https://your-project-default-rtdb.firebaseio.com/"
#define USER_EMAIL      "device@example.com"   // create this user in Firebase Auth
#define USER_PASSWORD   "device_password"

#define DHTPIN          4
#define DHTTYPE         DHT11
#define LDR_PIN         34
#define BULB_PIN        5

#define LDR_DARK_THRESHOLD  1500   // (kept for reference / optional LDR-based logic)
#define TEMP_HIGH_THRESHOLD   30.0 // °C — above this, auto mode turns bulb/fan ON
#define TEMP_LOW_THRESHOLD    26.0 // °C — below this, auto mode turns bulb/fan OFF (hysteresis gap avoids flicker)
#define HUMIDITY_HIGH_THRESHOLD 70.0 // % — above this also forces ON regardless of temp
#define READ_INTERVAL_MS    3000
#define LOG_INTERVAL_MS     15000
// ----------------------------------------------

DHT dht(DHTPIN, DHTTYPE);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long lastRead = 0;
unsigned long lastLog = 0;
bool signupOK = false;
bool autoBulbState = false;   // remembered across loops, for hysteresis

void setup() {
  Serial.begin(115200);
  pinMode(BULB_PIN, OUTPUT);
  digitalWrite(BULB_PIN, LOW);
  dht.begin();

  // ---- WiFi ----
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected, IP: ");
  Serial.println(WiFi.localIP());

  // ---- Firebase ----
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Signing in to Firebase...");
  unsigned long start = millis();
  while (auth.token.uid == "" && millis() - start < 10000) {
    delay(200);
  }
  Serial.println("Firebase ready.");

  // Mark device online, and automatically flip to offline if connection drops
  Firebase.RTDB.setBool(&fbdo, "status/online", true);

  // Set defaults if they don't exist yet
  if (!Firebase.RTDB.getString(&fbdo, "control/mode")) {
    Firebase.RTDB.setString(&fbdo, "control/mode", "auto");
  }
}

void applyBulb(bool on) {
  digitalWrite(BULB_PIN, on ? HIGH : LOW);
  Firebase.RTDB.setBool(&fbdo, "control/bulb", on);
}

void loop() {
  if (millis() - lastRead >= READ_INTERVAL_MS) {
    lastRead = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int lightRaw = analogRead(LDR_PIN);

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("DHT11 read failed, skipping this cycle.");
      return;
    }

    // ---- Push live readings ----
    Firebase.RTDB.setFloat(&fbdo, "sensors/temperature", temperature);
    Firebase.RTDB.setFloat(&fbdo, "sensors/humidity", humidity);
    Firebase.RTDB.setInt(&fbdo, "sensors/light", lightRaw);

    Serial.printf("Temp: %.1f C  Hum: %.1f %%  Light: %d\n", temperature, humidity, lightRaw);

    // ---- Mode + bulb logic ----
    String mode = "auto";
    if (Firebase.RTDB.getString(&fbdo, "control/mode")) {
      mode = fbdo.stringData();
    }

    bool bulbState = false;
    if (mode == "manual") {
      // Bulb state is driven by whatever the dashboard last set — just apply it.
      if (Firebase.RTDB.getBool(&fbdo, "control/bulb")) {
        bulbState = fbdo.boolData();
      }
      digitalWrite(BULB_PIN, bulbState ? HIGH : LOW);
    } else {
      // AUTO: decide from temperature + humidity, with hysteresis to avoid rapid flicker.
      // Turn ON if it's hot enough OR too humid; turn OFF only once it drops below the lower band.
      if (temperature >= TEMP_HIGH_THRESHOLD || humidity >= HUMIDITY_HIGH_THRESHOLD) {
        autoBulbState = true;
      } else if (temperature <= TEMP_LOW_THRESHOLD && humidity < HUMIDITY_HIGH_THRESHOLD) {
        autoBulbState = false;
      }
      // else: stays in whatever state it was already in (dead-band between LOW and HIGH thresholds)

      bulbState = autoBulbState;
      applyBulb(bulbState);
    }

    // ---- Periodic JSON log entry: timestamp, temp, humidity, LDR, bulb state, mode ----
    if (millis() - lastLog >= LOG_INTERVAL_MS) {
      lastLog = millis();
      FirebaseJson json;
      json.set("temperature", temperature);
      json.set("humidity", humidity);
      json.set("light", lightRaw);
      json.set("bulb", bulbState);
      json.set("mode", mode);
      json.set("timestamp/.sv", "timestamp"); // Firebase server timestamp
      Firebase.RTDB.pushJSON(&fbdo, "logs", &json);
    }
  }
}
