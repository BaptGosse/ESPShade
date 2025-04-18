#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <base64.h>

// Identifiants pour l'authentification
const char* http_username = "admin";
const char* http_password = "1234";

// UserAgent personnalisé (expulse les navigateurs qui n'ont pas ce UserAgent)
const char* requiredUserAgent = "ESPShade/ANYTHING";

// GPIO pour le moteur
const int motorUpPin = 4;
const int motorDownPin = 5;
const int enablePin = 18;
bool motorState = false;
const char* upOrDown = "OFF";

// IP fixe pour éviter les problèmes
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

// Création du wifi manager
WiFiManager wifiManager;

// Création du serveur
AsyncWebServer server(80);

// Fonction d'encodage en Base64
String encodeBase64(String input) {
  return base64::encode(input);
}

// Fonction de vérification du UserAgent
bool isUserAgentValid(AsyncWebServerRequest *request) {
  if (!request->hasHeader("User-Agent")) {
    request->send(400, "application/json", "{\"error\":\"Missing User-Agent\"}");
    return false;
  }

  String userAgent = request->header("User-Agent");

  if (!userAgent.startsWith("ESPShade/")) {
    Serial.println("⚠️ Accès refusé : n'est pas un UserAgent ESPShade !");
    request->client()->close();
    return false;
  }

  if (userAgent != requiredUserAgent) {
    request->send(403, "application/json", "{\"error\":\"Invalid User-Agent\"}");
    return false;
  }

  return true;
}

// Fonction de vérification du UserAgent de détection
bool isDetectionAgentValid(AsyncWebServerRequest *request) {
  if (!request->hasHeader("User-Agent")) {
    request->send(400, "application/json", "{\"error\":\"Missing User-Agent\"}");
    return false;
  }

  String userAgent = request->header("User-Agent");

  if (!userAgent.startsWith("ESPShade/")) {
    Serial.println("⚠️ Accès refusé : n'est pas un UserAgent ESPShade !");
    request->client()->close();
    return false;
  }

  if (userAgent != "ESPShade/Setup") {
    request->send(403, "application/json", "{\"error\":\"Invalid User-Agent\"}");
    return false;
  }

  return true;
}

// Fonction de vérification de l'authentification
bool isAuthenticated(AsyncWebServerRequest *request) {
  if (!request->hasHeader("Authorization")) {
    request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
    return false;
  }

  String authHeader = request->header("Authorization");
  String expectedAuth = "Basic " + encodeBase64(String(http_username) + ":" + String(http_password));

  if (authHeader.equals(expectedAuth)) {
    return true;
  } else {
    Serial.println("⚠️ Accès refusé : n'a pas les bons identifiants !");
    request->send(403, "application/json", "{\"error\":\"Forbidden\"}");
    return false;
  }
}

// Fonction de vérification combinée (Auth + UserAgent)
bool isRequestValid(AsyncWebServerRequest *request) {
  return isUserAgentValid(request) && isAuthenticated(request);
}

void wifi_setup(){
  // Connexion au WIFI

  wifiManager.setSTAStaticIPConfig(local_IP, gateway, subnet);
  
  wifiManager.setConnectTimeout(10);
  wifiManager.setConfigPortalTimeout(300);

  if (!wifiManager.autoConnect("ESPShadeSetup")) {
    Serial.println("❌ Connexion échouée. Redémarrage...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("✅ Connecté au Wi-Fi !");
  Serial.println(WiFi.localIP());
}

void progressive_move(){
  for(int i = 0; i <= 255; i+=5){
    analogWrite(enablePin, i);
    delay(100);
    if(i <= 150){
      i = i+5;
    }
  }
}

void progressive_stop(){
  for(int i = 255; i >= 0; i-=5){
    analogWrite(enablePin, i);
    delay(100);
    if(i <= 150){
      i = i-5;
    }
  }
}

void setup() {
  
  Serial.begin(115200);
  pinMode(motorUpPin, OUTPUT);
  digitalWrite(motorUpPin, LOW);
  pinMode(motorDownPin, OUTPUT);
  digitalWrite(motorDownPin, LOW);
  pinMode(enablePin, OUTPUT);

  wifi_setup();

  // Route POST pour activer le moteur en levage
  server.on("/motor/up", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!isRequestValid(request)) return;

    Serial.println("Accès autorisé : ouverture du volet");

    if (motorState) {
      request->send(400, "application/json", "{\"error\":\"Motor is already in another state. Please stop it before\"}");
    } else {
      motorState = true;
      upOrDown = "UP";
      request->send(200, "application/json", "{\"message\":\"Motor turned UP\"}");
      digitalWrite(motorDownPin, LOW);
      digitalWrite(motorUpPin, HIGH);
      progressive_move();
    }
  });

  // Route POST pour désactiver le moteur
  server.on("/motor/off", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!isRequestValid(request)) return;

    Serial.println("Accès autorisé : arrêt du volet");

    if (!motorState) {
      request->send(400, "application/json", "{\"error\":\"Motor is already OFF\"}");
    } else {
      motorState = false;
      upOrDown = "OFF";
      request->send(200, "application/json", "{\"message\":\"Motor turned OFF\"}");
      progressive_stop();
      digitalWrite(motorUpPin, LOW);
      digitalWrite(motorDownPin, LOW);
    }
  });

  // Route POST pour activer le moteur en descente
  server.on("/motor/down", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!isRequestValid(request)) return;

    Serial.println("Accès autorisé : fermeture du volet");

    if (motorState) {
      request->send(400, "application/json", "{\"error\":\"Motor is already in another state. Please stop it before\"}");
    } else {
      motorState = true;
      upOrDown = "DOWN";
      request->send(200, "application/json", "{\"message\":\"Motor turned DOWN\"}");
      digitalWrite(motorUpPin, LOW);
      digitalWrite(motorDownPin, HIGH);
      progressive_move();
    }
  });

  // Route GET pour obtenir l'état du moteur
  server.on("/motor/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!isRequestValid(request)) return;

    Serial.println("Accès autorisé : accès au statut");

    StaticJsonDocument<200> jsonResponse;
    jsonResponse["motorState"] = motorState ? "ON" : "OFF";
    jsonResponse["upOrDown"] = upOrDown;
    String response;
    serializeJson(jsonResponse, response);
    request->send(200, "application/json", response);
  });

  // Route GET pour vérifier si le périphérique est un ESPShade
  server.on("/infos", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!isDetectionAgentValid(request)) return;

    Serial.println("Accès autorisé : accès au informations");

    StaticJsonDocument<200> jsonResponse;
    jsonResponse["imwhat"] = "ESPShade";
    jsonResponse["time"] = millis()/1000;
    jsonResponse["mac"] = WiFi.macAddress();
    jsonResponse["ip"] = WiFi.localIP();
    String response;
    serializeJson(jsonResponse, response);
    request->send(200, "application/json", response);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
  });

  server.begin();
}

void loop() {}