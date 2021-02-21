/*
    Descripción: Comunicacion serial UART con NodeRED mediante JSON,
    Control de pines de entrada y salida de Arduino desde NodeRED
    Autor: Kevin Medrano Ayala
    Fecha: 17/01/21
*/
// Directivas de preprocesador
#include <ArduinoJson.h>
#define LED 2
#define BOTON 3

// Variables
float temperatura = 26.4;
int humedad = 78;
int estado_BOTON = 0; 
void setup()
{
  // Iniciamos el puerto serie a 115200 bps
  Serial.begin(115200);
  // Configuramos como salida 2 pines
  pinMode(LED,OUTPUT);
  pinMode(BOTON,INPUT_PULLUP);
    
}
 
void loop()
{
  EnviarJSON();
  delay(1400);
}
// Metodos
void EnviarJSON()
{
  String json;
  StaticJsonDocument<300> doc;
  doc["temperatura"] = temperatura;
  doc["humedad"] = humedad;
  estado_BOTON = digitalRead(BOTON);
  doc["Boton"] = estado_BOTON;
  // Se crea el formato de cadena JSON
  serializeJson(doc, json);
  // Enviamos la cadena JSON por serial
  Serial.println(json);
}
 
