#include <Time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#include "FirebaseESP32.h"


//Temperature Sensor
#include "DHT.h"
#include <Adafruit_Sensor.h>

#define DHT1PIN 18

DHT dht1(DHT1PIN, DHT11,15);

// WiFi Credentials                    
#define WIFI_SSID "Chiarino"
#define WIFI_PASSWORD "12x43943"

#define FIREBASE_HOST "home-assistant-68bb6.firebaseio.com"                         
#define FIREBASE_AUTH "IrIGJhncAE8CdV2S5sAUC952EcQ6juKD6jmWc6Bu"

FirebaseData firebaseData;
FirebaseJson json1;
FirebaseJson json2;

//onboard LED (BLUE)
#define LED 2


//Swithces and Relays
#define S1 39
#define R1 22

#define S2 34
#define R2 4

#define R3 17

#define R4 15

#define R5 33

#define R6 25

#define R7 26

#define R8 27


//Working with internet or without
int MODE = 0;


//WiFi
void WIFInit(){

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

}

//Firebase
void FirebaseInit(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  

  if(!Firebase.beginStream(firebaseData, "/")){
    Serial.println("Can't begin stream connection...");
    Serial.println("Reason: " + firebaseData.errorReason());
    Serial.println();
  }

  Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);
}

//Call Back 
void streamCallback(StreamData data){

  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  //Print out the value
  //Stream data can be many types which can be determined from function dataType

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
    //Serial.println(data.jsonString());
  if (data.dataType() == "json" && data.dataPath() == "/1/OnOff"){
    {
    Serial.println();
    FirebaseJson &json = data.jsonObject();

    //Print all object data
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;

    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);

      if(value == "true"){

        Serial.println("on");
        digitalWrite(R1, LOW);

      }

      if(value == "false"){

        Serial.println("off");
        digitalWrite(R1, HIGH);

      }


    }
    json.iteratorEnd();
    }
  }

  if (data.dataType() == "json" && data.dataPath() == "/2/OnOff"){
    {
    Serial.println();
    FirebaseJson &json = data.jsonObject();

    //Print all object data
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;

    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);

      if(value == "true"){

        Serial.println("on");
        digitalWrite(R2, LOW);

      }

      if(value == "false"){

        Serial.println("off");
        digitalWrite(R2, HIGH);

      }


    }
    json.iteratorEnd();
    }
  }
  

  
    

}

//Time out
void streamTimeoutCallback(bool timeout){
  if(timeout){
    Serial.println("Stream timeout, resume streaming...");
  }
}


//Control with internet connection.
void with_internet()
{
    //Switch 1
    if (digitalRead(S1) == HIGH)
    {
        if (switch_ON_Flag1_previous_I == 0)
        {
            digitalWrite(R1, LOW);
            //Serial.println("Relay1 - ON");
            

            //LUZ IZQ
            json1.set("on", true);
            Firebase.setJSON(firebaseData, "/1/OnOff", json1);
            

            switch_ON_Flag1_previous_I = 1;

            //led OFF custom character
            lcd.setCursor(3,1);
            lcd.write(1);
        }
            //Serial.println("Switch1 - ON");
    }
    if (digitalRead(S1) == LOW)
    {
        if (switch_ON_Flag1_previous_I == 1)
        {
            digitalWrite(R1, HIGH);
            //Serial.println("Relay1 - OFF");
            
            //Light LL
            json1.set("on", false);
            Firebase.setJSON(firebaseData, "/1/OnOff", json1);
            

            switch_ON_Flag1_previous_I = 0;

            //led ON custom character
            lcd.setCursor(3,1);
            lcd.write(0);
        }
            //Serial.println("Switch1 - OFF");
    }

    //Switch 2
    if (digitalRead(S2) == HIGH)
    {
        if (switch_ON_Flag2_previous_I == 0)
        {
            digitalWrite(R2, LOW);
            //Serial.println("Relay2 - ON");
            
            //Light RT
            json2.set("on", true);
            Firebase.setJSON(firebaseData, "/2/OnOff", json2);

            switch_ON_Flag2_previous_I = 1;

            //led OFF custom character
            lcd.setCursor(3,2);
            lcd.write(1);
        }
            //Serial.println("Switch2 - ON");
    }
    if (digitalRead(S2) == LOW)
    {
        if (switch_ON_Flag2_previous_I == 1)
        {
            digitalWrite(R2, HIGH);
            //Serial.println("Relay2 - OFF");
            
            
            //Light RT
            json2.set("on", false);
            Firebase.setJSON(firebaseData, "/2/OnOff", json2);

            switch_ON_Flag2_previous_I = 0;

            //led ON custom character
            lcd.setCursor(3,2);
            lcd.write(0);
        }
            //Serial.println("Switch2 - OFF");
    }

}

//Control without internet connection.
void without_internet()
{
    //code no internet
}

//Temperature
void temp_sensor() {
  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature();

  Serial.print("Temperature 1: ");
  Serial.print(t1);
  Serial.print(" Humidity 1: ");
  Serial.println(h1);
  delay(20000);
}



void setup()
{
    //Debug console
    Serial.begin(9600);

    //Temperature Sensor
    dht1.begin();

    //Inverter on boot
    digitalWrite(R1, HIGH);
    digitalWrite(R2, HIGH);
    digitalWrite(R3, HIGH);
    digitalWrite(R4, HIGH);
    digitalWrite(R5, HIGH);
    digitalWrite(R6, HIGH);
    digitalWrite(R7, HIGH);
    digitalWrite(R8, HIGH);

    //pinMode(NUMBER, MODE)
    //pinMode(00, INPUT/OUTPUT)
    pinMode(S1, INPUT);
    pinMode(R1, OUTPUT);

    pinMode(S2, INPUT);
    pinMode(R2, OUTPUT);

    pinMode(R3, OUTPUT);

    pinMode(R4, OUTPUT);

    pinMode(R5, OUTPUT);

    pinMode(R6, OUTPUT);

    pinMode(R7, OUTPUT);

    pinMode(R8, OUTPUT);


    //Start Wifi
    WIFInit();


    //Start Firebase
    FirebaseInit();


    //LCD begin on custom pins. 
    //SDA=(GPIO-16) SCL=(GPIO-13) 
    lcd.begin(16, 13);

    //LCD Custom Characters
    lcd.createChar(0, lamp0);
    lcd.createChar(1, lamp1);
    lcd.createChar(2, open_circ);
    lcd.createChar(3, close_circ);
    
    //Mode indicators
    lcd.createChar(4, online);
    lcd.createChar(5, offline);

    //Welcome MSG
    welcome();
    //Menu
    menu();
    //Mode Display
    lcd.setCursor(0,3);   
    lcd.print("Mode");
    


    //Get local time and sets alarm to 11hs
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    //printLocalTime();
    //timer.setInterval(15000L, printLocalTime);

}

void loop()
{
  

  printLocalTime();

  temp_sensor();

  if (WiFi.status() != WL_CONNECTED)
  {

    Serial.println("Not Connected");
    MODE = 1;

  }
  else{

    MODE = 0;

  }




  //Mode detection
  if (MODE == 0){
        
    with_internet();
      

  }else{

    without_internet();

  }

  
}
