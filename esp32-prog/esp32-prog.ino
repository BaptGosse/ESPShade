#include <WiFi.h>

// Remplace par les identifiants de ton Wi-Fi
const char* ssid = "SSID";
const char* password = "MDP";

// GPIO pour le moteur (via un relais ou un MOSFET)
const int motorPin = 5;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  // Connexion au Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté !");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connecté !");
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    if (request.indexOf("/ON") != -1) {
      digitalWrite(motorPin, HIGH);
    }
    if (request.indexOf("/OFF") != -1) {
      digitalWrite(motorPin, LOW);
    }

    // Page HTML envoyée au client
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<h1>Contrôle du moteur</h1>");
    client.println("<button onclick=\"location.href='/ON'\" style='height:100px;width:200px;font-size:30px;'>Démarrer</button>");
    client.println("<button onclick=\"location.href='/OFF'\" style='height:100px;width:200px;font-size:30px;'>Arrêter</button>");
    client.println("</html>");

    delay(1);
    Serial.println("Client déconnecté.");
  }
}
