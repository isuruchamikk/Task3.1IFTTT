#include <WiFiNINA.h> // Helps the Arduino connect to WiFi.
#include <ArduinoHttpClient.h>   // Allows sending web requests (to IFTTT).
#include <Wire.h> // Needed for I2C communication (used by the BH1750 sensor).
#include <BH1750.h> //  Library to read light levels from the sensor.

BH1750 lightSensor;  // Creates a BH1750 sensor object
WiFiClient client;    // Handles WiFi connection

String event1 = " "; // Prompt your event name in here. ex; "Sunlight_detected "
String event2 = " "; // Prompt your another event name in here. ex; "Sunlight_not_detected"

const char* ssid = " "; // Prompt your connenction name(whatever the device name of WiFi or Hospot)
const char* password = " "; // Prompt your connenction's password

// IFTTT settings
const char* HOST_NAME = "maker.ifttt.com";
const int HTTPS_PORT = 80;  // HTTP (change to 443 for HTTPS with additional SSL setup)
const String IFTTT_KEY = "your key";  // Replace with your key



String queryString = "?value1=";

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, "maker.ifttt.com", 80);


// Light threshold (adjust based on testing)
float lightThreshold = 300.0; // Adjust based on testing (in lux)
bool sunlightDetected = false; // Tracks if sunlight is currently detected

void setup() {
  Serial.begin(9600);  // Start serial communication (for debugging)
  Wire.begin(); // Initialize I2C (for BH1750 sensor)
  // Initialize the light sensor
  lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

 // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  float lux = lightSensor.readLightLevel(); // Read light level in lux
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lux"); // Print to Serial Monitor

// Check if sunlight is detected
  if (lux > lightThreshold && !sunlightDetected) {
    sendIFTTTNotification("event1"); // Send IFTTT alert
    sunlightDetected = true; // Update state
  } else if (lux <= lightThreshold && sunlightDetected) {
    sendIFTTTNotification("event2");
    sunlightDetected = false; // Update state
  }

  delay(5000); // Wait 5 seconds before next check
  
}

void sendIFTTTNotification(String eventName) {
  Serial.println("Attempting to connect to IFTTT...");
  
  if (client.connect(HOST_NAME, HTTPS_PORT)) {
    Serial.println("Connected to IFTTT server");

// Construct the HTTP GET request
    String path = "https://maker.ifttt.com/trigger/"+ eventName +"/with/key/"+ IFTTT_KEY;
    
    // Construct the request
      client.println("GET " + String(path) + queryString + " HTTP/1.1");
       client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println();  // End of the HTTP header
    
    

    // Wait for response (5-second timeout)
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 5000) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line); // Print IFTTT response
      }
    }
    
    client.stop(); // Close connection
    Serial.println("\nDisconnected from IFTTT");
  } else {
    Serial.println("Connection to IFTTT failed!");
  }
}