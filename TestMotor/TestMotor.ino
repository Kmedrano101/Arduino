
/* Test Step Motor with driver
  Date: 29/06/2022
*/
// ESTABLECER PINES DE ENTRADA
#define IN_SENSOR_S1 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_S2 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion
// ESTABLECER PINES DE SALIDA
#define OUT_STEP_M1 5
#define OUT_DIR_M1 6
#define OUT_STEP_M2 7
#define OUT_DIR_M2 8
#define OUT_ENABLE_M1 4
// DEFINIR PINES ACCELSTEPPER DRIVER
#include <AccelStepper.h>
// MOTOR 1
#define OUT_STEP_M1 5
#define OUT_DIR_M1 6
// MOTOR 2
#define OUT_STEP_M2 7
#define OUT_DIR_M2 8
// DEFINIR OBJETOS MOTORES PARA CONTROL
AccelStepper Motor1(AccelStepper::DRIVER, OUT_STEP_M1, OUT_DIR_M1);
AccelStepper Motor2(AccelStepper::DRIVER, OUT_STEP_M2, OUT_DIR_M2);
// VARIABLES
unsigned long PASOS_SECCION[10] = {120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000, 120000};
unsigned long TIEMPO_PARADA = 2000;

void setup() {
    pinMode(OUT_ENABLE_M1, OUTPUT);
    Serial.begin(9600);
    digitalWrite(OUT_ENABLE_M1, 1);
    // CONFIGURAR MOTOR 1
    Motor1.setMaxSpeed(200.0);
    Motor1.setAcceleration(200.0);
    Motor1.moveTo(PASOS_SECCION[0]);
    // CONFIGURAR MOTOR 2
    Motor2.setMaxSpeed(200.0);
    Motor2.setAcceleration(200.0);
    Motor2.moveTo(PASOS_SECCION[0]);
}

void loop() {
    Motor1.run();
    Motor2.run();
}
