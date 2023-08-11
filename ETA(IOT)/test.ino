#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>
#define BLYNK_PRINT Serial
#include "WiFi.h"


#define BLYNK_TEMPLATE_ID "TMPL3aiEksDHh"
#define BLYNK_TEMPLATE_NAME "ETA"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

#define VIRTUAL_PIN_PEOPLE 0
#define VIRTUAL_PIN_STOP_NUMBER 1
#define VIRTUAL_PIN_WAITING_PEOPLE_2 2
#define VIRTUAL_PIN_WAITING_PEOPLE_3 3


//blynk
char auth[] = "iDX0zEhM5RlogLTHzLFNteGXJT46W8Ri";  //Enter your Blynk Auth token
char ssid[] = "Preethi";  //Enter your WIFI SSID
char pass[] = "bat1234567";  //Enter your WIFI Password

BlynkTimer timer;


#define SENSOR_PIN_1 14   // Pin number of the slot sensor
#define IR_SENSOR_PIN_1 13 // Pin number of the IR sensor

#define SENSOR_PIN_2 5  // Pin number of the slot sensor
#define IR_SENSOR_PIN_2 4 // Pin number of the IR sensor

#define SENSOR_PIN_3 19  // Pin number of the slot sensor
#define IR_SENSOR_PIN_3 15 // Pin number of the IR sensor

#define LED_PIN 12        // Pin number of the LED

int count = 0;           // Current count of passengers on the bus
int limit = 3;           // Limit of passengers on the bus
int currentPeople = 0;
int stopNumber = 0;

int waitingPeoplestop2 = 1;  // Number of waiting people at stop 2
int waitingPeoplestop3 = 1;  // Number of waiting people at stop 3

bool busFull = false;    // Flag to indicate if the bus is full

unsigned long arrivalTime;
unsigned long departureTime;
unsigned long departureTime1;
unsigned long departureTime2;
unsigned long departureTime3;

WiFiClientSecure net;
PubSubClient client(net);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  
  // Connect to Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  while (!Blynk.connected()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Blynk");

  
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IoT");

  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  // Update Blynk labels
  Blynk.virtualWrite(VIRTUAL_PIN_PEOPLE, currentPeople);
  Blynk.virtualWrite(VIRTUAL_PIN_STOP_NUMBER, stopNumber);
  Blynk.virtualWrite(VIRTUAL_PIN_WAITING_PEOPLE_2, waitingPeoplestop2);
  Blynk.virtualWrite(VIRTUAL_PIN_WAITING_PEOPLE_3, waitingPeoplestop3);
  StaticJsonDocument<200> doc;
  doc["CurrentPeople"] = currentPeople;
  doc["BusFull"] = busFull;
  doc["ArrivalTime"] = formatTimeWithAMPM(arrivalTime); // Include the formatted arrival time
  doc["DepartureTime"] = formatTimeWithAMPM(departureTime); // Include the formatted departure time
  doc["StopNumber"] = stopNumber; // Add the stop number to the JSON document
  doc["waitingPeoplestop2"] = waitingPeoplestop2;
  doc["waitingPeoplestop3"] = waitingPeoplestop3;

  char jsonBuffer[1024];
  serializeJson(doc, jsonBuffer); // Serialize the JSON document to a buffer

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Incoming message: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // Parse the incoming message as JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  // Check if the message contains the departure time
  if (doc.containsKey("DepartureTime"))
  {
    departureTime = doc["DepartureTime"].as<unsigned long>();
    Serial.print("Departure Time: ");
    Serial.println(formatTimeWithAMPM(departureTime));
  }
}

String formatTimeWithAMPM(unsigned long timeInMillis)
{
  int hour = (timeInMillis / 3600000) % 12; // Convert milliseconds to hours and take modulo 12
  int minute = (timeInMillis / 60000) % 60; // Convert milliseconds to minutes and take modulo 60
  String ampm = (timeInMillis / 43200000) ? "PM" : "AM"; // Determine AM or PM based on milliseconds

  if (hour == 0) {
    hour = 12; // 12-hour format, so set hour to 12 if it's 0
  }

  String formattedTime = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + " " + ampm;
  return formattedTime;
}

