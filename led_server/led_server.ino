#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "credentials.h"

#define NUM_LEDS 61
#define DATA_PIN 5
#define CLOCK_PIN 13
CRGB leds[NUM_LEDS];
WebServer server(80);


bool LedIsOn = true;

void handleRoot() {
  server.send(200, "text/html", R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Almost Smart Home</title>
    <style>
        .slider-color {
            -webkit-appearance: none;
            width: 100%;
            height: 15px;
            border-radius: 5px;
            background: #d3d3d3;
            outline: none;
            opacity: 0.7;
            -webkit-transition: opacity .15s ease-in-out;
            transition: opacity .15s ease-in-out;
        }
    </style>
</head>
<body>
<button onclick="setStatusOn()">On</button>
<button onclick="setStatusOff()">Off</button>
<button onclick="postData(3)">Button 3</button>
<input type="range" min="0" max="256" value="128" class="slider-color" id="red">
<input type="range" min="0" max="256" value="128" class="slider-color" id="green">
<input type="range" min="0" max="256" value="128" class="slider-color" id="blue">

<script>
    const setStatusOn = () => setStatus("on");
    const setStatusOff = () => setStatus("off");


    function setStatus(newStatus, r, g, b) {
        fetch('/set', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({status: newStatus, red: r, green: g, blue: b})
        })
            .then(response => response.json())
            .then(data => console.log(data))
            .catch(error => console.error('Error:', error));
    }

    var r = 0, g = 0, b = 0;
    var sliderR = document.getElementById("red");
    var sliderG = document.getElementById("green");
    var sliderB = document.getElementById("blue");

    sliderR.oninput = function (e) {
        r = e.target.value;
        console.log(r);
        setStatus("rgb", r, g, b);
    }

    sliderG.oninput = function (e) {
        g = e.target.value;
        console.log(g);
        setStatus("rgb", r, g, b);
    }

    sliderB.oninput = function (e) {
        b = e.target.value;
        console.log(b);
        setStatus("rgb", r, g, b);
    }

</script>

</body>
</html>
)html");
}

void handleSet() {
  Serial.println("hallo");

  DynamicJsonDocument jsonDoc(1024);
  deserializeJson(jsonDoc, server.arg("plain"));

  String status = jsonDoc["status"];

  Serial.print("Received status: ");
  Serial.println(status);

  // Handle the status
  if (status == "on") {
    Serial.println("on");
    for(int i=0; i <= NUM_LEDS; i++){
      leds[i] = CRGB::White;
    }
    FastLED.show();
  } else if (status == "off") {
    Serial.println("off");
    for(int i=0; i <= NUM_LEDS; i++){
      leds[i] = CRGB::Black;
    }
    FastLED.show();
  } else if (status == "rgb") {
    Serial.println("rgb");
    int r = jsonDoc["red"];
    int g = jsonDoc["green"];
    int b = jsonDoc["blue"];
    Serial.println(r);
    for(int i=0; i <= NUM_LEDS; i++){
      leds[i].red = r;
      leds[i].green = g;
      leds[i].blue = b;
    }
    FastLED.show();
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid status\"}");
    return;
  }

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleNotFound() {
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
}

void setup(void) {
  delay(2000);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

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

  for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
  }
}

void loop(void) {
  server.handleClient();
  delay(2);  // allow the CPU to switch to other tasks
}
