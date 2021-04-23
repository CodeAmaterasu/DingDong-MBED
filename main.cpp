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

#define WIFI_SSID "LERNKUBE"


#define WIFI_PASSWORD "l3rnk4b3"

// UI
OLEDDisplay oled( MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA, MBED_CONF_IOTKIT_OLED_SCL );
MFRC522 rfidReader(MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO, MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS, MBED_CONF_IOTKIT_RFID_RST);
Servo servo1 (MBED_CONF_IOTKIT_SERVO2);

bool isDoorOpen = false;

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
    
    // Event loop
    while (true) {
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

        // By default the body is automatically parsed and stored in a buffer, this is memory heavy.
        // To receive chunked response, pass in a callback as last parameter to the constructor.
        string connectionUrl = "http://192.168.101.19:5000/user?guid=";
        connectionUrl.append(rfidUid);

        if (strlen(rfidUid.c_str()) == 0) {
            continue;
        }

        HttpRequest* get_req = new HttpRequest(network, HTTP_GET, connectionUrl.c_str());

        HttpResponse* get_res = get_req->send();
        // OK
        if ( get_res )
        {
            oled.clear();
            if (get_res->get_status_code() != 200) {
                string requestBody = "{ \"BadgeGuid\":\"";
                requestBody.append(rfidUid);
                requestBody.append("\" }");
                HttpRequest* post_req = new HttpRequest(network, HTTP_POST, "http://192.168.101.19:5000/user/sign");
                post_req->set_header("Content-Type", "application/json");
                HttpResponse* post_res = post_req->send(requestBody.c_str(), strlen(requestBody.c_str()));

                if (post_res) {
                    oled.clear();
                    if (post_res->get_status_code() == 200) {
                        oled.printf("Card registred");
                    } else {
                        oled.printf("Card could not be registred");
                    }
                }

                delete post_req;
            } else {
                oled.clear();
                // TODO: Move Servo
                MbedJSONValue parser;
                if (isDoorOpen) {
                    servo1.write(1.0f);
                    oled.printf("Door is closed");
                    isDoorOpen = false;
                } else {
                    servo1.write(0.0f);
                    oled.printf("Door is opened");
                    isDoorOpen = true;
                }
            }

        } else {
            printf("HttpRequest failed (error code %d)\n", get_req->get_error());
            return 1;
        }
        delete get_req;
        thread_sleep_for(1000);
    }

    
}
