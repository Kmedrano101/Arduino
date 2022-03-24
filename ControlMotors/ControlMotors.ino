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

// ESTABLECER PINES DE ENTRADA
#define IN_SENSOR_S1 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_S2 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion

// ESTABLECER PINES DE SALIDA
#define OUT_STEP_M1 4
#define OUT_DIR_M1 5
#define OUT_STEP_M2 6
#define OUT_DIR_M2 7
#define OUT_ENABLE_M1 8
#define OUT_ENABLE_M2 9

// VARIABLES GLOBALES
unsigned int RPM = 60; // Para modificar valocidad
unsigned int POSICION_CINTA = 0;
unsigned long TIEMPO_VELOCIDAD_M1[10] = {5,5,5,5,5,5,5,5,5,5}; // Tiempo en milisegundos
unsigned long TIEMPO_VELOCIDAD_M2[10] = {5,5,5,5,5,5,5,5,5,5}; // Tiempo en milisegundos
unsigned long CONTADOR_PASOS_M1 = 0;
unsigned long CONTADOR_PASOS_M2 = 0;
volatile bool MOTOR_RUN = false;
unsigned int DIRECCION_MOTORES = 0; // 0 Direccion adelante, 1 direccion volver
unsigned int TC_Velocidad = 120; // Variable de correccion de tiempo de retardo de ejecucion
const int TH_Sensors = 150;

// VARIABLES DE TIEMPO AUXILIARES
volatile unsigned long TH_TimeS1 = 0;
volatile unsigned long TH_TimeS2 = 0; 
unsigned long TM1 = 0;
unsigned long TM2 = 0;

void setup()
{
    pinMode(IN_SENSOR_S1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_S1), ISR_S1, FALLING);
    pinMode(IN_SENSOR_S2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_S2), ISR_S2, FALLING);
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
    delay(2500); // Retardo de inicio
}
void loop()
{
    
}

// FUNCIONES STANDARD
void MoverMotores()
{
    // Acticar motores
    if(MOTOR_RUN)
    {
        // Activar direccion motores
        if(DIRECCION_MOTORES)
        {
            digitalWrite(OUT_DIR_M1, 1);
            digitalWrite(OUT_DIR_M2, 1);
        }
        else
        {
            digitalWrite(OUT_DIR_M1, 0);
            digitalWrite(OUT_DIR_M2, 0);
        }
        
        if((millis() - TM1) > TIEMPO_VELOCIDAD_M1[POSICION_CINTA])
        {
            TM1 = millis();
            digitalWrite(OUT_STEP_M1, 1); // Validar sensibilidad de cambio 1 a 0 del driver
            digitalWrite(OUT_STEP_M1,0);
            CONTADOR_PASOS_M1++;
        }
        if((millis() - TM2) > TIEMPO_VELOCIDAD_M2[POSICION_CINTA])
        {
            TM2 = millis();
            digitalWrite(OUT_STEP_M2, 1); // Validar sensibilidad de cambio 1 a 0 del driver
            digitalWrite(OUT_STEP_M2,0);
            CONTADOR_PASOS_M2++;
        }
    }
    else
    {
        BloquearMotores();
        MOTOR_RUN = false;
    }
    
}
void BloquearMotores()
{
    // Apagar o bloquear Motores
    digitalWrite(OUT_ENABLE_M1, 0);
    digitalWrite(OUT_ENABLE_M2, 0);
}
// FUNCIONES ISR
void ISR_S1()
{
  if((millis()-TH_TimeS1) > TH_Sensors){
    // Activar flags
    TH_TimeS1 = millis();
  }
}
void ISR_S2()
{
  if((millis()-TH_TimeS2) > TH_Sensors){
    // Activar flags
    TH_TimeS2 = millis();
  }
}