void setup()
{
  // Initialize Blynk virtual pins
  Blynk.virtualWrite(VIRTUAL_PIN_PEOPLE, currentPeople);
  Blynk.virtualWrite(VIRTUAL_PIN_STOP_NUMBER, stopNumber);
  Blynk.virtualWrite(VIRTUAL_PIN_WAITING_PEOPLE_2, waitingPeoplestop2);
  Blynk.virtualWrite(VIRTUAL_PIN_WAITING_PEOPLE_3, waitingPeoplestop3);
  Serial.begin(115200);
  pinMode(SENSOR_PIN_1, INPUT);
  pinMode(IR_SENSOR_PIN_1, INPUT);
  pinMode(SENSOR_PIN_2, INPUT);
  pinMode(IR_SENSOR_PIN_2, INPUT);
  pinMode(SENSOR_PIN_3, INPUT);
  pinMode(IR_SENSOR_PIN_3, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  connectAWS();
}


//blynk

void loop()
{
  int value1 = digitalRead(SENSOR_PIN_1);
  int sensorValue1 = digitalRead(IR_SENSOR_PIN_1);
  int value2 = digitalRead(SENSOR_PIN_2);
  int sensorValue2 = digitalRead(IR_SENSOR_PIN_2);
  int value3 = digitalRead(SENSOR_PIN_3);
  int sensorValue3 = digitalRead(IR_SENSOR_PIN_3);

  if (busFull) {
    return; // Stop the code execution if the bus is full
  }

  // Check if bus 1 has arrived
  if (value1 == LOW && sensorValue1 == HIGH) {
    Serial.println("Bus didn't come - Stop 1");
    digitalWrite(LED_PIN, LOW);
  }
  delay(1000);
  if (sensorValue1 == LOW) {
    Serial.println("Bus arrived - Stop 1");
    
    if (value1 == HIGH) {
      count++;
      currentPeople = limit - count;

      Serial.print("Current people in bus at stop 1: ");
      Serial.println(currentPeople);
      Blynk.virtualWrite(V0,currentPeople);
      publishMessage();

      unsigned long stop1Time = millis(); // Capture the current time
      arrivalTime = stop1Time; // Assign the arrival time
      departureTime1 = stop1Time; // Assign the departure time for stop 1

      Serial.print("Time at Stop 1: ");
      Serial.println(formatTimeWithAMPM(stop1Time));
      publishMessage();
      stopNumber = 1;
      // Publish the JSON document with stop number
      publishMessage();
      Blynk.virtualWrite(V1,stopNumber );
    }
  }
  delay(1000);

  // Check if bus 2 has arrived
  if (value2 == LOW && sensorValue2 == HIGH) {
    Serial.println("Bus didn't come - Stop 2");
    digitalWrite(LED_PIN, LOW);
  }
  delay(1000);
  if (sensorValue2 == LOW) {
    Serial.println("Bus arrived - Stop 2");
    if (value2 == HIGH) {
      if (waitingPeoplestop2 > 0) {
        waitingPeoplestop2--;
        limit--;
        if (limit < 0) {
          limit = 0;
        }
      }

      Serial.print("Waiting people at stop 2: ");
      Serial.println(waitingPeoplestop2);
      Blynk.virtualWrite(V2,waitingPeoplestop2);
      publishMessage();
      count = min(count, limit); // Ensure count does not exceed the new limit
      currentPeople = count;

      Serial.print("Current people in bus at stop 2: ");
      Serial.println(currentPeople);
      Blynk.virtualWrite(V0,currentPeople );
      publishMessage();

      unsigned long stop2Time = millis(); // Capture the current time
      arrivalTime = stop2Time; // Assign the arrival time
      departureTime2 = stop2Time; // Assign the departure time for stop 2

      Serial.print("Time at Stop 2: ");
      Serial.println(formatTimeWithAMPM(stop2Time));
      publishMessage();
      stopNumber = 2;
      // Publish the JSON document with stop number
      publishMessage();
      Blynk.virtualWrite(V1,stopNumber );
    }
  }
  delay(1000);

//bus 3
  if (value3 == LOW && sensorValue3 == HIGH) {
    Serial.println("Bus didn't come - Stop 3");
    digitalWrite(LED_PIN, LOW);
  }
  delay(1000);
  if (sensorValue3 == LOW) {
    Serial.println("Bus arrived - Stop 3");
    if (value3 == HIGH) {
      if (waitingPeoplestop3 > 0) {
        waitingPeoplestop3--;
        limit--;
        if (limit < 0) {
          limit = 0;
        }
      }

      Serial.print("Waiting people at stop 3: ");
      Serial.println(waitingPeoplestop3);
      Blynk.virtualWrite(V3,waitingPeoplestop3);
      publishMessage();
      count = min(count, limit); // Ensure count does not exceed the new limit
      currentPeople = count;

      Serial.print("Current people in bus at stop 3: ");
      Serial.println(currentPeople);
      Blynk.virtualWrite(V0,currentPeople );
      publishMessage();

      unsigned long stop3Time = millis(); // Capture the current time
      arrivalTime = stop3Time; // Assign the arrival time
      departureTime3 = stop3Time; // Assign the departure time for stop 2

      Serial.print("Time at Stop 3: ");
      Serial.println(formatTimeWithAMPM(stop3Time));
      publishMessage();
      stopNumber = 3;
      // Publish the JSON document with stop number
      publishMessage();
      Blynk.virtualWrite(V1,stopNumber );
    }
  }

  delay(1000);

  // Check if the bus is full
  if (count >= limit) {
    busFull = true;
    Serial.println("Bus is full");
    digitalWrite(LED_PIN, HIGH);
    publishMessage();
  }

  // Reconnect to AWS if the connection is lost
  if (!client.connected()) {
    connectAWS();
  }

  // Maintain the MQTT connection
  client.loop();

  Blynk.run();//Run the Blynk library
  timer.run();//Run the Blynk timer
}
