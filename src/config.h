#ifndef CONFIG_H
#define CONFIG_H

#define HOME_WIFI_SSID "COGECO-222E58"
#define HOME_WIFI_PASS "uw5mmnkj"

#define LAB_WIFI_SSID "SEPT SmartLAB 537"
#define LAB_WIFI_PASS "Factory1"

#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883

#define DISPENSER_ID "dispenser_A"
#define DISPENSE_TOPIC "dispenser/test/esp32/#"
#define STATUS_TOPIC "dispenser/status/esp32/A"

#define STEPS_PER_REV 2048
#define MOTOR_A_PINS {16, 18, 17, 19}
#define MOTOR_B_PINS {27, 25, 26, 13}
//#define SENSOR_PIN_EN 39
#define SENSOR_PIN_OUT 34

enum Position
{
    LEFT_BOTTOM,
    LEFT_TOP,
    RIGHT_BOTTOM,
    RIGHT_TOP,
};

#define ORDER {"ID" : 0, "items" : [ "a", "b", "c", "d" ], "time" : "2025-01-01 00:00:00.0"}

#endif