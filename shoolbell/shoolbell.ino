#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include "index_html.h" // Include the HTML file

// WiFi Credentials
const char* ssid = "loki";
const char* password = "12345678";

// Web server & mDNS
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST (UTC+5:30)

// Bell Relay Pin
#define BELL_PIN LED_BUILTIN  // Change if needed (D4 = GPIO2)

// Bell Schedule Storage
#define MAX_SCHEDULES 10
struct BellSchedule {
    int hour;
    int minute;
    bool isPM;  // AM/PM tracking
};
BellSchedule schedules[MAX_SCHEDULES];
int scheduleCount = 0;

void saveSchedules() {
    EEPROM.begin(512);
    EEPROM.put(0, schedules);
    EEPROM.put(100, scheduleCount);
    EEPROM.commit();
}

void loadSchedules() {
    EEPROM.begin(512);
    EEPROM.get(0, schedules);
    EEPROM.get(100, scheduleCount);
}

void ringBell() {
    Serial.println("🔔 Bell is ON");
    digitalWrite(BELL_PIN, LOW);
    delay(5000);  // Increased ringing time
    digitalWrite(BELL_PIN, HIGH);
    Serial.println("🔕 Bell is OFF");
}

void checkBellTime() {
    timeClient.update();
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    bool isPM = (currentHour >= 12);

    for (int i = 0; i < scheduleCount; i++) {
        int scheduledHour = schedules[i].hour;
        if (schedules[i].isPM && scheduledHour < 12) scheduledHour += 12;
        if (!schedules[i].isPM && scheduledHour == 12) scheduledHour = 0;

        if (scheduledHour == currentHour && schedules[i].minute == currentMinute) {
            ringBell();
            delay(60000); // Prevent multiple rings within the same minute
        }
    }
}

// 📡 Handle Root Webpage
void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

// 🕒 Handle Schedule Set
void handleSet() {
    if (server.hasArg("hour") && server.hasArg("minute") && server.hasArg("isPM")) {
        if (scheduleCount < MAX_SCHEDULES) {
            schedules[scheduleCount].hour = server.arg("hour").toInt();
            schedules[scheduleCount].minute = server.arg("minute").toInt();
            schedules[scheduleCount].isPM = (server.arg("isPM") == "true");
            scheduleCount++;
            saveSchedules();
        }
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

// ❌ Handle Schedule Delete
void handleDelete() {
    if (server.hasArg("index")) {
        int index = server.arg("index").toInt();
        if (index < scheduleCount) {
            for (int i = index; i < scheduleCount - 1; i++) {
                schedules[i] = schedules[i + 1];
            }
            scheduleCount--;
            saveSchedules();
        }
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

// 📡 Handle Bell Trigger
void handleRing() {
    ringBell();
    server.sendHeader("Location", "/");
    server.send(303);
}

// 📡 Handle Schedule List
void handleSchedules() {
    String json;
    StaticJsonDocument<512> doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < scheduleCount; i++) {
        JsonObject obj = arr.createNestedObject();
        obj["hour"] = schedules[i].hour;
        obj["minute"] = schedules[i].minute;
        obj["isPM"] = schedules[i].isPM;
    }
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void setup() {
    pinMode(BELL_PIN, OUTPUT);
    digitalWrite(BELL_PIN, HIGH);
    
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Connected to WiFi");

    // 🛜 Start mDNS Service
    if (MDNS.begin("schoolbell")) {
        Serial.println("🎉 mDNS responder started! Access: http://schoolbell.local");
    } else {
        Serial.println("❌ mDNS failed!");
    }

    timeClient.begin();
    loadSchedules();
    
    // Web routes
    server.on("/", handleRoot);
    server.on("/set", HTTP_POST, handleSet);
    server.on("/delete", HTTP_POST, handleDelete);
    server.on("/ring", handleRing);
    server.on("/schedules", handleSchedules);
    
    server.begin();
}

void loop() {
    server.handleClient();
    MDNS.update();
    checkBellTime();
}
