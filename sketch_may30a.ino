#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>

// Wi-Fi settings
const char* ssid     = "M10WiFi";
const char* password = "8326896452";

// TheSportsDB API
const char* apiKey = "123";
const char* teamId = "134862";

TFT_eSPI tft = TFT_eSPI();  // uses config from User_Setup.h

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60000;

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1); // landscape
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connecting to Wi-Fi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.println("Wi-Fi connected");
  delay(1000);
  fetchAndDisplayScore();
}

void loop() {
  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    fetchAndDisplayScore();
  }
}

void fetchAndDisplayScore() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = "https://www.thesportsdb.com/api/v1/json/" + String(apiKey) + "/eventslast.php?id=" + teamId;

  if (https.begin(client, url)) {
    int httpCode = https.GET();
    if (httpCode == 200) {
      String payload = https.getString();

      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        String home = doc["events"][0]["strHomeTeam"] | "N/A";
        String away = doc["events"][0]["strAwayTeam"] | "N/A";
        String homeScore = doc["events"][0]["intHomeScore"] | "N/A";
        String awayScore = doc["events"][0]["intAwayScore"] | "N/A";

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 20);
        tft.println("Last Game:");
        tft.setCursor(10, 60);
        tft.print(home + ": " + homeScore);
        tft.setCursor(10, 90);
        tft.print(away + ": " + awayScore);
      } else {
        showError("JSON Parse Error");
      }
    } else {
      showError("HTTP Error: " + String(httpCode));
    }
    https.end();
  } else {
    showError("HTTPS Fail");
  }
}

void showError(String msg) {
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(10, 60);
  tft.println(msg);
}
