#include <M5Stack.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "WiFiCredentials.h"

#include "WiFiClient.h"
#include "WiFi.h"

#define NOTE_D0 -1
#define NOTE_D1 294
#define NOTE_D2 330
#define NOTE_D3 350
#define NOTE_D4 393
#define NOTE_D5 441
#define NOTE_D6 495
#define NOTE_D7 556

#define NOTE_DL1 147
#define NOTE_DL2 165
#define NOTE_DL3 175
#define NOTE_DL4 196
#define NOTE_DL5 221
#define NOTE_DL6 248
#define NOTE_DL7 278

#define NOTE_DH1 589
#define NOTE_DH2 661
#define NOTE_DH3 700
#define NOTE_DH4 786
#define NOTE_DH5 882
#define NOTE_DH6 990
#define NOTE_DH7 112

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "192.168.178.27"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "elisensor"
#define AIO_KEY         "Eli14Sens"


/************ Global Variables ******************/
boolean debug = false;
int i = 0;
bool status_door = false;
bool previous_status_door = false;
/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//Adafruit_MQTT_Publish door_status = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/door/door-status");
Adafruit_MQTT_Publish entrance_light = Adafruit_MQTT_Publish(&mqtt, "cmnd/switch-entrance/power");
Adafruit_MQTT_Publish living_light = Adafruit_MQTT_Publish(&mqtt, "cmnd/switch-living/power");

Adafruit_MQTT_Publish lights_mode = Adafruit_MQTT_Publish(&mqtt, "elisensor/feeds/switch-living/mode");



/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void publishMsg(Adafruit_MQTT_Publish topic,const char* Msg){
  if(debug){
    Serial.print(F("\nSending Topic Value "));
    Serial.print(Msg);
    Serial.print("...");
  }
  if (! topic.publish(Msg)) {
    if(debug){Serial.println(F("Failed"));}
  } else {
    if(debug){Serial.println(F("OK!"));}
  }
}

void MQTT_connect() {
  // Function to connect and reconnect as necessary to the MQTT server.
  // Should be called in the loop function and it will take care if connecting.
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  if(debug){Serial.print("Connecting to MQTT... ");}

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       if(debug){Serial.println(mqtt.connectErrorString(ret));}
       if(debug){Serial.println("Retrying MQTT connection in 5 seconds...");}
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  if(debug){Serial.println("MQTT Connected!");}
}


void setup() {
  // Initialize the M5Stack object
  M5.begin();
  
  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();

  M5.Lcd.printf("M5Stack Speaker test:\r\n");

	Serial.begin(115200);
	delay(10);

	if(debug){
		Serial.println(F("Door-Sensor Debug"));
		// Connect to WiFi access point.
		Serial.println(); Serial.println();
		Serial.print("Connecting to ");
		Serial.println(WLAN_SSID);
	}
  
	WiFi.begin(WLAN_SSID, WLAN_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		if(debug){Serial.print(".");}
	}
	if(debug){Serial.println();}

	if(debug){
		Serial.println("WiFi connected");
		Serial.println("IP address: "); Serial.println(WiFi.localIP());
	}



}

void loop() {
  
  MQTT_connect();
  //publishMsg(door_status,"Opened");
  uint8_t status_el = false;
  uint8_t status_ll = false;

  while(1){

    if(M5.BtnA.wasPressed()) {
      M5.Lcd.wakeup();
      M5.Lcd.setBrightness(100);
      M5.Lcd.clearDisplay();
      M5.Lcd.setCursor(0, 0, 2);
      M5.Lcd.printf("Entrance Light Switch \r\n");

      MQTT_connect();
      if(status_el){ publishMsg(entrance_light,"0");}
      else{ publishMsg(entrance_light,"1");}
      status_el = !status_el;
      publishMsg(lights_mode,"Auto");
      delay(2000);
      M5.Lcd.sleep();
      M5.Lcd.setBrightness(0);
      //M5.Speaker.beep(); //beep
    }

    if(M5.BtnB.wasPressed())
    {
      M5.Lcd.wakeup();
      M5.Lcd.setBrightness(100);
      M5.Lcd.clearDisplay();
      M5.Lcd.setCursor(0, 0, 2);
      M5.Lcd.printf("Living Light Switch \r\n");

      MQTT_connect();
      if(status_ll){ publishMsg(living_light,"0");}
      else{ publishMsg(living_light,"1");}
      status_ll = !status_ll;
      publishMsg(lights_mode,"Auto");
      delay(2000);
      M5.Lcd.sleep();
      M5.Lcd.setBrightness(0);
      //M5.Speaker.tone(NOTE_DH2, 200); //frequency 3000, with a duration of 200ms
    }

    if(M5.BtnC.wasPressed())
    {
      M5.Lcd.wakeup();
      M5.Lcd.setBrightness(100);
      M5.Lcd.clearDisplay();
      M5.Lcd.setCursor(0, 0, 2);
      M5.Lcd.printf("Bed Time - All Off \r\n");

      MQTT_connect();
      publishMsg(living_light,"0");
      publishMsg(entrance_light,"0");
      publishMsg(lights_mode,"Auto");
      //M5.Speaker.tone(NOTE_DH2, 200); //frequency 3000, with a duration of 200ms
      delay(2000);
      M5.Lcd.sleep();
      M5.Lcd.setBrightness(0);
    }


    M5.update();

  }
  
  
}
