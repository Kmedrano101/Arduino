/*
    Descripción: Cantidad de pulsos de boton en un tiempo determinado
    Autor: Kevin Medrano Ayala
    Fecha: XX/XX/XX
*/
// Directivas de preprocesador
#define BOTON_INICIO 3
#define BOTON_PULSOS 4 
#define LED 10
// Variables, estructuras, declaraciones
 byte aux_inicio = 0;
 unsigned long tiempo_capturado_inicio = 0;
 byte aux_boton = 0;
 int pulsos = 0;
void setup()
{
 pinMode(BOTON_INICIO, INPUT_PULLUP);
 pinMode(BOTON_PULSOS, INPUT_PULLUP);
 pinMode(LED, OUTPUT);
 Serial.begin(9600);
}
void loop()
{
 if (!digitalRead(BOTON_INICIO) && aux_inicio == 0)
 {
     aux_inicio = 1;
     tiempo_capturado_inicio = millis();
     Serial.println("Se ha iniciado, pulsa el boton!");
 }
 if (aux_inicio == 1)
 {  // 7000 - 2000 >= 5000
     if (millis() - tiempo_capturado_inicio >= 5000)
     {
         aux_inicio = 0;
         Serial.println("Ha finalizado el tiempo de 5 segundos!");
         Serial.print("Cantidad de pulsos: ");
         Serial.print(pulsos);
         encenderNLed(pulsos);
         Serial.println();
         pulsos = 0;
     }
    if(!digitalRead(BOTON_PULSOS))
    {
        aux_boton = 1;
    }
    if (aux_boton == 1 && digitalRead(BOTON_PULSOS))
    {
        pulsos += 1;
        aux_boton = 0;
    }
 }
 
 
}

// Metodos
void encenderNLed(int cantidad)
{
    for (int i = 0; i < cantidad; i++)
    {
        digitalWrite(LED, 1);
        delay(400);
        digitalWrite(LED, 0);
        delay(400);
    }
    
}