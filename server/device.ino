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

#include <SPI.h>
#include <MFRC522.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>


#define USE_SERIAL Serial

WiFiMulti wifiMulti;

enum MODE {
  REGISTER_MODE,
  READ_MODE
};

volatile MODE curr_mode = READ_MODE;


// GPS setup
TinyGPSPlus gps;
static const int RXPin = 35, TXPin = 34;
static const uint32_t GPSBaud = 9600;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

#define MODE_SELECT_PIN 16
// RFID setup
#define RST_PIN 22
#define SS_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

// Timezone offset in hours (e.g., +1 for UTC+1)
const int timezoneOffset = 1;

// ISR for mode select button
void IRAM_ATTR select_mode() {
  if(curr_mode == REGISTER_MODE) {
    curr_mode = READ_MODE;
    Serial.println("Current Mode: READ");
  }else {
    curr_mode = REGISTER_MODE;
    Serial.println("Current Mode: REGISTER");
  }
}

void setup() {

    USE_SERIAL.begin(115200);
    
    // Start the GPS software serial
    ss.begin(GPSBaud);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    wifiMulti.addAP("david", "swordfish");
    
//    // Configure mode select button pin
    pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
    attachInterrupt(MODE_SELECT_PIN, select_mode, FALLING);
    
    // Initialize SPI bus and RFID reader
    SPI.begin();
    rfid.PCD_Init();

}

void loop() {
  String card_uid = "";
  SPI.begin();
  // Read Gps and save values as "strings" into longitude and latitude
  while (ss.available() > 0) {   // Continuously check for GPS data
    gps.encode(ss.read());
  }
  
  // Check if a new RFID card is present
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // Extract and print the RFID UID
    for (byte i = 0; i < rfid.uid.size; i++) {
      card_uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      card_uid += String(rfid.uid.uidByte[i], HEX);
    }
    
    Serial.println("Card read successfully: " + card_uid + "\n");
    
    // HALT PICC
    rfid.PICC_HaltA();
    
    int err_code = 0;
    // LCD CODE SHOULD BE IN THIS CONDITIONAL STATEMENTS
    if(curr_mode == REGISTER_MODE) {
      err_code = register_user(card_uid);
      Serial.println("Card id sent for registration");
      if(err_code == 200) {
        Serial.println("Card details sent successfully for registration\n\n");
        
      }else {
        Serial.printf("Error [ %d ] Invalid card id was sent to the server (null value)\n\n", err_code);
      }
    }else if (curr_mode == READ_MODE) {
       err_code = send_metrics(card_uid);
       if(err_code == 200) {
         Serial.println("Card id and GPS location sent successfully\n\n");
       }else if(err_code == 404) { // Card declined (user not in the database)
          Serial.println("CARD DECLINED (USER NOT REGISTERED)");
       }else {
         Serial.printf("Error [ %d ] \n\n", err_code);
       }
    }else {
      Serial.println("INVALID MODE");
    }
  }
}

int register_user(String uid) {
  Serial.println("Sending card uid to server...");
  String payLoad = "{\"id\":\"" + uid + "\"}";
  int err_code = send_payload("http://192.168.135.60:5138/esp-register", payLoad);
  return err_code;
}

int send_metrics(String uid) {
  String longitude = "";
  String latitude = "";
  String curr_time = "";

  
  // Check if GPS data is valid
  if (gps.location.isValid()) {
    Serial.println("Formatting GPS data");
    // Adjust time for UTC+1
    int hour = gps.time.hour() + timezoneOffset;
    if (hour >= 24) {
      hour -= 24; // Adjust for next day if it crosses midnight
    } else if (hour < 0) {
      hour += 24; // Adjust for previous day if negative
    }
    
    // Print latitude, longitude, and adjusted time
    longitude = String(gps.location.lat(), 6);
    latitude = String(gps.location.lng(), 6);
    
    // Read current time and save into curr_time as a string
    curr_time = String(hour < 10 ? "0" : ""); // Add leading zero if necessary
    curr_time += String(hour);
    curr_time += ":" + String(gps.time.minute() < 10 ? "0" : ""); // Add leading zero if necessary
    curr_time += String(gps.time.minute());
    curr_time += ":" + String(gps.time.second() < 10 ? "0" : ""); // Add leading zero if necessary
    curr_time += String(gps.time.second());
    
    Serial.println("GPS data read successfully");
  }

  Serial.println("Sending device metrics...");
  String payLoad = "{\"uid\":\"" + uid + "\", \"longitude\": \"" + longitude + "\", \"latitude\": \"" + latitude + "\", \"curr_time\": \"" + curr_time + "\"}";
  int err_code = send_payload("http://192.168.135.60:5138/esp-metrics", payLoad);
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