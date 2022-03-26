// ESTABLECER PINES DE SALIDA
#define OUT_STEP_M1 5
#define OUT_DIR_M1 6
#define OUT_STEP_M2 7
#define OUT_DIR_M2 8
#define OUT_ENABLE_M1 4

void setup() {
  // put your setup code here, to run once:
    pinMode(OUT_STEP_M1, OUTPUT);
    pinMode(OUT_DIR_M1, OUTPUT);
    pinMode(OUT_STEP_M2, OUTPUT);
    pinMode(OUT_DIR_M2, OUTPUT);
    pinMode(OUT_ENABLE_M1, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
    digitalWrite(OUT_DIR_M1, 1);
    digitalWrite(OUT_DIR_M1, 2);
    for (int i = 0; i < 200; i++)
    {
        digitalWrite(OUT_STEP_M1, 1);
        digitalWrite(OUT_STEP_M2, 1);
        delay(5);
        digitalWrite(OUT_STEP_M2, 0);
        digitalWrite(OUT_STEP_M2, 0);
        delay(5);
    }
}