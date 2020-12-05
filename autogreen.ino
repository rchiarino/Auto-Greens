// Authors Lucas Alvarez Renzo Chiarino

//Wifi library
#include <WiFi.h>
#include <WiFiClient.h>

#include "FirebaseESP32.h"

//Temperature Sensor
#include "DHT.h"
#include <Adafruit_Sensor.h>

//Definitions
#define DHT1PIN 14

DHT dht1(DHT1PIN, DHT11, 15);

// WiFi Credentials
#define WIFI_SSID "NAME"
#define WIFI_PASSWORD "PASSWORD"

#define FIREBASE_HOST "FIREBASE HOST"
#define FIREBASE_AUTH "FIREBASE AUTH"

FirebaseData firebaseData;

//onboard LED (BLUE)
#define LED 2

#define Pump 13          //digital
#define WaterSensor 12   //analog
#define HumiditySensor 9 //digital

//Status
int pump = false;

//Sensor LOG
float ambientTemp = 0; //ambient Temp from DHT
float ambientHum = 0;  //ambient Temp from DHT
int soilHum = 0;       //soil Humidity 0 dry 1 moist
int water = 0;         //water Level from T1592 P

double soilDelay = 30000; //time to wait before checking the soil sensor //*default 1800
int level = 1800;         // water limit, this triggers the led and app. //*max 1/2 water sensor
int ledTimmer = 90;       //  90 = 30mins
int pumpDelay = 300;      // how long the pump is on

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

    Serial.println();
    Serial.println(data.dataPath());
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

//Soil Sensor
void soil_sensor()
{

    soilHum = digitalRead(HumiditySensor); //read the moisture sensor and save the value
    Serial.print("Soil moisture: ");
    Serial.print(soilHum);
    //Serial.println(" ('1' means soil is too dry and '0' means the soil is moist enough.)");

    if (soilHum == 1)
    {

        digitalWrite(Pump, HIGH);
        Serial.println("pump on");
        delay(pumpDelay);
        digitalWrite(Pump, LOW);
        Serial.println("pump off");
        delay(800);
    }
    digitalWrite(LED, LOW); //TODO: deactivar esto
}

//Water Sensor
void water_sensor()
{
    water = analogRead(WaterSensor); //read the value of the water level sensor
    Serial.print("Water level: ");   //print it to the serial monitor
    Serial.println(water);

    if (water < level)
    { //low watter level
        for (int i = 0; i <= ledTimmer; i++)
        //blinks the LED
        {
            digitalWrite(LED, LOW);
            delay(1000);
            digitalWrite(LED, HIGH);
            delay(1000);
        }
    }
    else
    {
        digitalWrite(LED, HIGH); //TODO: deactivar esto
        delay(soilDelay);        //wait before checking the soil humidity
        soil_sensor();
    }
}

void setup()
{
    //Debug console
    Serial.begin(9600);

    //ONBOARD LED
    pinMode(LED, OUTPUT);

    //pump & snesor
    pinMode(Pump, OUTPUT);
    pinMode(HumiditySensor, INPUT);

    //Temperature Sensor
    dht1.begin();

    //Start Wifi
    WIFInit();

    //Start Firebase
    FirebaseInit();

    //flash the LED five times
    for (int i = 0; i <= 4; i++)
    {
        digitalWrite(LED, HIGH);
        delay(300);
        digitalWrite(LED, LOW);
        delay(300);
    }
    delay(2000);
}

void loop()
{

    temp_sensor();
    water_sensor();
}
