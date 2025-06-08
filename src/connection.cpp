#include "config.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "connection.h"

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
    long lastMsg = 0;
    char msg[50];
    int value = 0;

    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
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
    if (client.connected())
    {
        client.publish(STATUS_TOPIC, status.c_str());
    }
    else
    {
        Serial.println("MQTT client not connected, cannot publish status.");
    }
}