#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

const char* temperaturaTopic = "mestrado/iot/aluno/klaus/temperatura";
const char* umidadeTopic = "mestrado/iot/aluno/klaus/umidade";
const char* mqttServer ="5.196.78.28"; //"test.mosquitto.org";
const int mqttPort = 1883;

const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

ESP8266WiFiMulti WiFiMulti;
WiFiClient espClient;  

PubSubClient client(espClient);

#define PIN_INPUT 0
#define RELAY_PIN 2

#define SSID_LEN 32             // Max SSID length as 802.11 definition
#define KEY_LEN 63              // Max WPA2-PSK length
#define DHTPIN 0
#define DHTTYPE DHT11   // DHT 11

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ESP8266WebServer server(80);

DHT dht(DHTPIN, DHTTYPE);
void displayTemperatureInfo(float tempC, float humidity, float heatIndexC) {
  display.clearDisplay();
  
  display.setTextSize(2); // Set text size to normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Set text color to white

  // Prepare the message strings
  String tempStr = "T: " + String(tempC) + " C";
  String humidityStr = "H: " + String(humidity) + "%";
  String heatIndexStr = "S: " + String(heatIndexC) + " C";
  
  // Display Temperature
  display.setCursor(0, 0);
  display.println(tempStr);
  display.display();
  delay(500);
  
  // Display scrolling humidity text
  display.setCursor(0, 20);
  display.println(humidityStr);
  display.display();
  delay(500);

  // Display scrolling heat index text
  display.setCursor(0, 40);
  display.println(heatIndexStr);
  display.display();
  delay(500);

  delay(3000);
}

#define BITCOIN_LOGO_HEIGHT 64
#define BITCOIN_LOGO_WIDTH  64

static const unsigned char PROGMEM bitcoin_logo_bmp[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc7, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x03, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x8f, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xe7, 0x8f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x8f, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7f, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x07, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7e, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0xfc, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfe, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x01, 0xfc, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xfc, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x87, 0xfc, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xf0, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0xfb, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x3f, 0xe0, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xf8, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x07, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xf0, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x03, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x7f, 0xf0, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x03, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xf8, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x1f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xf0, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xdf, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc7, 0xff, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x03, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x8f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0x8f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x8f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void displayBitcoinLogo() {
  display.drawBitmap(  (display.width()  - BITCOIN_LOGO_WIDTH ) / 2,
    (display.height() - BITCOIN_LOGO_HEIGHT) / 2, bitcoin_logo_bmp, BITCOIN_LOGO_WIDTH,BITCOIN_LOGO_HEIGHT, WHITE);
  display.display();
  delay(2000);
}

float getBitcoinPrice() {
  if (WiFiMulti.run() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();  // Ignore SSL certificate for simplicity

    HTTPClient https;
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://api.coindesk.com/v1/bpi/currentprice/BTC.json")) {  // Coindesk API URL
      int httpCode = https.GET();
      if (httpCode > 0) {
        String payload = https.getString();
        Serial.println("[HTTPS] GET... success");
        Serial.println(payload);

        int priceStart = payload.indexOf("rate\":\"") + 7;
        int priceEnd = payload.indexOf("\",", priceStart);
        String priceStr = payload.substring(priceStart, priceEnd);
        priceStr.replace(",", "");  // Remove commas for larger values

        return priceStr.toFloat();
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  return 0.0;  // Return 0.0 if there was an error
}

void displayBitcoinPrice(float price) {
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 8);
  
  display.println("BTC:");
  display.display();   
  delay(200);

  display.setCursor(8, 32);
  display.println("$"+String(price, 2));

  display.display();   
  delay(5000);
}

/**
 * HTTP route handlers
 ********************************************************************************/

/**
 * GET /
 */
void handleGETRoot() 
{
  // I always loved this HTTP code
  server.send(418, "text/plain", "\
            _           \r\n\
         _,(_)._            \r\n\
    ___,(_______).          \r\n\
  ,'__.           \\    /\\_  \r\n\
 /,' /             \\  /  /  \r\n\
| | |              |,'  /   \r\n\
 \\`.|                  /    \r\n\
  `. :           :    /     \r\n\
    `.            :.,'      \r\n\
      `-.________,-'        \r\n\
  \r\n");
}

/**
 * GET /temperature
 */
void handleGETTemp()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  String json = "{\"Temperature\":";
  json += t;
  json += ",\"PID_Output\":";
  json += h;
  json += " }\r\n";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  Serial.println(F("DHT11 started"));
  dht.begin();

  pinMode(LED_BUILTIN, OUTPUT);
 
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFiMulti.addAP("YOUR WIFI", "PASSWORD");

  Serial.println("Connecting to WiFi...");
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println(" Conectado!");
    } else {
      Serial.print(" Falhou, código de erro: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }

  server.on("/", handleGETRoot );
  server.on("/temperature", HTTP_GET, handleGETTemp);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
  server.begin();
  Serial.println("HTTP server started.");
  displayBitcoinLogo();
}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  displayTemperatureInfo(t, h, hic);

  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://fishy-control.vercel.app/api/data")) {  // HTTPS

      Serial.print("[HTTPS] POST...\n");
      https.addHeader("Content-Type", "application/json");

      String json = "{\"Temperature\":";
      json += t;
      json += ",\"Output\":";
      json += h;
      json += " }";
      
      // start connection and send HTTP header
      int httpCode = https.POST(json);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  
  server.handleClient();
  float btcPrice = getBitcoinPrice();
  if (btcPrice > 0.0) {
    display.clearDisplay();
    displayBitcoinLogo();  // Display Bitcoin logo
    displayBitcoinPrice(btcPrice);  // Display Bitcoin price with scrolling text
  }

  if (!client.connected()) {
    // Reconecta se a conexão MQTT for perdida
    while (!client.connected()) {
      Serial.print("Reconectando ao broker MQTT...");
      if (client.connect("ESP8266Client_klaus")) {
        Serial.println(" Reconectado!");
      } else {
        Serial.print(" Falhou, código de erro: ");
        Serial.print(client.state());
        Serial.println(" Tentando novamente em 5 segundos...");
        delay(5000);
      }
    }
  } 
    // Format temperature
    snprintf(msg, MSG_BUFFER_SIZE, "%.2f", t);  // Format temperature with 2 decimal places

    // Publish formatted temperature
    if (client.publish(temperaturaTopic, msg)) {
      Serial.println("Temperature successfully published to MQTT.");
    } else {
      Serial.println("!!!!Failed to publish temperature to MQTT!!!!!");
    }

    // Format humidity
    snprintf(msg, MSG_BUFFER_SIZE, "%.2f", h);  // Format humidity with 2 decimal places

    // Publish formatted humidity
    if (client.publish(umidadeTopic, msg)) {
      Serial.println("Humidity successfully published to MQTT.");
    } else {
      Serial.println("!!!!Failed to publish humidity to MQTT!!!!");
    }
  
  delay(4000);
}
