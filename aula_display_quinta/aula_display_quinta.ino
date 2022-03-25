
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>



#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    14  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);




const char* ssid="motorolajoao";
const char* password="q1w2e3r4";

String token;

void setup() {

  Serial.begin(115200);
  Serial.print("Serial Connected.");

  //display lcd
  Wire.begin(4,5); // SDA pino4 = (D2), SCL pino 5 = (D1)


  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }




  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("conectado!");

  pegarToken();


}



void pegarToken(){
  
  if (WiFi.status() == WL_CONNECTED){

    HTTPClient https;
    WiFiClientSecure client;
    client.setInsecure(); //the magic line, use with caution
    client.connect("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=[id_projeto_firebase]", 443);
    https.begin(client, "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=[id_projeto_firebase]");
    https.addHeader("Content-Type", "application/json");

    String parametros = "{\"email\": \"lucas.desenv.web@gmail.com\",\"password\": \"123456\",\"returnSecureToken\": true}";

    int codeResp = https.POST(parametros);
    String retorno = https.getString();


    // parse retorno do token que veio do firebase
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
    DynamicJsonBuffer jsonBuffer(capacity);

    //aqui fazemos o parse transforma o retorno do jeito que precisamos
    JsonObject& root = jsonBuffer.parseObject(retorno);

    token = root["idToken"].as<char*>();
    https.end();
    
    Serial.println(token);
    
  }
}



void enviarBancoDeDados(int temperatura){

  String temp = "";
  temp += temperatura;
  if (WiFi.status() == WL_CONNECTED){
    
    HTTPClient https;
    WiFiClientSecure client;
    client.setInsecure(); //the magic line, use with caution
    client.connect("https://apisenaciot.azurewebsites.net/api/default/" + temp, 443);
    https.begin(client, "https://apisenaciot.azurewebsites.net/api/default/" + temp);
    https.addHeader("Authorization","Bearer "+token);

    int codeResp = https.GET();
    String retorno = https.getString();

    Serial.println("retorno: " + retorno);
    https.end();

    delay(3000);
  }
  
}



void loop() {


  float temperatura = analogRead(A0);
  temperatura = (3.3 * temperatura * 100)/1023;
  String resp = "";
  resp += temperatura;
  

  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0,0);
  display.println("TEMP: " + resp);
  display.display();

  if (WiFi.status() == WL_CONNECTED){
    display.println("wf:ok");
    display.display();
  }else{
    display.println("wf:off");
    display.display();
  }
  
  delay(2000);

  //convertendo para inteiro 

  int temp = temperatura;
  
  enviarBancoDeDados(temperatura);

  
}
