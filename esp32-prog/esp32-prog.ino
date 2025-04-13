#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <base64.h>

// Indentifiants pour le WIFI
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";

// Identifiants pour l'authentification
const char* http_username = "admin";
const char* http_password = "1234";

// UserAgent personnalisé (expulse les navigateurs qui n'ont pas ce UserAgent)
const char* requiredUserAgent = "ESPShade/ANYTHING";

// GPIO pour le moteur
const int motorUpPin = 5;
const int motorDownPin = 5;
bool motorState = false;
const char* upOrDown = "OFF";

// IP fixe pour éviter les problèmes
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

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
  WiFi.disconnect();

  // Connexion Wi-Fi avec IP statique
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Échec de la configuration de l'IP statique");
  }

  // Connexion au WIFI
  WiFi.begin(ssid, password);
  Serial.print("Connexion au Wi-Fi...");

  int maxRetries = 20; // 20 * 500ms = 10 secondes max d'attente
  while (WiFi.status() != WL_CONNECTED && maxRetries > 0) {
    delay(500);
    Serial.print(".");
    maxRetries--;
  }

  Serial.println(" ");

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Problème de connexion...");
    return;
  }

  Serial.println("\nConnecté !");
  Serial.println(WiFi.localIP());
}

void setup() {
  
  Serial.begin(115200);
  //pinMode(motorUpPin, OUTPUT);
  //digitalWrite(motorUpPin, LOW);
  //pinMode(motorDownPin, OUTPUT);
  //digitalWrite(motorDownPin, LOW);

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
      digitalWrite(motorDownPin, LOW);
      digitalWrite(motorUpPin, HIGH);
      request->send(200, "application/json", "{\"message\":\"Motor turned UP\"}");
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
      digitalWrite(motorUpPin, LOW);
      digitalWrite(motorDownPin, LOW);
      request->send(200, "application/json", "{\"message\":\"Motor turned OFF\"}");
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
      digitalWrite(motorUpPin, LOW);
      digitalWrite(motorDownPin, HIGH);
      request->send(200, "application/json", "{\"message\":\"Motor turned DOWN\"}");
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

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
  });

  server.begin();
}

void loop() {}