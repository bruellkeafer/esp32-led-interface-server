#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "credentials.h"

WebServer server(80);

const int led = 13;

bool LedIsOn = true;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Almost Smart Home</title>
</head>
<body>
    <button onclick="setStatusOn()">On</button>
    <button onclick="setStatusOff()">Off</button>
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

        const setStatusOn = () => setStatus("on");
        const setStatusOff = () => setStatus("off");


        function setStatus(newStatus){
          fetch('/set', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ status: newStatus })
            })
            .then(response => response.json())
            .then(data => console.log(data))
            .catch(error => console.error('Error:', error));
        }
    </script>
</body>
</html>
)html");
  digitalWrite(led, 0);
}

void handleSet() {
  Serial.println("hallo");  

  DynamicJsonDocument jsonDoc(1024);
  deserializeJson(jsonDoc, server.arg("plain"));

  // Extract the status from the JSON
  String status = jsonDoc["status"];

  // Log the status to the serial console
  Serial.print("Received status: ");
  Serial.println(status);

  // Handle the status
  if (status == "on") {
    // Code to turn something on
    Serial.println("on");
  } else if (status == "off") {
    // Code to turn something off
    Serial.println("off");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid status\"}");
    digitalWrite(led, 0);
    return;
  }

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
  server.on("/set", HTTP_POST, handleSet);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);  // allow the CPU to switch to other tasks
}
