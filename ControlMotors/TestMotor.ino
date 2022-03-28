
#define IN_SENSOR_S1 2
// ESTABLECER PINES DE SALIDA
#define OUT_STEP_M1 5
#define OUT_DIR_M1 6
#define OUT_STEP_M2 7
#define OUT_DIR_M2 8
#define OUT_ENABLE_M1 4
#define OUT_ENABLE_M1 5

unsigned long tiempo_rev = 0;
unsigned long tiempo_revF = 0;
volatile bool MOTOR_RUN = false;
// DMODE0 > 0, DMODE1 > 0, DMODE2 > 1 ==> Paso Completo
// DMODE0 > 1, DMODE1 > 1, DMODE2 > 1 ==> 1/32 

int DMODE0 = 9;
int DMODE1 = 10;
int DMODE2 = 11;

// VARIABLES DE TIEMPO AUXILIARES
volatile unsigned long TH_TimeS1 = 0;

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
    MOTOR_RUN = true;
}

void loop() {
  // Determinar tiempo de retardo minimo para cambio de pulso steps
    digitalWrite(OUT_DIR_M1, 1);
    digitalWrite(OUT_DIR_M1, 2);
    tiempo_rev = millis();
    Serial.print("/nTiempo Inicial:")
    Serial.print(tiempo_rev)
    while (MOTOR_RUN)
    {
      digitalWrite(OUT_STEP_M1, 1);
      digitalWrite(OUT_STEP_M2, 1);
      delayMicroseconds(340);
      digitalWrite(OUT_STEP_M2, 0);
      digitalWrite(OUT_STEP_M2, 0);
    }
    //delayMicroseconds(340);
    tiempo_revF = millis();
    Serial.print("/nTiempo Final")
    Serial.print(tiempo_revF)
    Serial.println("")
}
void ISR_S1()
{
  if((millis()-TH_TimeS1) > TH_Sensors){
    // Activar flags
    MOTOR_RUN = false;
    TH_TimeS1 = millis();
  }
}