#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal.h>

AsyncWebServer server(80);

const int rs = 19, en = 23, d4 = 18, d5 = 17, d6 = 16, d7 = 15;
const int p1BPin = 27, p2BPin=26; //p1 is white p2 is green
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
unsigned long p1Time = 60000;
unsigned long p2Time = 60000;
unsigned long increment = 0;
bool isP1Turn = false;
bool gameStarted = false;
unsigned long previousMillis = 0;
const unsigned long interval = 1000;

const char* ssid = "chessclock";
const char* password = "password";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<html lang=en-EN><head><meta http-equiv='refresh' content='60'/>
<title>Chess Clock Time Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }</style>
<h1>Chess Clock Time Adjust</h1>
<h3>Enter time in seconds and submit</h3>
<form action="/get">
    Player 1 Time: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
<form action="/get">
    Player 2 Time: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form>
  <h3>Enter increment in seconds and submit</h3>
  <form action="/get">
    Time Increment: <input type="text" name="input3">
    <input type="submit" value="Submit">
  </form>
</body>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  lcd.begin(16, 2);
  lcd.print("P1            P2");
  pinMode(p1BPin, INPUT_PULLUP);
  pinMode(p2BPin, INPUT_PULLUP);
  Serial.begin(115200);
  // WiFi.mode(WIFI_STA);
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password); 
  // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   Serial.println("WiFi Failed!");
  //   return;
  // }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String val1;
    String val2;
    String val3;
    if (request->hasParam(PARAM_INPUT_1)) {
      val1 = request->getParam(PARAM_INPUT_1)->value();
      p1Time = atol(val1.c_str())*1000;
    }
    else if (request->hasParam(PARAM_INPUT_2)){
      val2 = request->getParam(PARAM_INPUT_2)->value();
      p2Time = atol(val2.c_str())*1000;
    }
    else if (request->hasParam(PARAM_INPUT_3)){
      val3 = request->getParam(PARAM_INPUT_3)->value();
      increment = atol(val3.c_str())*1000;
    }
    request->send_P(200, "text/html", index_html);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  chessClock();
}

void chessClock(){
  if(!gameStarted){
    lcd.setCursor(0, 1);
    printTime(p1Time);
    lcd.setCursor(11, 1);
    printTime(p2Time);
    if(digitalRead(p1BPin)==LOW){
      gameStarted = true;
      previousMillis = millis();

    }
    else if(digitalRead(p2BPin)==LOW){
      isP1Turn = true;
      gameStarted=true;
      previousMillis = millis();
    }
  }
  else{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if(isP1Turn){ //when p2 hits button its p1 turn
        if (p1Time > 0) {
          p1Time -= 1000;
        } 
        else {
          // Player 1's time is up, game over
          while (true) {
            // Wait indefinitely
          }
        }
      }
      else{
        if (p2Time > 0) {
          p2Time -= 1000;
        } 
        else {
          // Player 2's time is up, game over
          while (true) {
            // Wait indefinitely
          }
        }
      }
    }
    // Update LCD display
    lcd.setCursor(0, 1);
    printTime(p1Time);
    lcd.setCursor(11, 1);
    printTime(p2Time);

    // Check for player switch
    if (digitalRead(p1BPin) == LOW) {
      if (isP1Turn){
        p1Time += increment;
      }
      isP1Turn = false;
    } 
    if (digitalRead(p2BPin) == LOW) {
      if (!isP1Turn){
        p2Time += increment;
      }
      isP1Turn = true;
    }
  }
}

void printTime(unsigned long time) {
  unsigned long seconds = (time / 1000) % 60;
  unsigned long minutes = (time / 1000) / 60;

  if (minutes < 10) {
    lcd.print("0");
  }
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) {
    lcd.print("0");
  }
  lcd.print(seconds);
}