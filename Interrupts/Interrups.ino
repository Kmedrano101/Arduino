#define IN_SENSOR_SINI 2 // Sensor Parada Final > Pin como entrada de Interrupcion
#define IN_SENSOR_SINT 3 // Sensor Parada Intermedias > Pin como entrada de Interrupcion

// VARIABLES DE TIEMPO AUXILIARES
volatile unsigned long TH_TimeS1 = 0;
volatile unsigned long TH_TimeS2 = 0;
const int TH_Sensors = 120;

void setup()
{
    pinMode(IN_SENSOR_SINI, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S2, FALLING); // Verficar porque falla interrupcion 
    pinMode(IN_SENSOR_SINT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IN_SENSOR_SINT), ISR_S2, FALLING);
    Serial.begin(9600);

}
void loop()
{
  Serial.println("Ejecutando...");
  delay(2400);
}
void ISR_S1()
{
  if((millis()-TH_TimeS1) > TH_Sensors){
    // Activar flags
    TH_TimeS1 = millis();
    Serial.println("Sensor 1 activado");
  }
}
void ISR_S2(){
  if((millis()-TH_TimeS2) > TH_Sensors){
    // Activar flags
    TH_TimeS2 = millis();
    Serial.println("Sensor 2 activado");
  }
}