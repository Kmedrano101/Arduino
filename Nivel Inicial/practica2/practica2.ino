/*
    Descripción: Proyecto de...
    Autor: Kevin Medrano Ayala
    Fecha: XX/XX/XX
*/
// Directivas de preprocesador
 #define led 2
 #define boton1 5
 #define boton2 6
// Variables, estructuras, declaraciones

int var = 23;
int contador=0;
int acumulador=0;
int aux = 23;
 
void setup()
{
    pinMode(led, OUTPUT);
    pinMode(boton1, INPUT_PULLUP);
    pinMode(boton2, INPUT_PULLUP);
    //Serial.begin(9600);
}
void loop()
{
    int estado_boton1 = digitalRead(boton1);
    int estado_boton2 = digitalRead(boton2);
    if (!estado_boton1) // Verdadero
    {
        delay(1400);
        contador = contador + 1;
        while (estado_boton1)
        {
            contador = contador + 1;
            //Serial.println(contador);
        }
        
        if (estado_boton1)
        {
            contador = contador + 1;
            Serial.println(contador);
        }  
    }
    /*else{
        digitalWrite(led, 0);
    }
    for (int i = 0; i < 10; i++)
        {
            digitalWrite(led,1);
            delay(300);
            digitalWrite(led, 0);
            delay(300);
        }
    do
    {
        digitalWrite(led,1);
        delay(300);
        digitalWrite(led, 0);
        delay(300);
        var = 2;
    } while (var==23);
    
    while (contador<3) // 0 >> 0<3=V 1 >> 1<3=V 2 >> 2
    {1
        digitalWrite(led,);
        delay(300);
        digitalWrite(led, 0);
        delay(300);
        contador = contador + 1;
    }
    for (int i = 0; i < contador; i++)
    {
        digitalWrite(led,1);
        delay(300);
        digitalWrite(led, 0);
        delay(300);
    }
    
*/

    

}
// Metodos