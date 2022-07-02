/*
    Proyecto: Control de Motores Paso a Paso
    Desarrollador: Bittron
    Fecha: 15/06/2022
    Descripcion general: Version Motores V1.4
    Añadido SIM900 Ultima Modificación 22-06-22
*/


// IMPORTAR LIBRERIAS
#include <EEPROM.h>
#include <SoftwareSerial.h>
// ESTABLECER PINES DE ENTRADA
#define IN_SENSOR_SFIN 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_SINT 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion
#define OUT_STEP_M1 5 // MOTOR 1
#define OUT_DIR_M1 6 // MOTOR 1
#define OUT_STEP_M2 7 // MOTOR 2
#define OUT_DIR_M2 8 // MOTOR 2
#define IN_ERROR_M1_L1 18
#define IN_ERROR_M1_L2 19
#define IN_ERROR_M2_L1 20
#define IN_ERROR_M2_L2 21 
// ESTABLECER PINES DE SALIDA
#define OUT_ENABLE_M1 52  // Motor izquierda     (Validar pin de salida)
#define OUT_ENABLE_M2 50 // Motor derecha       (Validar pin de salida)
#define PIN_MOVIL 10
#define MOVIL_TX 14 //Arduino
#define MOVIL_RX 15 //Arduino
// DEFINIR PINES ACCELSTEPPER DRIVER
#include <AccelStepper.h>

// DEFINIR OBJETOS MOTORES PARA CONTROL
AccelStepper Motor1(AccelStepper::DRIVER, OUT_STEP_M1, OUT_DIR_M1);
AccelStepper Motor2(AccelStepper::DRIVER, OUT_STEP_M2, OUT_DIR_M2);
// Directivas para realizar debugging
//Serial.print(__FILE__); 
//  Serial.print(':');
#define DEBUGING // Quitar esta directiva para salir modo debug
#ifdef DEBUGING
#define DEBUG(x) \
  Serial.print("DEBUG: ");  \
  Serial.print(__LINE__); \
  Serial.print(' '); \
  Serial.println(x);
#else
#define DEBUG(x)
#endif

// VARIABLES GLOBALES
// Variables para guardar en memoria EEPROM
struct Process_Values {
  unsigned int POSICION_CINTA = 0;
  unsigned long CONTADOR_PASOS = 0;
  // long CONTADOR_PASOS_M2 = 0;
  unsigned int DIRECCION_MOTORES = 0; // 0 Direccion adelante, 1 direccion volver
  int status = 0; // Estado de funcionamiento, 1: En falla, 2: Normal y 3: Con errores no criticos (Puede seguir funcionando)
  unsigned long TIEMPO_PARADA = 2000; // Tiempo de paradas intermedias
  unsigned long PASOS_SECCION[10] = {120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000}; // Variable para calibrar y determinar la cantidad de pasos del motor hasta primera parada
  unsigned long BAUD_RATE = 9600;
  unsigned int NUMERO_PARADAS = 10; // 9 + 1(que es 0) = 10 paradas
};
Process_Values VALUES;
// Variables globales
unsigned int EEPROM_DIR = 0;
// Velocidad de ida y velocidad de vuelta
int VelocidadVuelta[10] = {41800, 38800, 3580, 3280, 2980, 2680, 2380, 2080, 1990, 1900}; // Cambio de velocidad. Cuanto mas alto el numero, mas lento el motor
int VelocidadIda[10] = {1900, 1990, 2080, 2380, 2680, 2980, 3280, 3580, 3880, 4180}; // Cambio de velocidad

volatile bool MOTOR_RUN = true;
const int TH_Sensors = 120;
volatile unsigned int Estado_Sen1 = 0;
volatile unsigned int Estado_Sen2 = 0;
volatile unsigned int FError = 0;
unsigned int AUX_CAMBIO_DIR = 1;
unsigned int AUX_SEN = 1;
unsigned int SEN1_ON = 0;
unsigned int SEN2_ON = 0;
unsigned int AUX_PARADA = 1;
unsigned int AUX_SENTIDO = 0;
volatile unsigned int Errores[5] = {0,0,0,0,0}; // Errores de drivers, {M1_L1,M1_L2,M2_L1,M2_L2,ParadaMaximo}
unsigned int AUX_LLAMANDO = 0;

