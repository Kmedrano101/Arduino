/*
    Proyecto: Control de Motores Paso a Paso
    Desarrollador: Bittron
    Fecha: 24/03/2022
    Descripcion general: Version Motores V0.1

*/

/*
    - Validacoin de Motor en avance por secciones con contador de pasos y tiempo
    - Ajustar corriente maximo en controlador driver de motor
    - Notificacion de errores del driver, Bloqueo del funcionamiento en caso de error critico, caso contrario solo notificacion 
    - 60 RPM como velocidad maxima de cada motor

*/

// IMPORTAR LIBRERIAS
#include <EEPROM.h>
// ESTABLECER PINES DE ENTRADA
#define IN_SENSOR_S1 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_S2 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion
#define IN_ERROR_S1 10
#define IN_ERROR_S2 11
#define IN_ERROR_S3 12
#define IN_ERROR_S4 13
// ESTABLECER PINES DE SALIDA
#define OUT_STEP_M1 5
#define OUT_DIR_M1 6
#define OUT_STEP_M2 7
#define OUT_DIR_M2 8
#define OUT_ENABLE_M1 4
#define OUT_ENABLE_M2 9

// VARIABLES GLOBALES
struct Process_Values{
  unsigned int POSICION_CINTA = 0;
  unsigned long CONTADOR_PASOS_M1 = 0;
  unsigned long CONTADOR_PASOS_M2 = 0;
  unsigned int DIRECCION_MOTORES = 0; // 0 Direccion adelante, 1 direccion volver 
  int status=0; // Estado de funcionamiento, 1: En falla, 2: Normal y 3: Con errores no criticos (Puede seguir funcionando)
  unsigned long TIEMPO_PARADA = 15000; // Tiempo de paradas intermedias
};
Process_Values VALUES;
unsigned int EEPROM_DIR = 0;
unsigned int RPM = 60; // Para modificar valocidad
unsigned long TIEMPO_VELOCIDAD_M1[10] = {5,5,5,5,5,5,5,5,5,5}; // Tiempo en milisegundos
unsigned long TIEMPO_VELOCIDAD_M2[10] = {5,5,5,5,5,5,5,5,5,5}; // Tiempo en milisegundos
volatile bool MOTOR_RUN = false;
unsigned int TC_Velocidad = 120; // Variable de correccion de tiempo de retardo de ejecucion
const int TH_Sensors = 120;
volatile unsigned int Estado_Sen1 = 0;
volatile unsigned int Estado_Sen2 = 0;
unsigned int FError = 0;

// VARIABLES DE TIEMPO AUXILIARES
volatile unsigned long TH_TimeS1 = 0;
volatile unsigned long TH_TimeS2 = 0; 
unsigned long TM1 = 0;
unsigned long TM2 = 0;
unsigned long TParadas = 0;

void setup()
{
    pinMode(IN_SENSOR_S1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_S1), ISR_S1, FALLING);
    pinMode(IN_SENSOR_S2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_S2), ISR_S2, FALLING);
    pinMode(IN_ERROR_S1, INPUT_PULLUP);
    pinMode(IN_ERROR_S2, INPUT_PULLUP);
    pinMode(IN_ERROR_S3, INPUT_PULLUP);
    pinMode(IN_ERROR_S4, INPUT_PULLUP);
    pinMode(OUT_STEP_M1, OUTPUT);
    pinMode(OUT_DIR_M1, OUTPUT);
    pinMode(OUT_STEP_M2, OUTPUT);
    pinMode(OUT_DIR_M2, OUTPUT);
    pinMode(OUT_ENABLE_M1, OUTPUT);
    pinMode(OUT_ENABLE_M2, OUTPUT);
    Serial.begin(9600);
    digitalWrite(OUT_ENABLE_M1, 1);
    digitalWrite(OUT_ENABLE_M2, 1);
    MOTOR_RUN = true;
    restablecerValores();
    VALUES.DIRECCION_MOTORES = 0;
    VALUES.TIEMPO_PARADA = 2300;
    delay(2500); // Retardo de inicio
    Serial.println("Modo TEST v0.01");
}
void loop()
{
    MoverMotores();
    paradasMotor();
    determinarDireccionMotores();
}

