#include "config.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <Arduino.h>
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
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error)
    {
        Serial.println("JSON parse error");
        publishStatus(1, "ERROR: JSON parse error");
        return;
    }

    String topicStr = String(topic);

    if (topicStr == PING_TOPIC && doc["CMD"] == "reboot")
    {
        ESP.restart();
    }

/* CMD
#define CMD {"flag" : 0, "pos" : {0, 0, 0, 0}, "count" : {0, 0, 0, 0}}
flag 0 = DISPENSE, 1 = OPEN, 2 = CLOSE, 3 = CW 1 STEP, 4 = CCW 1 STEP
pos = 0 disable 1 enable for each position in order LEFT_BOTTOM, LEFT_TOP, RIGHT_BOTTOM, RIGHT_TOP
count = number for each position in order LEFT_BOTTOM, LEFT_TOP, RIGHT_BOTTOM, RIGHT_TOP
*/
    if (topicStr.endsWith("/open"))
    {
        try
        {
            int flag = doc["flag"] | 0;
            if (flag < 0 || flag > 4)
            {
                Serial.println("Invalid flag");
                publishStatus(1, "ERROR: Invalid flag");
                return;
            }
            bool pos[4] = {
                doc["pos"][0] | 0,
                doc["pos"][1] | 0,
                doc["pos"][2] | 0,
                doc["pos"][3] | 0
            };
            for (int i = 0; i < 4; i++)
            {
                if (pos[i] < 0 || pos[i] > 1)
                {
                    Serial.println("Invalid position");
                    publishStatus(1, "ERROR: Invalid position");
                    return;
                }
            }
            int count[4] = {
                doc["count"][0] | 0,
                doc["count"][1] | 0,
                doc["count"][2] | 0,
                doc["count"][3] | 0
            };
            if (count[0] < 0 && count[1] < 0 && count[2] < 0 && count[3] < 0)
            {
                Serial.println("Invalid count");
                publishStatus(1, "ERROR: Invalid count");
                return;
            }
            Serial.println(doc.as<String>());
            if (!isBusy())
            {
                queueTask(flag, pos, count);
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
    JsonDocument doc;
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