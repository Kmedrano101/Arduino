// Importar librerias y directivas de preprocesador
#include "DHT.h"
#include <ArduinoJson.h>
#define LED 4
#define BOTON 6
#define DHTPIN 8
#define DHTTYPE DHT11 
// Declaracion de variables
DHT dht(DHTPIN, DHTTYPE);

int Estado_Led = 0;
int Temperatura = 0;
int aux_boton = 0;
unsigned long auxTiempo=0;

void setup() {
  // Configuraciones de puertos
  pinMode(LED, OUTPUT);
  pinMode(BOTON,INPUT_PULLUP);
  Serial.begin(9600);
  dht.begin();
  delay(2000);
}

void loop() {
  // Logica del programa
  if(!digitalRead(BOTON) && aux_boton==0)
  {
    Estado_Led = 1;
    aux_boton = 1;
    while(!digitalRead(BOTON)){}
  }
  if(!digitalRead(BOTON) && aux_boton==1)
  {
    Estado_Led = 0;
    aux_boton = 0;
    while(!digitalRead(BOTON)){}
  }
  encender_apagar_Led(LED);
  
  if(millis() - auxTiempo > 700)
  {
    EnviarJSON_PC();
    RecibirJSON_PC();
    auxTiempo = millis();
  }
}
void EnviarJSON_PC()
{
  String json;
  StaticJsonDocument<300> doc;
  doc["temperatura"] = obtener_Temperatura();
  doc["Led"] = obtener_estado_Led();
  serializeJson(doc, json);
  Serial.println(json);
}
void RecibirJSON_PC()
{
  String json = "";
  while(Serial.available()>0)
  {
    json = Serial.readStringUntil('\n');
  }
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, json);
  if(error) { return; }
  Estado_Led = doc["Led"];
  json = "";
}
int obtener_Temperatura()
{
  // Codigo para lectura de temperatura
  Temperatura = int(dht.readTemperature());
  return Temperatura;
}
int obtener_estado_Led()
{
  return Estado_Led;
}
void encender_apagar_Led(int pin)
{
  digitalWrite(pin,Estado_Led);
}
