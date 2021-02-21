/*
  Descripción: Calcular el tiempo pulsado de un boton
  Autor: Kevin Medrano Ayala
  Fecha: XX/XX/XX
*/
// Directivas de preprocesador
#define BOTON 4
// Variables, estructuras, declaraciones
unsigned long tiempo_capturado_pulsado = 0;
unsigned long tiempo_capturado_libre = 0;
unsigned long tiempo_transcurrido = 0;
byte aux_boton = 0;

void setup()
{
 pinMode(BOTON, INPUT_PULLUP);
 Serial.begin(9600);
}

void loop()
{
  if(!digitalRead(BOTON) && aux_boton == 0)
  {
    aux_boton = 1;
    tiempo_capturado_pulsado = millis();
  }
  if (digitalRead(BOTON) && aux_boton == 1)
  {
    tiempo_capturado_libre = millis();
    tiempo_transcurrido = tiempo_capturado_libre - tiempo_capturado_pulsado;
    aux_boton = 0;
    Serial.println(tiempo_transcurrido);
  }
  
}
// Metodos