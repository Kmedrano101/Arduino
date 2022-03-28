// ESTABLECER PINES DE SALIDA
#define OUT_STEP_M1 5
#define OUT_DIR_M1 6
#define OUT_STEP_M2 7
#define OUT_DIR_M2 8
#define OUT_ENABLE_M1 4
#define OUT_ENABLE_M1 5

unsigned long tiempo_rev = 0;
unsigned long tiempo_revF = 0;
// DMODE0 > 0, DMODE1 > 0, DMODE2 > 1 ==> Paso Completo
// DMODE0 > 1, DMODE1 > 1, DMODE2 > 1 ==> 1/32 

int DMODE0 = 9;
int DMODE1 = 10;
int DMODE2 = 11;

void setup() {
  // put your setup code here, to run once:
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
}

void loop() {
  // Determinar tiempo de retardo minimo para cambio de pulso steps
    digitalWrite(OUT_DIR_M1, 1);
    digitalWrite(OUT_DIR_M1, 2);
    tiempo_rev = millis();
    Serial.print("/nTiempo Inicial:")
    Serial.print(tiempo_rev)
    digitalWrite(OUT_STEP_M1, 1);
    digitalWrite(OUT_STEP_M2, 1);
    delayMicroseconds(340);
    digitalWrite(OUT_STEP_M2, 0);
    digitalWrite(OUT_STEP_M2, 0);
    delayMicroseconds(340);
    tiempo_revF = millis();
    Serial.print("/nTiempo Final")
    Serial.print(tiempo_revF)
    Serial.println("")
}