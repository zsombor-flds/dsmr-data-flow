
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>;

#include "secrets.h"

#define DHTPIN D2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); // setup Temp sensor

typedef struct
{
    String name;
    String key; // OBIS code
    int start;  // start index
    int end;    // end index
} Measurement;

// Sanxing SX6x1 SxxU1x datasheet map
String incomingString = "";
const Measurement measurements[] = {
    {"power_timestamp", "0-0:1.0.0", 10, 23},
    {"power_device_name", "0-0:42.0.0", 11, 27},
    {"power_breaker_status", "0-0:96.14.0", 12, 15},
    {"power_consumption", "1-0:15.8.0", 11, 25},
    {"l1_phase_voltage", "1-0:32.7.0", 11, 18},
    {"l2_phase_voltage", "1-0:52.7.0", 11, 18},
    {"l3_phase_voltage", "1-0:72.7.0", 11, 18},
    {"l1_phase_current", "1-0:31.7.0", 11, 16},
    {"l2_phase_current", "1-0:51.7.0", 11, 16},
    {"l3_phase_current", "1-0:71.7.0", 11, 16}};


DynamicJsonDocument doc(4048);
String json;

void setup()
{
    Serial.printf("\n\n\n App Started running\n");

    // Setup Serial connection to Smart Meter
    Serial.begin(115200, SERIAL_8N1, SERIAL_FULL);

    // Wait until serial connection
    do
        delay(250);
    while (!Serial);

    // Start Temp sensor
    dht.begin();

    // Invert RX
    Serial.flush();
    USC0(UART0) = USC0(UART0) | BIT(UCRXI);

    // Setup WIFI connection
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
    Serial.println("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // If serial received, read until newline
    if (Serial.available() > 0)
    {
        incomingString = Serial.readStringUntil('\n');
        handleString(incomingString);
    }

    // hacky way to ensure all data from the meter are valid
    if (doc.size() > 9)
    {

        doc["humidity"] = h;
        doc["temperature"] = t;

        serializeJson(doc, json);

        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;
            HTTPClient http;
            http.begin(client, host);
            http.addHeader("Content-Type", "application/json");

            int httpResponseCode = http.POST(json);

            // Clear up
            http.end();
            doc.clear();
            json = "";
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
    }
}

void handleString(String incomingString)
{
    int i;
    int arraySize = sizeof(measurements) / sizeof(measurements[0]);

    for (i = 0; i < arraySize; i++)
    {
        Measurement measurement = measurements[i];
        String obisKey = measurement.key;

        if (incomingString.indexOf(obisKey) > -1)
        {

            String value = incomingString.substring(measurement.start, measurement.end);
            doc[measurement.name] = value;
        }
    }
}