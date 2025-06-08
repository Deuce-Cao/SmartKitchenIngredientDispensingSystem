#include "config.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "connection.h"
#include "dispenser.h"
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

void connectToWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
}

void connectToMQTT()
{
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
    reconnectMQTT();
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error)
    {
        Serial.println("JSON parse error");
        return;
    }

    String topicStr = String(topic);

    if (topicStr.endsWith("/open"))
    {
        int count = doc["open"];
        handleCommand(count);
    }
    else
    {

        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        Serial.print(doc["message"].as<String>());
            Serial.println();
    }
}

void reconnectMQTT()
{
    if (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(DISPENSER_ID))
        {
            Serial.println("connected");
            client.subscribe(DISPENSE_TOPIC);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void loopMQTT()
{
    if (!client.connected())
    {
        reconnectMQTT();
    }
    client.loop();
}

void publishStatus(String status)
{
    StaticJsonDocument<200> doc;
    doc["status"] = status;

    if (client.connected())
    {
        char buffer[256];
        serializeJson(doc, buffer);
        client.publish(STATUS_TOPIC, buffer);
    }
    else
    {
        Serial.println("MQTT client not connected, cannot publish status.");
    }
}