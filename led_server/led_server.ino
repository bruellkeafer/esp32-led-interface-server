#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <credentials.h>

WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", R"=====(
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <title>Almost Smart Home</title>
        </head>
        <body>
            <button onclick="postData(1)">Button 1</button>
            <button onclick="postData(2)">Button 2</button>
            <button onclick="postData(3)">Button 3</button>

            <script>
                function postData(number) {
                    fetch('/set', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({ number: number })
                    })
                    .then(response => response.json())
                    .then(data => console.log(data))
                    .catch(error => console.error('Error:', error));
                }
            </script>
        </body>
        </html>
    )=====");
  digitalWrite(led, 0);
}

void handleSet() {
  digitalWrite(led, 1);
  Serial.println("hallo");
  Serial.println(server.header("Content-Type"));
  // Check if the content type is application/json

  // Parse the JSON data
  DynamicJsonDocument jsonDoc(1024);
  deserializeJson(jsonDoc, server.arg("plain"));

  // Extract the number from the JSON
  int number = jsonDoc["number"];

  // Log the number to the serial console
  Serial.print("Received number: ");
  Serial.println(number);

  server.send(200, "application/json", "{\"status\":\"ok\"}");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/set", HTTP_POST, handleSet);  // Updated to handle the /set route for POST requests

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);  // allow the CPU to switch to other tasks
}