// VARIABLES DE TIEMPO AUXILIARES
volatile unsigned long TH_TimeS1 = 0;
volatile unsigned long TH_TimeS2 = 0;
volatile unsigned long TH_TimeS3 = 0;
volatile unsigned long TH_TimeS4 = 0;
volatile unsigned long TH_TimeS5 = 0;
volatile unsigned long TH_TimeS6 = 0;
unsigned long TM1 = 0;
unsigned long TM2 = 0;
unsigned long TParadas = 0;
unsigned long TLlamada = 10000; // Tiempo de llamada
unsigned long TLlamada_Actual = 0;

unsigned long TMaximoParada = 0;

// Variables SIM900
SoftwareSerial SIM900(MOVIL_RX, MOVIL_TX); // Validar Pines
String TELEFONO = "+34635728722";

void setup()
{
  // Configuraciones Iniciales
  configurarEntradas();
  configurarSalidas();
  configuracionExtra();
}
void loop()
{
  //alertaFallas();
  moverMotores();
  determinarDireccionMotores();     
  // Condicion para reestablecer errores
}

// FUNCIONES STANDARD
void moverMotores()
{
  if (MOTOR_RUN)
  {
  // Activar direccion motores
    DEBUG(VALUES.POSICION_CINTA)
    AUX_PARADA = 1;
    digitalWrite(OUT_ENABLE_M1, 1);
    digitalWrite(OUT_ENABLE_M2, 1);
    if (!VALUES.DIRECCION_MOTORES)
    {
      // Segun la direccion se activan los pasos en positivo o negativo
      Motor1.moveTo(VALUES.PASOS_SECCION[VALUES.POSICION_CINTA]);
      Motor2.moveTo(VALUES.PASOS_SECCION[VALUES.POSICION_CINTA]);
      pasosMotores();
    }
    else
    {
      Motor1.moveTo(-VALUES.PASOS_SECCION[VALUES.POSICION_CINTA]);
      Motor2.moveTo(-VALUES.PASOS_SECCION[VALUES.POSICION_CINTA]);
      pasosMotores();
    }

  }
  else
  {
    BloquearMotores();
  }
}
void pasosMotores()
{
  for (long i = 0; i < VALUES.PASOS_SECCION[VALUES.POSICION_CINTA]; i++) // Determinar el maxino numero de pasos por seccion
  {
    Motor1.run();
    Motor2.run();
    DEBUG("Numero de pasos: ");
    DEBUG(i);
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
  Errores[0]=0;
  Errores[1]=0;
  Errores[2]=0;
  Errores[3]=0;
  Errores[4]=0;
}
void determinarDireccionMotores()
{
  // Cuando los dos sensores detectan es posicion final
  if (!MOTOR_RUN)
  {
    Estado_Sen1 = 0;
    Estado_Sen2 = 0;
    // Validacoin por conteo de posicion, prioridad sensores
    if (VALUES.POSICION_CINTA == VALUES.NUMERO_PARADAS)
    {
      VALUES.DIRECCION_MOTORES = 1;
    }
    if (VALUES.POSICION_CINTA == 0)
    {
      VALUES.DIRECCION_MOTORES = 0;
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
    }
  }

}
void alertaFallas()
{
    if (FError)
    {
        BloquearMotores();
        llamar();
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
void configurarSalidas() {
  pinMode(OUT_ENABLE_M1, OUTPUT);
  pinMode(OUT_ENABLE_M2, OUTPUT);
  pinMode(PIN_MOVIL, OUTPUT);
}
void configurarEntradas() {
  pinMode(IN_SENSOR_SFIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SFIN), ISR_S1_SFIN, FALLING); 
  pinMode(IN_SENSOR_SINT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S2_SINT, FALLING);
  pinMode(IN_ERROR_M1_L1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S3_M1L1, FALLING);
  pinMode(IN_ERROR_M1_L2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S4_M1L2, FALLING);
  pinMode(IN_ERROR_M2_L1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S5_M2L1, FALLING);
  pinMode(IN_ERROR_M2_L2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S6_M2L2, FALLING);
}
void configuracionExtra() {
  // CONFIGURAR MOTOR 1
  Motor1.setMaxSpeed(200.0);
  Motor1.setAcceleration(200.0);
  Motor1.moveTo(VALUES.PASOS_SECCION[0]);
  // CONFIGURAR MOTOR 2
  Motor2.setMaxSpeed(200.0);
  Motor2.setAcceleration(200.0);
  Motor2.moveTo(VALUES.PASOS_SECCION[0]);
  Serial.begin(VALUES.BAUD_RATE);
  SIM900.begin(19200);  // Iniciar comunicación SIM
  DEBUG("Modo Test 3.0");
  delay(1500); // Retardo de inicio
}
void BloquearMotores()
{
  // Apagar o bloquear Motores
  digitalWrite(OUT_ENABLE_M1, 0);
  digitalWrite(OUT_ENABLE_M2, 0);
  Motor1.stop();
  Motor2.stop();
  MOTOR_RUN = false;
}
void llamar(){
  // Función que permite llamar a un celular local
  digitalWrite(PIN_MOVIL, 1);
  // Agregar salida del bucle en caso de no respuesta del modulo SIM en X tiempo
  while(!SIM900.available())
  {
      DEBUG(".");
  }
  if (!AUX_LLAMANDO)
  {
    DEBUG("Encendiendo Movil");
    delay(200);
    SIM900.println("AT");
    DEBUG("Esperando red");
    delay(1500);
    DEBUG("Conectando con operador");
    SIM900.println("AT+COPS?");
    delay(500);
    DEBUG("Llamando");
    SIM900.println("ATD" + TELEFONO + ";"); // Comando AT para realizar una llamada    /* code */
    AUX_LLAMANDO = 1;
  }
  if (millis() - TLlamada_Actual > TLlamada)
  {
    SIM900.println("ATH"); // Cuelga la llamada
    delay(200);
    DEBUG("Llamada finalizada");
    digitalWrite(PIN_MOVIL, 0);
    TLlamada_Actual = millis();
    AUX_LLAMANDO = 0;    
  } 
}
// FUNCIONES ISR
void ISR_S1_SFIN()
{
  if ((millis() - TH_TimeS1) > TH_Sensors) {
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS1 = millis();
    TParadas = millis();
    Estado_Sen1 = 1;
  }
}
void ISR_S2_SINT()
{
  if ((millis() - TH_TimeS2) > TH_Sensors) {
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS2 = millis();
    TParadas = millis();
    Estado_Sen2 = 1;
  }
}
void ISR_S3_M1L1()
{
  if ((millis() - TH_TimeS3) > TH_Sensors) {
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS3 = millis();
    TLlamada_Actual = millis();
    Errores[0] = 1;
    FError = 1;
  }
}
void ISR_S4_M1L2()
{
  if ((millis() - TH_TimeS4) > TH_Sensors) {
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS4 = millis();
    TLlamada_Actual = millis();
    Errores[1] = 1;
    FError = 1;
  }
}
void ISR_S5_M2L1()
{
  if ((millis() - TH_TimeS5) > TH_Sensors) {
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS5 = millis();
    TLlamada_Actual = millis();
    Errores[2] = 1;
    FError = 1;
  }
}
void ISR_S6_M2L2()
{
  if ((millis() - TH_TimeS6) > TH_Sensors) {
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS6 = millis();
    TLlamada_Actual = millis();
    Errores[3] = 1;
    FError = 1;
  }
}