/*
    Descripción: Ejemplos de Contadores
    Autor: Kevin Medrano Ayala
    Fecha: XX/XX/XX
*/
// Directivas de preprocesador
 #define led 4
 #define boton 3
// Variables, estructuras, declaraciones
int contador = 0; 
bool aux_led = false; 

int aux_contador = 0;

unsigned long aux_millis=0;
void setup()
{
    // Configuraciones
    pinMode(led, OUTPUT);
    pinMode(boton, INPUT_PULLUP);
    Serial.begin(9600);
}
void loop()
{
    encenderLed();
    if (!digitalRead(boton))
    {
        aux_contador = 1;
    }
    if (aux_contador == 1 && digitalRead(boton))
    {
        contador += 1;
        aux_contador = 0;
    }
    /*
    if (!digitalRead(boton))
    {
        while(!digitalRead(boton)){}
        contador = contador + 1;
        //delay(1200);
    }*/
    Serial.println(contador);

}
// Metodos
void encenderLed()
{
    if (millis() - aux_millis >= 300)
    {
        aux_millis = millis();
        if (!aux_led)
        {
            digitalWrite(led, 1);
            aux_led = true;
        }
        else
        {
            digitalWrite(led, 0);
            aux_led = false;
        }
    } 
}