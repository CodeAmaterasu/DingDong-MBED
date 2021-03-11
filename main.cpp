/** Beispiel Abfrage Cloud Dienst Sunrise / Sunset
 */
#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include "OLEDDisplay.h"
#include "http_request.h"
#include "MbedJSONValue.h"

// UI
OLEDDisplay oled( MBED_CONF_IOTKIT_OLED_RST, MBED_CONF_IOTKIT_OLED_SDA, MBED_CONF_IOTKIT_OLED_SCL );

int main()
{
    oled.clear();
    
    // Connect to the network with the default networking interface
    // if you use WiFi: see mbed_app.json for the credentials
    WiFiInterface* network = WiFiInterface::get_default_instance();
    if (!network) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = network->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }
    printf("Success\n\n");
    printf("MAC: %s\n", network->get_mac_address());
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP: %s\n", a.get_ip_address());    

    while( 1 )
    {
        // By default the body is automatically parsed and stored in a buffer, this is memory heavy.
        // To receive chunked response, pass in a callback as last parameter to the constructor.
        HttpRequest* get_req = new HttpRequest(network, HTTP_GET, "http://38a685d92582.ngrok.io/test");

        const char body[] = "{\"integer\": 123}";

        HttpResponse* get_res = get_req->send();
        // OK
        if ( get_res )
        {
            MbedJSONValue parser;
            // HTTP GET (JSON) parsen  
            parse( parser, get_res->get_body_as_string().c_str() );
            std::string data = parser["message"].get<std::string>();

            oled.printf("%s", data.c_str());
            printf("%s", data.c_str());
            printf(data.c_str());

            /*std::string sunrise;
            std::string sunset;            
            
            sunrise = parser["results"]["sunrise"].get<std::string>();
            sunset  = parser["results"]["sunset"] .get<std::string>(); 
            data = parser["results"]["data"].get<int>();
            
            // Umwandlung nach int. Damit die Zeiten besser verglichen werden können.
            int rh, rm, rs, sh, sm, ss;
            sscanf( sunrise.c_str(), "%d:%d:%d AM", &rh, &rm, &rs );
            sscanf( sunset .c_str(), "%d:%d:%d PM", &sh, &sm, &ss );
            
            oled.cursor( 1, 0 );
            oled.printf( "auf   %02d.%02d.%02d\nunter %02d.%02d.%02d\n", rh+2, rm, rs, sh+2+12, sm, ss );
            */
        }
        // Error
        else
        {
            printf("HttpRequest failed (error code %d)\n", get_req->get_error());
            return 1;
        }
        delete get_req;

        thread_sleep_for( 10000 );
    }
}
