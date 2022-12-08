#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "secrets.h"

typedef struct
{
    String name;
    String key; // OBIS code
    int start;  // start index
    int end;    // end index
    enum
    {
        STRING,
        FLOAT,
        INT
    } valueType;

} Measurement;

// Sanxing SX6x1 SxxU1x datasheet map
String incomingString = "";
const Measurement measurements[] = {
    {"power_timestamp", "0-0:1.0.0", 10, 23, Measurement::STRING},
    {"power_device_name", "0-0:42.0.0", 11, 27, Measurement::STRING},
    {"power_breaker_status", "0-0:96.14.0", 12, 15, Measurement::STRING},
    {"power_consumption", "1-0:15.8.0", 11, 25, Measurement::STRING},
    {"l1_phase_voltage", "1-0:32.7.0", 11, 18, Measurement::STRING},
    {"l2_phase_voltage", "1-0:52.7.0", 11, 18, Measurement::STRING},
    {"l3_phase_voltage", "1-0:72.7.0", 11, 18, Measurement::STRING},
    {"l1_phase_current", "1-0:31.7.0", 11, 16, Measurement::STRING},
    {"l2_phase_current", "1-0:51.7.0", 11, 16, Measurement::STRING},
    {"l3_phase_current", "1-0:71.7.0", 11, 16, Measurement::STRING}};

char identifier[24];
int valueState[29];

DynamicJsonDocument doc(4048);
String json;

void setup()
{
    // Setup Serial connection to Smart Meter
    Serial.begin(115200, SERIAL_8N1, SERIAL_FULL);
    do
        delay(250);
    while (!Serial);
    Serial.printf("\n\n\n App Started running\n");

    Serial.flush();
    USC0(UART0) = USC0(UART0) | BIT(UCRXI);
    Serial.println("seri : RX inverted");

    // Setup WIFI connection
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{

    // If serial received, read until newline
    if (Serial.available() > 0)
    {
        incomingString = Serial.readStringUntil('\n');
        handleString(incomingString);
        // Serial.println(incomingString);
    }

    if (doc.size() > 9)
    {
        // serializeJsonPretty(doc, Serial);
        serializeJson(doc, json);

        if (WiFi.status() == WL_CONNECTED)
        {
            WiFiClient client;
            HTTPClient http;
            http.begin(client, host);
            http.addHeader("Content-Type", "application/json");

            int httpResponseCode = http.POST(json);
            //  Serial.print("HTTP Response code: ");
            //  Serial.println(httpResponseCode);
            http.end();

            // Clear up
            doc.clear();
            json = "";
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
    }
}

// Regex are not supported, so use indexOf and substring
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
            switch (measurement.valueType)
            {
            case Measurement::FLOAT:
                value = String(value.toFloat(), 3);
                break;
            case Measurement::INT:
                value = String(value.toInt());
                break;
            default:
                break;
            }
            doc[measurement.name] = value;
        }
    }
}