import time
import json
import queue
import threading
import paho.mqtt.client as mqtt

MQTT_BROKER = "broker.hivemq.com"
CMD_TOPIC = "dispenser/test/esp32/open"
STATUS_TOPIC = "dispenser/status/#"
PING_TOPIC = "dispenser/test/esp32/ping"

errorCount = 0
rebootCount = 0
MAX_ERRORS = 5
MAX_REBOOTS = 3

attReady = threading.Event()
dispenserStatus = ""
cmdQueue = queue.Queue()


cmdQueue.put({"pos":0,"count":4})
cmdQueue.put({"pos":2,"count":2})
cmdQueue.put({"pos":1,"count":1})
cmdQueue.put({"pos":3,"count":3})
cmdQueue.put({"pos":0,"count":4})
cmdQueue.put({"pos":3,"count":2})


# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# cmdQueue.put({"pos":9,"count":1})
# #################################
# cmdQueue.put({"pos":0,"count":4})
# cmdQueue.put({"pos":2,"count":2})
# cmdQueue.put({"pos":1,"count":1})
# cmdQueue.put({"pos":3,"count":3})
# cmdQueue.put({"pos":0,"count":4})
# cmdQueue.put({"pos":3,"count":2})
# #################################
# cmdQueue.put({"pos":9,"count":3})
# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# cmdQueue.put({"pos":9,"count":1})
# #################################
# cmdQueue.put({"pos":9,"count":3})
# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# cmdQueue.put({"pos":9,"count":1})
# cmdQueue.put({"pos":9,"count":3})
# #################################
# cmdQueue.put({"pos":0,"count":4})
# cmdQueue.put({"pos":2,"count":2})
# #################################
# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# cmdQueue.put({"pos":9,"count":1})
# cmdQueue.put({"pos":9,"count":3})
# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# cmdQueue.put({"pos":9,"count":1})
# cmdQueue.put({"pos":9,"count":3})
# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# #################################
# cmdQueue.put({"pos":0,"count":4})
# #################################
# cmdQueue.put({"pos":9,"count":1})
# cmdQueue.put({"pos":9,"count":3})
# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# cmdQueue.put({"pos":9,"count":1})
# cmdQueue.put({"pos":9,"count":3})
# cmdQueue.put({"pos":9,"count":4})
# cmdQueue.put({"pos":9,"count":2})
# cmdQueue.put({"pos":9,"count":1})
# cmdQueue.put({"pos":9,"count":3})

def onConnect(client, userdata, flags, rc):
    print("Connected to broker")
    client.subscribe(STATUS_TOPIC)
    client.publish(PING_TOPIC, json.dumps({"CMD":"ping"}))

def onMessage(client, userdata, msg):
    global dispenserStatus, errorCount
    try:
        payload = json.loads(msg.payload.decode())
        print(payload)
        if "att" in payload:
            newStatus = payload["status"]
            if payload["att"] == False:
                print(newStatus)
                attReady.set()
                dispenserStatus = newStatus
            elif payload["att"] == True:
                print("Pausing CMD")
                attReady.clear()
                dispenserStatus = newStatus
                if newStatus.startswith("ERROR"):
                    errorCount += 1
                    print(f"Error count: {errorCount}")
                else:
                    errorCount = 0
        else:
            print("Received message:", payload)
    except json.JSONDecodeError:
        print("Error decoding JSON payload:", msg.payload.decode())

def sendCommand(client):
    cmd = cmdQueue.get()
    print("Sending CMD:", cmd)
    client.publish(CMD_TOPIC, json.dumps(cmd))
    attReady.clear()
    print("Waiting for CMD to complete")

def main():
    global errorCount, rebootCount, dispenserStatus
    client = mqtt.Client()
    client.on_connect = onConnect
    client.on_message = onMessage

    client.connect(MQTT_BROKER, 1883, 60)
    client.loop_start()

    try:
        while not attReady.is_set():
            time.sleep(0.2)
        while True:
            if rebootCount >= MAX_REBOOTS:
                print("Max reboots reached, exiting")
                break
            if errorCount >= MAX_ERRORS:
                print("Rebooting")
                client.publish(PING_TOPIC, json.dumps({"CMD": "reboot", "count": rebootCount}))
                dispenserStatus = "REBOOTING"
                attReady.clear()
                errorCount = 0
                rebootCount += 1
            if not cmdQueue.empty():
                if attReady.is_set():
                    sendCommand(client)
                else:
                    print("Waiting " + dispenserStatus)
            else:
                print("No cmd")
                break
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("Exit")
    finally:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()