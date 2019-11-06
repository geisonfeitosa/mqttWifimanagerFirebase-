#if defined(ESP8266)
#include <ESP8266WiFi.h> //ESP8266 Core WiFi Library         
#else
#include <WiFi.h> //ESP32 Core WiFi Library    
#endif
 
#if defined(ESP8266)
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#else
#include <WebServer.h> //Local WebServer used to serve the configuration portal ( https://github.com/zhouhan0126/WebServer-esp32 )
#endif

#include <IOXhop_FirebaseESP32.h>
#include <Ticker.h>
#define FIREBASE_HOST "iotionic-53721.firebaseio.com"
#define FIREBASE_AUTH "Ksopm93YEZGhQu53aIAnuuSjCgdANWw7PfgJRq8R"
#define PUBLISH_INTERVAL 1000*60

#include "DHT.h"

#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal ( https://github.com/zhouhan0126/DNSServer---esp32 )
#include <WiFiManager.h> //WiFi Configuration Magic ( https://github.com/zhouhan0126/WIFIMANAGER-ESP32 ) >> https://github.com/tzapu/WiFiManager (ORIGINAL)

Ticker ticker;
bool publishNewState = true;
int pinLed = 18;
int pinTerm = 19;
DHT dht(pinTerm, DHT11);

const int PIN_AP = 2;
WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);
  pinMode(pinLed, OUTPUT);
  pinMode(PIN_AP, INPUT);
  Serial.println("DHTxx test!");
  dht.begin();

  //declaração do objeto wifiManager
  WiFiManager wifiManager;
 
  //callback para quando entra em modo de configuração AP
  wifiManager.setAPCallback(configModeCallback);
  
  //callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
  wifiManager.setSaveConfigCallback(saveConfigCallback); 
 
  //cria uma rede de nome ESP_AP com senha 12345678
  wifiManager.autoConnect("ESP_AP", "12345678");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  ticker.attach_ms(PUBLISH_INTERVAL, publish);
}

void loop() {
   if ( digitalRead(PIN_AP) == HIGH ) {
      WiFiManager wifiManager;
      if(!wifiManager.startConfigPortal("ESP_AP", "12345678")) {
        Serial.println("Falha ao conectar");
        delay(2000);
        ESP.restart();
      }
   }

    if(publishNewState) {
    Serial.println("Publish new State");
    // Obtem os dados do sensor DHT 
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
  
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT");
    } else {
      Firebase.pushFloat("temperature", temperature);
      Firebase.pushFloat("humidity", humidity);    
      publishNewState = false;
    }
  }
  
  bool lampValue = Firebase.getBool("lamp");
  Serial.println(lampValue);
  digitalWrite(pinLed, lampValue ? HIGH : LOW);
}

//callback que indica que o ESP entrou no modo AP
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entrou no modo de configuração");
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
  Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede
}

//callback que indica que salvamos uma nova rede para se conectar (modo estação)
void saveConfigCallback () {
  Serial.println("Configuração salva");
}

void publish() {
  publishNewState = true;
}
