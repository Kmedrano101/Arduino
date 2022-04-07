/*
    Proyecto: Control de Motores Paso a Paso
    Desarrollador: Bittron
    Fecha: 07/04/2022
    Descripcion general: Version Motores V1.01

*/

/*
    - Validacoin de Motor en avance por secciones con contador de pasos y tiempo
    - Ajustar corriente maximo en controlador driver de motor
    - Notificacion de errores del driver, Bloqueo del funcionamiento en caso de error critico, caso contrario solo notificacion
    - 60 RPM como velocidad maxima de cada motor

*/

// IMPORTAR LIBRERIAS
#include <EEPROM.h>
#include <SoftwareSerial.h>
// ESTABLECER PINES DE ENTRADA
#define IN_SENSOR_SFIN 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_SINT 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion
#define IN_ERROR_M1_L1 18
#define IN_ERROR_M1_L2 19
#define IN_ERROR_M2_L1 20
#define IN_ERROR_M2_L2 21
// ESTABLECER PINES DE SALIDA
#define OUT_STEP_MIZQ 5
#define OUT_DIR_MIZQ 6
#define OUT_STEP_MDER 7
#define OUT_DIR_MDER 8
#define OUT_ENABLE_M1 22  // Motor izquierda     (Validar pin de salida)
#define OUT_ENABLE_M2 23 // Motor derecha       (Validar pin de salida)
#define PIN_MOVIL 10
#define MOVIL_TX 18
#define MOVIL_RX 19

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
  unsigned long TIEMPO_PARADA = 1500; // Tiempo de paradas intermedias
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
volatile unsigned int Errores[4] = {0,0,0,0}; // Errores de drivers, {M1_L1,M1_L2,M2_L1,M2_L2}

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

// Variables SIM900
SoftwareSerial SIM900(MOVIL_RX, MOVIL_TX); // Validar Pines
String TELEFONO = "635728722";

void setup()
{
  // Configuraciones Iniciales
  configurarEntradas();
  configurarSalidas();
  configuracionExtra();
}
void loop()
{
  if (!FError)
  {
    moverMotores();
    determinarDireccionMotores();     
  }
  // Condicion para reestablecer errores
}

// FUNCIONES STANDARD
void moverMotores()
{
  // Acticar motores
  if (MOTOR_RUN)
  {
    // Activar direccion motores
    DEBUG(VALUES.POSICION_CINTA)
    AUX_PARADA = 1;
    if (!VALUES.DIRECCION_MOTORES)
    {
      digitalWrite(OUT_ENABLE_M1, 0);
      digitalWrite(OUT_ENABLE_M2, 1);
      digitalWrite(OUT_DIR_MDER, 1);
      pasosMotores(OUT_STEP_MDER, VelocidadIda[VALUES.POSICION_CINTA]);
    }
    else
    {
      digitalWrite(OUT_ENABLE_M1, 1);
      digitalWrite(OUT_ENABLE_M2, 0);
      digitalWrite(OUT_DIR_MIZQ, 0);
      digitalWrite(OUT_DIR_MDER, 0);
      pasosMotores(OUT_STEP_MIZQ, VelocidadVuelta[VALUES.POSICION_CINTA]);
    }

    // Notificar Exeption
    // En caso de terminar for y no detectar sensores para apagar los motores
  }
  else
  {
    BloquearMotores();
  }
}
void pasosMotores(int Motor, long retardo)
{
  for (long i = 0; i < VALUES.PASOS_SECCION[VALUES.POSICION_CINTA]; i++) // Determinar el maxino numero de pasos por seccion
  {
    digitalWrite(Motor, 1);
    delayMicroseconds(720);
    digitalWrite(Motor, 0);
    delayMicroseconds(retardo);
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
}
void determinarDireccionMotores()
{
  // Cuando los dos sensores detectan es posicion final
  if (!MOTOR_RUN)
  {
    // Deteccio de ambos sensores para cambiar sentido
    if (Estado_Sen1 == 1 && Estado_Sen2 == 1)
      {
        VALUES.DIRECCION_MOTORES = 1;
      }
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
  pinMode(OUT_STEP_MIZQ, OUTPUT);
  pinMode(OUT_DIR_MIZQ, OUTPUT);
  pinMode(OUT_STEP_MDER, OUTPUT);
  pinMode(OUT_DIR_MDER, OUTPUT);
  pinMode(OUT_ENABLE_M1, OUTPUT);
  pinMode(OUT_ENABLE_M2, OUTPUT);
  pinMode(PIN_MOVIL, OUTPUT);
}
void configurarEntradas() {
  pinMode(IN_SENSOR_SFIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SFIN), ISR_S1_SFIN, FALLING); // Verficar porque falla interrupcion
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
  Serial.begin(VALUES.BAUD_RATE);
  delay(1500); // Retardo de inicio
  SIM900.begin(19200);  // Iniciar comunicación SIM
  DEBUG("Modo TEST v1.1");
}
void BloquearMotores()
{
  // Apagar o bloquear Motores
  digitalWrite(OUT_ENABLE_M1, 0);
  digitalWrite(OUT_ENABLE_M2, 0);
  MOTOR_RUN = false;
}
void llamar(){
  // Función que permite llamar a un celular local
  while(!SIM900.available())
  {
      DEBUG(".");
  }
  digitalWrite(PIN_MOVIL, 1);    
  DEBUG("Encendiendo Movil");
  delay(200);
  SIM900.println("AT");
  DEBUG("Esperando red");
  delay(1500);
  DEBUG("Conectando con operador");
  SIM900.println("AT+COPS?");
  delay(500);
  DEBUG("Llamando");
  SIM900.println("ATD" + TELEFONO + ";"); // Comando AT para realizar una llamada
  if (millis() - TLlamada_Actual > TLlamada)
  {
    SIM900.println("ATH"); // Cuelga la llamada
    delay(200);
    DEBUG("Llamada finalizada");
    digitalWrite(PIN_MOVIL, 0);
    TLlamada_Actual = millis();    
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