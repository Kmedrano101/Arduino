/*
    Descripción: Proyecto de...
    Autor: Kevin Medrano Ayala
    Fecha: XX/XX/XX
*/

// Directivas de preprocesador
#define led 11
#define boton 4


//const byte led = 11; 
// Variables, estructuras, declaraciones
 
void setup()
{
    pinMode(led, OUTPUT);
    pinMode(boton, INPUT_PULLUP);
}

void loop()
{
    bool stado = digitalRead(4);
    if (!stado)
    {
        digitalWrite(led, 1);
    }
    
}
// Metodos
