
// ESTABLECER PINES DE ENTRADA
#define IN_SENSOR_S1 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_S2 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion
// ESTABLECER PINES DE SALIDA
#define OUT_STEP_M1 5
#define OUT_DIR_M1 6
#define OUT_STEP_M2 7
#define OUT_DIR_M2 8
#define OUT_ENABLE_M1 4

int Velocidades_MotorIZQ[7][4] = {{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
int Velocidades_MotorDER[7][3] = {{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
int Velocidades_Retardo[7] = {130,250,370,490,510,630,750};
int VelocidadRetardo = 340; // Usar minima resolucion
unsigned long tiempo_rev = 0;
unsigned long tiempo_revF = 0;
volatile int MOTOR_RUN = 0;
volatile int Posicion = 0;
int DireccionMotor = 1;
// DMODE0 > 0, DMODE1 > 0, DMODE2 > 1 ==> Paso Completo
// DMODE0 > 1, DMODE1 > 1, DMODE2 > 1 ==> 1/32 

int DMODE0 = 9;
int DMODE1 = 10;
int DMODE2 = 11;

// VARIABLES DE TIEMPO AUXILIARES
volatile unsigned long TH_TimeS1 = 0;
const int TH_Sensors = 120;

void setup() {
  // put your setup code here, to run once:
    pinMode(IN_SENSOR_S1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_S1), ISR_S1, FALLING);
    pinMode(OUT_STEP_M1, OUTPUT);
    pinMode(OUT_DIR_M1, OUTPUT);
    pinMode(OUT_STEP_M2, OUTPUT);
    pinMode(OUT_DIR_M2, OUTPUT);
    pinMode(OUT_ENABLE_M1, OUTPUT);
    pinMode(DMODE0, OUTPUT);
    pinMode(DMODE1, OUTPUT);
    pinMode(DMODE2, OUTPUT);
    Serial.begin(9600);
    digitalWrite(OUT_ENABLE_M1, 1);
    // Configuracoin de velocidad por pasos
    digitalWrite(DMODE0, 0);
    digitalWrite(DMODE1, 0);
    digitalWrite(DMODE2, 1);
    //MOTOR_RUN = 0;
}

void loop() {
  // Determinar tiempo de retardo minimo para cambio de pulso steps
    digitalWrite(OUT_DIR_M1, DireccionMotor);
    digitalWrite(OUT_DIR_M2, DireccionMotor);
    tiempo_rev = millis();
    Serial.println("Tiempo Inicial: ");
    Serial.print(tiempo_rev);
    if (!MOTOR_RUN)
    {
      for (long i = 0; i < 120000; i++)
      {
        digitalWrite(OUT_STEP_M1, 1);
        digitalWrite(OUT_STEP_M2, 1);
        digitalWrite(OUT_STEP_M1, 0);
        digitalWrite(OUT_STEP_M2, 0);
        delayMicroseconds(VelocidadRetardo);
        if (MOTOR_RUN)
        {
          MOTOR_RUN = 0;
          Posicion++;
          break;
        }
      }
    }
    //delayMicroseconds(340);
    cambiarVelocidad();
    tiempo_revF = millis();
    Serial.println("Tiempo Final: ");
    Serial.print(tiempo_revF);
    Serial.println("");
}
void ISR_S1()
{
  if((millis()-TH_TimeS1) > TH_Sensors){
    MOTOR_RUN = 1;
    digitalWrite(OUT_DIR_M1, 0);
    TH_TimeS1 = millis();
  }
}

void cambiarVelocidad()
{
  // Segun la direcion de los motores se cambia el sentido 
  if (Posicion>=10)
  {
    Posicion = 0;
  }
  
  if (DireccionMotor)
  {
    digitalWrite(DMODE0, Velocidades_MotorIZQ[Posicion][0]);
    digitalWrite(DMODE1, Velocidades_MotorIZQ[Posicion][1]);
    digitalWrite(DMODE2, Velocidades_MotorIZQ[Posicion][2]);
  }
  else
  {
    digitalWrite(DMODE0, Velocidades_MotorIZQ[Posicion][0]);
    digitalWrite(DMODE1, Velocidades_MotorIZQ[Posicion][2]);
    digitalWrite(DMODE2, Velocidades_MotorIZQ[Posicion][1]);
  }
  VelocidadRetardo = Velocidades_Retardo[Posicion];
}