// FUNCIONES STANDARD
void MoverMotores()
{
    // Acticar motores
    if(MOTOR_RUN)
    {
        // Activar direccion motores
        if(VALUES.DIRECCION_MOTORES)
        {
            digitalWrite(OUT_DIR_M1, 1);
            digitalWrite(OUT_DIR_M2, 1);
        }
        else
        {
            digitalWrite(OUT_DIR_M1, 0);
            digitalWrite(OUT_DIR_M2, 0);
        }
        
        if((millis() - TM1) > TIEMPO_VELOCIDAD_M1[VALUES.POSICION_CINTA]) // Para mayor presicion probar micros() > 5000
        {
            TM1 = millis();
            digitalWrite(OUT_STEP_M1, 1); // Validar sensibilidad de cambio 1 a 0 del driver
            delay(10);
            digitalWrite(OUT_STEP_M1,0);
            delay(10);
            VALUES.CONTADOR_PASOS_M1++;
        }
        if((millis() - TM2) > TIEMPO_VELOCIDAD_M2[VALUES.POSICION_CINTA])
        {
            TM2 = millis();
            digitalWrite(OUT_STEP_M2, 1); // Validar sensibilidad de cambio 1 a 0 del driver
            delay(10);
            digitalWrite(OUT_STEP_M2,0);
            delay(10);
            VALUES.CONTADOR_PASOS_M2++;
        }
    }
    else
    {
        BloquearMotores();
    }
    
}
void reiniciarProceso()
{
    // Reiniciar todos los valores y volver a estado inicial
}
void determinarDireccionMotores()
{
    // Cuando los dos sensores detectan es posicion final
    if (Estado_Sen1 == 1 && Estado_Sen2 == 1)
    {
        VALUES.DIRECCION_MOTORES = 1;
    }
    // Cuando el sensor 1 detecta es la posicion inicial
    if(Estado_Sen1 == 1 && Estado_Sen2 == 0) // Verificar estados de sensor en estado inicial y final
    {
        VALUES.DIRECCION_MOTORES = 0;
    }
}
void detertarFallas()
{
    if (digitalRead(IN_ERROR_S1) || digitalRead(IN_ERROR_S2) || digitalRead(IN_ERROR_S3) || digitalRead(IN_ERROR_S4))
    {
        BloquearMotores();
        FError = 1;
        // Deteminar el tipo de error para notificar 
    }
    
}
void paradasMotor()
{
    // Realizar las paradas intermedias de 15 segundos
    if(!MOTOR_RUN)
    {
        Serial.print("/nCantidad Pasos por Parada: ");
        Serial.print(VALUES.CONTADOR_PASOS_M1);
        if (!VALUES.DIRECCION_MOTORES)
        {   
            VALUES.POSICION_CINTA++;
        }
        else
        {
            VALUES.POSICION_CINTA--;
        }
        // Posible error de sensor no detecto parada
        if (VALUES.POSICION_CINTA > 10)
        {
            BloquearMotores();
        }
        // Agregar condicion segun sentido de los motores
        if (millis() - TParadas > VALUES.TIEMPO_PARADA)
        {
            TParadas = millis();
            MOTOR_RUN = true;
            VALUES.POSICION_CINTA++;
        } 
    }
}
void restablecerValores()
{
    // Restablecer valores de EEPROM
    EEPROM.get(EEPROM_DIR, VALUES);
}
void guardarValores()
{
    // Guardar valores en la EEPROM
    EEPROM.put(EEPROM_DIR, VALUES);
}
void BloquearMotores()
{
    // Apagar o bloquear Motores
    digitalWrite(OUT_ENABLE_M1, 0);
    digitalWrite(OUT_ENABLE_M2, 0);
    MOTOR_RUN = false;
}
// FUNCIONES ISR
void ISR_S1()
{
  if((millis()-TH_TimeS1) > TH_Sensors){
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS1 = millis();
    TParadas = millis();
    Estado_Sen1 = 1;
  }
}
void ISR_S2()
{
  if((millis()-TH_TimeS2) > TH_Sensors){
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS2 = millis();
    TParadas = millis();
    Estado_Sen2 = 1;
  }
}
