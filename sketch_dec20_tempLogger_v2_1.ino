/***************************************************
  Temperature logger
  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works with Adafruit's Huzzah ESP board:
  ----> https://www.adafruit.com/product/2471

  
  Adapted from code Written by Todd Treece.

  Andrew Wade
  20161220
  Change log:
# Scrubbed network ID and AIO key for public upload. Search for '???' and replace elements with your values there
  
 ****************************************************/
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "Adafruit_IO_Client.h"

// function prototypes
int temp_level();
void wifi_init();
void TriggerEventReport(boolean ReportState);
void blinkLED();

//// Set interval in seconds between log events
#define SLEEP_LENGTH 5


#define WLAN_SSID       "???"
#define WLAN_PASS       "???" // This is not needed for open networks, leave as "" in cases where there is no password

// configure Adafruit IO access
#define AIO_KEY         "???ENTERYOURKEYHERE???"

// create an Adafruit IO client instance
WiFiClient client;
Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);


int TrigCount = 0; 
int TrigLevel = 650; 







void setup() {
  pinMode(0, OUTPUT); // Set up red LED for blinking
  Serial.begin(115200); // Start serial link for debug monitoring
  Serial.println("Temperature logger starting...");
   wifi_init();
   
}

// noop
void loop() {
  temp_level();
  int level = analogRead(A0);
  Serial.println("Data logged, waiting for next point...");
  delay(SLEEP_LENGTH * 1000);

  // Conditional filtering to report a change of state for five consecutive measurments over a value. Daemon checking reported state may miss single true value but will repeatedly report if updated with same value over and over again. Need a single change of state on this latch. 
  if (level >= TrigLevel && TrigCount < 5) {TrigCount ++;}
  else if (level >= TrigLevel && TrigCount == 5) {TrigCount ++; TriggerEventReport(true);}
  else if (level < TrigLevel && TrigCount > 5) {TrigCount = 0; TriggerEventReport(false);}
  else if (level < TrigLevel && TrigCount < 5) {TrigCount = 0;}
}



void TriggerEventReport(boolean ReportState) {
 Serial.print("Reporting state "); Serial.println(ReportState);

   if (WiFi.status() != WL_CONNECTED){
    wifi_init();
  }

  // grab the temperature feed
  Adafruit_IO_Feed coffeebrewtrigger = aio.getFeed("coffeebrewtrigger");

  // send temp level to AIO
  coffeebrewtrigger.send(ReportState);
}
  

// connect to wifi network and establish connection to io.adafruit dashboard
void wifi_init() {
// wifi init
    if (WiFi.status() != WL_CONNECTED) {
  Serial.println("Starting WiFi");
//  WiFi.begin(WLAN_SSID, WLAN_PASS); // Use this if you have a password protected network
  WiFi.begin(WLAN_SSID);
  }

  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.write('.');
    delay(3000);
  }

  // AIO init
  Serial.println("Connecting to Adafruit.io");
  aio.begin();

}


int temp_level() {

  // read the temp level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  int level = analogRead(A0);

  // convert temp level count to eqv. temp float
  level = map(level, 0, 1000,0, 1000);
  Serial.print("Temp level: "); Serial.print(level); Serial.println("C");
  // turn on wifi if we aren't connected
  if (WiFi.status() != WL_CONNECTED){
    wifi_init();
  }

  // grab the temperature feed
  Adafruit_IO_Feed tempSens = aio.getFeed("tempSens");

  // send temp level to AIO
  tempSens.send(level);
  blinkLED();
  return level;
}

float floatmap(long x, long in_min, long in_max, float out_min, float out_max) {
return ((float)x - (float)in_min) * (out_max - out_min) / ((float)in_max - (float)in_min) + out_min;
}

void blinkLED() {
  digitalWrite(0, LOW);
  delay(200);
  digitalWrite(0, HIGH);
}

