/*
    Proyecto: Control de Motores Paso a Paso
    Desarrollador: Bittron
    Fecha: 04/04/2022
    Descripcion general: Version Motores V1.0

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
#define IN_SENSOR_SFIN 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_SINT 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion
#define IN_ERROR_S1 10
#define IN_ERROR_S2 11
#define IN_ERROR_S3 12
#define IN_ERROR_S4 13
// ESTABLECER PINES DE SALIDA
#define OUT_STEP_MIZQ 5
#define OUT_DIR_MIZQ 6
#define OUT_STEP_MDER 7
#define OUT_DIR_MDER 8
#define OUT_ENABLE_M1 4  // Motor izquierda     (Validar pin de salida)
#define OUT_ENABLE_M2 9 // Motor derecha       (Validar pin de salida)

// Directivas para realizar debugging
#define DEBUG // Quitar esta directiva para salir modo debug
#ifdef DEBUG
#define DEBUG(x) \
  Serial.print("DEBUG: ");  \
  Serial.print(__FILE__); \
  Serial.print(':'); \
  Serial.print(__LINE__); \
  Serial.print(' '); \
  Serial.println(x);
#else
#define DEBUG(x) 
#endif

// VARIABLES GLOBALES
// Variables para guardar en memoria EEPROM
struct Process_Values{
  unsigned int POSICION_CINTA = 0;
  unsigned long CONTADOR_PASOS = 0;
  // long CONTADOR_PASOS_M2 = 0;
  unsigned int DIRECCION_MOTORES = 0; // 0 Direccion adelante, 1 direccion volver 
  int status=0; // Estado de funcionamiento, 1: En falla, 2: Normal y 3: Con errores no criticos (Puede seguir funcionando)
  unsigned long TIEMPO_PARADA = 1500; // Tiempo de paradas intermedias
  unsigned long PASOS_SECCION[10] = {120000,120000,120000,120000,120000,120000,120000,120000,120000,120000}; // Variable para calibrar y determinar la cantidad de pasos del motor hasta primera parada
  unsigned long BAUD_RATE = 9600;
};
Process_Values VALUES;
// Variables globales
unsigned int EEPROM_DIR = 0;
int Retardos[10] = {780,790,1280,1680,1880,2080,2280,2680,2880,3780}; // Cambio de velocidad
volatile bool MOTOR_RUN = true;
const int TH_Sensors = 120;
volatile unsigned int Estado_Sen1 = 0;
volatile unsigned int Estado_Sen2 = 0;
unsigned int FError = 0;
unsigned int AUX_CAMBIO_DIR = 1;
unsigned int AUX_SEN = 1;
unsigned int SEN1_ON = 0;
unsigned int SEN2_ON = 0;
unsigned int AUX_PARADA = 1;
unsigned int AUX_SENTIDO = 0;

// VARIABLES DE TIEMPO AUXILIARES
volatile unsigned long TH_TimeS1 = 0;
volatile unsigned long TH_TimeS2 = 0; 
unsigned long TM1 = 0;
unsigned long TM2 = 0;
unsigned long TParadas = 0;

void setup()
{
    // Configuraciones Iniciales
    configurarEntradas();
    configurarSalidas();
    configuracionExtra();
}
void loop()
{
    moverMotores();
    determinarDireccionMotores();
}

// FUNCIONES STANDARD
void moverMotores()
{
    // Acticar motores
    if(MOTOR_RUN)
    {
        // Activar direccion motores
        DEBUG(VALUES.POSICION_CINTA)
        AUX_PARADA = 1;
        if(!VALUES.DIRECCION_MOTORES)
        {
            digitalWrite(OUT_ENABLE_M1, 0);
            digitalWrite(OUT_ENABLE_M2, 1);
            digitalWrite(OUT_DIR_MDER, 1);
            pasosMotores(OUT_STEP_MDER);
        }
        else
        {
            digitalWrite(OUT_ENABLE_M1, 1);
            digitalWrite(OUT_ENABLE_M2, 0);
            digitalWrite(OUT_DIR_MIZQ, 0);
            digitalWrite(OUT_DIR_MDER, 0);
            pasosMotores(OUT_STEP_MIZQ);
        }
        
        // Notificar Exeption
        // En caso de terminar for y no detectar sensores para apagar los motores
    }
    else
    {
        BloquearMotores();
    }
}
void pasosMotores(int Motor)
{
    for (long i = 0; i < VALUES.PASOS_SECCION[VALUES.POSICION_CINTA]; i++) // Determinar el maxino numero de pasos por seccion
        {
            digitalWrite(Motor, 1);
            delayMicroseconds(720);
            digitalWrite(Motor, 0);
            delayMicroseconds(Retardos[VALUES.POSICION_CINTA]);
            VALUES.CONTADOR_PASOS++;
            if (!MOTOR_RUN)
            {
                TParadas = millis();
                AUX_CAMBIO_DIR = 1;
                break;
            }
        }
}
void reiniciarProceso()
{
    // Reiniciar todos los valores y volver a estado inicial
}
void determinarDireccionMotores()
{
    // Cuando los dos sensores detectan es posicion final
    if(!MOTOR_RUN)
    {
        // VERIFICAR ESTO !!! Posicion real de los sensores
        /*if (Estado_Sen1 == 1 && Estado_Sen2 == 1)
        {
            VALUES.DIRECCION_MOTORES = 1;
            AUX_SENTIDO = 1;
            AUX_SEN = 0;
        }
        // Cuando el sensor 1 detecta es la posicion inicial
        if(Estado_Sen1 == 0 && Estado_Sen2 == 1 && AUX_SENTIDO == 0) // Verificar estados de sensor en estado inicial y final
        {
            VALUES.DIRECCION_MOTORES = 0;
            AUX_SEN = 0;
        }*/
        Estado_Sen1 = 0;
        Estado_Sen2 = 0;
        // Validacoin por conteo de posicion, prioridad sensores
        if(VALUES.POSICION_CINTA==9)
        {
            VALUES.DIRECCION_MOTORES = 1;
        }
        if(VALUES.POSICION_CINTA==0)
        {
            VALUES.DIRECCION_MOTORES = 0;
            AUX_SENTIDO = 0;
        }
        if (AUX_CAMBIO_DIR)
        {
            if (!VALUES.DIRECCION_MOTORES)
                {   
                    VALUES.POSICION_CINTA++;
                }
            else
                {
                    VALUES.POSICION_CINTA--;
                }
            AUX_CAMBIO_DIR = 0;
        }
        // Posible error de sensor no detecto parada
        if (VALUES.POSICION_CINTA > 9)
        {
            BloquearMotores();
        }
        // Agregar condicion segun sentido de los motores
        if (millis() - TParadas > VALUES.TIEMPO_PARADA)
        {
            TParadas = millis();
            MOTOR_RUN = true;
            AUX_SEN = 1;
        }
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
void configurarSalidas(){
    pinMode(OUT_STEP_MIZQ, OUTPUT);
    pinMode(OUT_DIR_MIZQ, OUTPUT);
    pinMode(OUT_STEP_MDER, OUTPUT);
    pinMode(OUT_DIR_MDER, OUTPUT);
    pinMode(OUT_ENABLE_M1, OUTPUT);
    pinMode(OUT_ENABLE_M2, OUTPUT);
}
void configurarEntradas(){
    pinMode(IN_SENSOR_SFIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SFIN), ISR_S1, FALLING); // Verficar porque falla interrupcion 
    pinMode(IN_SENSOR_SINT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S2, FALLING);
    pinMode(IN_ERROR_S1, INPUT_PULLUP);
    pinMode(IN_ERROR_S2, INPUT_PULLUP);
    pinMode(IN_ERROR_S3, INPUT_PULLUP);
    pinMode(IN_ERROR_S4, INPUT_PULLUP);
}
void configuracionExtra(){
    Serial.begin(VALUES.BAUD_RATE);
    //restablecerValores();
    //VALUES.DIRECCION_MOTORES = 0;
    //VALUES.TIEMPO_PARADA = 2300;
    delay(1500); // Retardo de inicio
    DEBUG("Modo TEST v1.0");
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

