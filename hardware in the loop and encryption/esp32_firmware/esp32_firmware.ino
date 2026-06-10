#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>

// network and server configurations

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "YOUR_THINGSPEAK_WRITE_API_KEY";

// components pins

#define DHTPIN 4  // DHT11 data pin will connect to GPIO 4
#define DHTTYPE DHT11  // specifying the sensor type
DHT dht(DHTPIN, DHTTYPE);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  // initializing the oled using the sh1106 I2C driver

// storing the hardware data in variables

String quantumKey = "00";
float temperature = 0.0;

// bitwise XOR encryption cipher

String encryptXOR(String msg, String key){
  String hexCiphertext = "";
  if(key.length() == 0) return msg;

  for(int i = 0; i< msg.length(); i++){
    char encryptedChar = msg[i] ^ key[i % key.length()];  // bitwise XOR operation between data character and key character
    if(encryptedChar < 16) hexCiphertext += "0";
    hexCiphertext += String(encryptedChar, HEX);
  }
  return hexCiphertext;
}

void setup(){

Serial.begin(115200);
dht.begin();
Wire.begin(21, 22);
u8g2.begin();

WiFi.begin(ssid, password);
while(WiFi.status() != WL_CONNECTED){delay(500); Serial.print(".");}
Serial.println("nConnected to Network.");

}

void loop(){
  // reading sensor data

 float t = dht.readTemperature();
  if (!isnan(t)) { temperature = t; }
  String tempStr = String(temperature, 1);

  // fetching the qunatum key from the flask using HTTP GET

if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      JsonDocument doc;
      deserializeJson(doc, payload);
      if (doc["quantum_key"]) { quantumKey = String(doc["quantum_key"]); }
    }
    http.end();
  }

  //  encrypt data using the Quantum Key
  String ciphertext = encryptXOR(tempStr, quantumKey);
  Serial.println("Original Temp: " + tempStr + " | Key: " + quantumKey + " | Ciphertext: " + ciphertext);

  // Transmitting payload to ThingSpeak 
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // We send the encrypted ciphertext to Field 1, and send the key to Field 2so the cloud can decrypt it!
    String url = "http://api.thingspeak.com/update?api_key=" + String(thinkSpeakApiKey) + 
                 "&field1=" + ciphertext + 
                 "&field3=" + quantumKey;
    
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Cloud Sync Successful. Code: " + String(httpResponseCode));
    } else {
      Serial.println("Cloud Sync Failed!");
    }
    http.end();
  }

  // rendering the display
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 12, ("Plain:  " + tempStr + " C").c_str());
  u8g2.drawStr(0, 26, ("Q-Key:  " + quantumKey).c_str());
  u8g2.drawStr(0, 44, "Cipher:");
  u8g2.drawStr(0, 56, ciphertext.c_str()); // Displaying the scrambled ciphertext visually!
  u8g2.drawHLine(0, 32, 128);
  u8g2.sendBuffer();

  delay(30000);
  }

