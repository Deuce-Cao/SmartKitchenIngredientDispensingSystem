#include "config.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include "connection.h"
#include "dispenser.h"
#include <ArduinoJson.h>

WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

void connectToWiFi()
{
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(HOME_WIFI_SSID, HOME_WIFI_PASS);
    wifiMulti.addAP(LAB_WIFI_SSID, LAB_WIFI_PASS);

    while (wifiMulti.run() != WL_CONNECTED)
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
        publishStatus(1, "ERROR: JSON parse error");
        return;
    }

    String topicStr = String(topic);

    if (topicStr == PING_TOPIC && doc.containsKey("reboot"))
    {
        ESP.restart();
    }

    if (topicStr.endsWith("/open"))
    {
        try
        {
            Position pos = doc["pos"];
            if (pos < 0 || pos > 3)
            {
                Serial.println("Invalid position");
                publishStatus(1, "ERROR: Invalid position");
                return;
            }
            int count = doc["count"];
            if (count <= 0)
            {
                Serial.println("Invalid count");
                publishStatus(1, "ERROR: Invalid count");
                return;
            }
            Serial.println(doc.as<String>());
            if (!isBusy())
            {
                queueTask(pos, count);
            }
            else
            {
                Serial.println("Busy");
                publishStatus(1, "BUSY");
            }
        }
        catch (const std::exception &e)
        {
            Serial.println("Error parsing open command");
            publishStatus(1, "ERROR: Parsing error");
            return;
        }
    }
    else
    {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("]: ");
        Serial.print(doc.as<String>());
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
        }
    }
}

void loopMQTT()
{
    static unsigned long lastReconnect = 0;
    unsigned long now = millis();
    if (!client.connected())
    {
        if (now - lastReconnect > RECONNECT_INTERVAL)
        {
            lastReconnect = now;
            reconnectMQTT();
        }
    }
    client.loop();
}

void publishStatus(bool att, String status)
{
    StaticJsonDocument<200> doc;
    doc["att"] = att;
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