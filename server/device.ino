/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#define USE_SERIAL Serial

WiFiMulti wifiMulti;

#define REGISTER_MODE 112
#define READ_MODE 64

int curr_mode = 0;

/*
const char* ca = \ 
"-----BEGIN CERTIFICATE-----\n" \  
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \  
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \  
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \  
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \  
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \  
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \  
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \  
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \  
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \  
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \  
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \  
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \  
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \  
"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \  
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \  
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \  
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \  
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \  
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \  
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \  
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \  
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \  
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \  
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \  
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \  
"-----END CERTIFICATE-----\n";
*/

void setup() {

    USE_SERIAL.begin(115200);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    wifiMulti.addAP("david", "swordfish");

}

void loop() {
  curr_mode = READ_MODE;
  int err_code = 0;
  if(curr_mode == REGISTER_MODE) {
    err_code = register_user();
    Serial.println(err_code);
    
  }else if (curr_mode == READ_MODE) {
     err_code = send_metrics();
    
  }else {
    Serial.println("INVALID MODE");
  }
  
    // wait for WiFi connection
//    if((wifiMulti.run() == WL_CONNECTED)) {
//
//        HTTPClient http;
//
//        USE_SERIAL.print("[HTTP] begin...\n");
//        // configure traged server and url
//        //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
//        http.begin("http://192.168.114.60:5138/"); //HTTP
//
//        USE_SERIAL.print("[HTTP] GET...\n");
//        // start connection and send HTTP header
//        int httpCode = http.GET();
//        
//        // httpCode will be negative on error
//        if(httpCode > 0) {
//            // HTTP header has been send and Server response header has been handled
//            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
//
//            // file found at server
//            if(httpCode == HTTP_CODE_OK) {
//                String payload = http.getString();
//                USE_SERIAL.println(payload);
//            }
//        } else {
//            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//        }
//
//        http.end();
//    }
    delay(10000);
}

int register_user() {
  String id = "";
  // read rfid card
  id = "1223445666"; 
  String payLoad = "{\"id\":\"" + id + "\"}";
  int err_code = send_payload("http://192.168.114.60:5138/esp-register", payLoad);
  return err_code;
}

int send_metrics() {
  String card_uid = "778439";
  String longitude = "12 432 54";
  String latitude = "43 65 7";
  String curr_time = "12:45";


  // Read card and save value as a string into card_uid 

  // Read Gps and save values as "strings" into longitude and latitude

  // Read current time and save into curr_time as a string

  
  String payLoad = "{\"id\":\"" + card_uid + "\", \"long\": \"" + longitude + "\", \"lat\": \"" + latitude + "\", \"curr_time\": \"" + curr_time + "\"}";
  int err_code = send_payload("http://192.168.114.60:5138/esp-metrics", payLoad);
  return err_code; 
}

int send_payload(String url, String payLoad) {
  if (wifiMulti.run() == WL_CONNECTED) {
    HTTPClient http;

    USE_SERIAL.println("[HTTP] Begin POST request...");

    // Your server's URL or IP address
    http.begin(url); 

    // Specify content type header
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    String jsonPayload = payLoad;

    // Send POST request
    int httpResponseCode = http.POST(jsonPayload);

    // Check the response
    if (httpResponseCode > 0) {
        String response = http.getString();  // Get the response payload
        USE_SERIAL.println("[HTTP] Response:");
        USE_SERIAL.println(response);
    } else {
        USE_SERIAL.print("[HTTP] POST... failed, error: ");
        USE_SERIAL.println(http.errorToString(httpResponseCode).c_str());
    }
    
    http.end();  // Close connection
    return httpResponseCode;
      
  } else {
      USE_SERIAL.println("WiFi not connected");
      return 404;
  }
}
