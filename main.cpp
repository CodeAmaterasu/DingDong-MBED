/** Beispiel Abfrage Cloud Dienst Sunrise / Sunset
 */
#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <string>
#include "OLEDDisplay.h"
#include "http_request.h"
#include "MbedJSONValue.h"
#include "MFRC522.h"
#include "Servo.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

#define WIFI_SSID "LERNKUBE"


#define WIFI_PASSWORD "l3rnk4b3"

// UI
OLEDDisplay oled( MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA, MBED_CONF_IOTKIT_OLED_SCL );
MFRC522 rfidReader(MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO, MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS, MBED_CONF_IOTKIT_RFID_RST);
Servo servo1 (MBED_CONF_IOTKIT_SERVO2);

bool isDoorOpen = false;

// Topics for publishing
string authTopic = "iotkit/danilo/auth";
string authResponseTopic = "iotkit/danilo/auth/response";
string registerBadgeTopic = "iotkit/danilo/registerBadge";
string registerBadgeResponseTopic = "iotkit/danilo/registerBadge/response";

// MQTT stuff
string hostname = "cloud.tbz.ch";
int port = 1883;
MQTT::Message message;
int responseStatusCode = 0;


void publish( MQTTNetwork &mqttNetwork, MQTT::Client<MQTTNetwork, Countdown> &client, string topic, string payload )
{
    printf("Publishing");
    MQTT::Message message;    

    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) payload.c_str();
    message.payloadlen = strlen(payload.c_str())+1;
    client.publish( topic.c_str(), message);  
}

/**
 *  
 *  Callback for incoming messages from broker
 *
 */
void messageArrived( MQTT::MessageData& md ) {
    printf("CAllback called");
    message = md.message;
    responseStatusCode = (int) message.payload;
}

int main()
{
    servo1.write(1.0f);
    oled.clear();
    rfidReader.PCD_Init();
    // Connect to the network with the default networking interface
    // if you use WiFi: see mbed_app.json for the credentials
    WiFiInterface* network = WiFiInterface::get_default_instance();
    if (!network) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = network->connect(WIFI_SSID, WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", network->get_mac_address());
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP: %s\n", a.get_ip_address());   

    MQTTNetwork mqttNetwork( network );
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    int rc = mqttNetwork.connect(hostname.c_str(), port);

    client.subscribe("iotkit/danilo/auth/response", MQTT::QOS0, messageArrived);
    client.subscribe(registerBadgeResponseTopic.c_str(), MQTT::QOS0, messageArrived);
    string test = "234320";
    publish(mqttNetwork, client, authTopic, test);
    // Event loop
    while (true) {
        responseStatusCode = 0;
        

        string rfidUid = "";
        char uid[10] = {};
        if (rfidReader.PICC_IsNewCardPresent()) {
            if (rfidReader.PICC_ReadCardSerial()) {
                for (int i = 0; i < rfidReader.uid.size; i++) {
                    uid[i] = rfidReader.uid.uidByte[i];
                    rfidUid.append(to_string(rfidReader.uid.uidByte[i]));
                }
            }
        }

        if (strlen(rfidUid.c_str()) == 0) {
            continue;
        }

        //publish(mqttNetwork, client, authTopic, rfidUid);
        printf("Published");
        do {
            if (responseStatusCode == 200) {
                printf("Auth successfull\n");
                oled.clear();

                if (isDoorOpen) {
                    servo1.write(1.0f);
                    oled.printf("Door is closed");
                    isDoorOpen = false;
                } else {
                    servo1.write(0.0f);
                    oled.printf("Door is opened");
                    isDoorOpen = true;
                }
            } else if (responseStatusCode == 400) {
                printf("Auth failing\n");
                string requestBody = "{ \"BadgeGuid\":\"";
                requestBody.append(rfidUid);
                requestBody.append("\" }");

                publish(mqttNetwork, client, registerBadgeTopic, rfidUid);
                printf("Registering Badge request\n");
                do {
                     if (responseStatusCode == 200) {
                        printf("Card registred\n");
                        oled.clear();
                        oled.printf("Card registred\n");
                    } else {
                        printf("Couldn't register card");
                        oled.printf("Card could not be registred");
                    }
                } while (responseStatusCode == 0);
                
            }

        } while (responseStatusCode == 0);
        responseStatusCode = 0;
    }

    
}
