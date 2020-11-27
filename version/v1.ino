//Wifi library
#include <WiFi.h>
#include <WiFiClient.h>

#include "FirebaseESP32.h"

//Temperature Sensor
#include "DHT.h"
#include <Adafruit_Sensor.h>

//Definitions

#define DHT1PIN 18

DHT dht1(DHT1PIN, DHT11, 15);

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

//Working with internet or without
int MODE = 0;

//Status
int pump = 0;

//Temps LOG
float ambientTemp = 0;
float soilTemp = 0;
float ambientHum = 0;
float soilHum = 0;

int deltaHum = ambientHum - soilHum;

//WiFi
void WIFInit()
{

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
void FirebaseInit()
{
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Firebase.setReadTimeout(firebaseData, 1000 * 60);
    Firebase.setwriteSizeLimit(firebaseData, "tiny");

    if (!Firebase.beginStream(firebaseData, "/"))
    {
        Serial.println("Can't begin stream connection...");
        Serial.println("Reason: " + firebaseData.errorReason());
        Serial.println();
    }

    Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);
}

//Call Back
void streamCallback(StreamData data)
{

    // Serial.println("Stream Data...");
    // Serial.println(data.streamPath());
    // Serial.println(data.dataType());
    Serial.println();
    Serial.println(data.dataPath());

    //Print out the value
    //Stream data can be many types which can be determined from function dataType

    // if (data.dataType() == "int")
    // {
    //     Serial.println(data.intData());
    // }
    // else if (data.dataType() == "float")
    // {
    //     Serial.println(data.floatData(), 5);
    // }
    // else if (data.dataType() == "double")
    // {
    //     printf("%.9lf\n", data.doubleData());
    // }
    // else if (data.dataType() == "boolean")
    // {
    //     Serial.println(data.boolData() == 1 ? "true" : "false");
    // }
    // else if (data.dataType() == "string")
    // {
    //     Serial.println(data.stringData());
    // }
    // else if (data.dataType() == "json")
    // {
    //     //Serial.println(data.jsonString());
    // }

    //!CHECK IF PUMP IS ON OR OF
    if (data.dataPath() == "/bomba/OnOff")
    {

        FirebaseJson &json = data.jsonObject();

        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;

        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value); //i index, value - on off

            if (value == "true")
            {
                Serial.println("on");
                digitalWrite(LED, HIGH);
            }
            else
            {
                Serial.println("off");
                digitalWrite(LED, LOW);
            }
        }
        json.iteratorEnd();
    }
}

//Time out
void streamTimeoutCallback(bool timeout)
{
    if (timeout)
    {
        //TODO: comment this line
        Serial.println("Stream timeout, resume streaming...");
    }
}

//Control with internet connection.
void with_internet()
{
    // Serial.println("with internet");
}

//Control without internet connection.
void without_internet()
{
    //code no internet
}

//Temperature
void temp_sensor()
{
    float h1 = dht1.readHumidity();
    float t1 = dht1.readTemperature();

    if (isnan(h1) || isnan(t1))
    {
        Serial.println("Error DHT11!");
        delay(3000);
        return;
    }
    else
    {
        //blink onboard led
        digitalWrite(LED, LOW);
        delay(100);
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);

        if (ambientTemp != t1 || ambientHum != h1)
        {
            ambientTemp = t1;
            ambientHum = h1;
        }

        //Set Humy on firebase
        Firebase.setInt(firebaseData, "/humedad/TemperatureSetting/thermostatHumidityAmbient", h1);
        //Set Temp on firebase
        Firebase.setInt(firebaseData, "/humedad/TemperatureSetting/thermostatTemperatureAmbient", t1);

        Serial.println("");
        Serial.println(ambientTemp);
        Serial.println(ambientHum);
    }
    delay(60000); //1m //TODO: cambiar a 3m
}

void setup()
{
    //Debug console
    Serial.begin(9600);

    //ONBOARD LED
    pinMode(LED, OUTPUT);

    //Temperature Sensor
    dht1.begin();

    //Start Wifi
    WIFInit();

    //Start Firebase
    FirebaseInit();
}

void loop()
{

    temp_sensor();

    if (WiFi.status() != WL_CONNECTED)
    {

        Serial.println("Not Connected");
        MODE = 1;
    }
    else
    {

        MODE = 0;
    }

    //Mode detection
    if (MODE == 0)
    {

        with_internet();
    }
    else
    {

        without_internet();
    }
}
