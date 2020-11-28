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

// // These constants won't change. They're used to give names to the pins used:
// const int ledPin = 2;            // Digital output pin that the LED is attached to
// const int pumpPin = 12;          // Digital output pin that the water pump is attached to
// const int waterLevelPin = A3;    // Analoge pin water level sensor is connected to
// const int moistureSensorPin = 7; // Digital input pin used to check the moisture level of the soil

// // These are the values to edit - see the instructional video to find out what needs adjusting and why:

// double checkInterval = 1800;   //time to wait before checking the soil moisture level - default it to an hour = 1800000
// int waterLevelThreshold = 380; // threshold at which we flash the LED to warn you of a low water level in the pump tank - set this as per the video explains
// int emptyReservoirTimer = 90;  // how long the LED will flash to tell us the water tank needs topping up - default it to 900 = 30mins
// int amountToPump = 300;        // how long the pump should pump water for when the plant needs it

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

        ambientTemp = t1;
        ambientHum = h1;

        //Set Humy on firebase
        Firebase.setInt(firebaseData, "/humedad/TemperatureSetting/thermostatHumidityAmbient", h1);
        //Set Temp on firebase
        Firebase.setInt(firebaseData, "/humedad/TemperatureSetting/thermostatTemperatureAmbient", t1);

        Serial.println("");
        Serial.println(ambientTemp);
        Serial.println(ambientHum);
    }
    delay(120000);
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

    // // put your setup code here, to run once:
    // Serial.begin(9600);
    // pinMode(ledPin, OUTPUT);
    // pinMode(pumpPin, OUTPUT);
    // pinMode(moistureSensorPin, INPUT);

    // //flash the LED five times to confirm power on and operation of code:
    // for (int i = 0; i <= 4; i++)
    // {
    //     digitalWrite(ledPin, HIGH);
    //     delay(300);
    //     digitalWrite(ledPin, LOW);
    //     delay(300);
    // }
    // delay(2000);

    // digitalWrite(ledPin, HIGH); // turn the LED on
}

void loop()
{

    temp_sensor();

    //     // put your main code here, to run repeatedly:

    //     sensorWaterLevelValue = analogRead(waterLevelPin); //read the value of the water level sensor
    //     Serial.print("Water level sensor value: ");        //print it to the serial monitor
    //     Serial.println(sensorWaterLevelValue);

    //     if (sensorWaterLevelValue < waterLevelThreshold)
    //     { //check if we need to alert you to a low water level in the tank
    //         for (int i = 0; i <= emptyReservoirTimer; i++)
    //         {
    //             digitalWrite(ledPin, LOW);
    //             delay(1000);
    //             digitalWrite(ledPin, HIGH);
    //             delay(1000);
    //         }
    //     }
    //     else
    //     {
    //         digitalWrite(ledPin, HIGH);
    //         delay(checkInterval); //wait before checking the soil moisture level
    //     }

    //     // check soil moisture level

    //     moistureSensorValue = digitalRead(moistureSensorPin); //read the moisture sensor and save the value
    //     Serial.print("Soil moisture sensor is currently: ");
    //     Serial.print(moistureSensorValue);
    //     Serial.println(" ('1' means soil is too dry and '0' means the soil is moist enough.)");

    //     if (moistureSensorValue == 1)
    //     {
    //         //pulse the pump
    //         digitalWrite(pumpPin, HIGH);
    //         Serial.println("pump on");
    //         delay(amountToPump); //keep pumping water
    //         digitalWrite(pumpPin, LOW);
    //         Serial.println("pump off");
    //         delay(800); //delay to allow the moisture in the soil to spread through to the sensor
    //     }
}
