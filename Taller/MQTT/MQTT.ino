#include "Wifi.h"
#include <PubSubClient.h>
#include "DHT.h"
#define DHTTYPE DHT11
#define DHTPIN 2
#define LED 4
#define BOTON 16

// Instancias Wifi, cliente MQTT y DHT 
WiFiClient espClient;
PubSubClient client_mqtt(espClient);
DHT dht(DHTPIN, DHTTYPE);

// Variables globales
int temperatura;
char temperatura_mqtt[2];
int humedad;
char humedad_mqtt[2];
int estado_led;
char estado_led_mqtt[2];
int aux_boton;
unsigned long aux_tiempo;
unsigned long tiempo_actual;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BOTON, INPUT_PULLDOWN_16);
  pinMode(DHTPIN,INPUT);
  dht.begin();
  configurar_wifi();
  client_mqtt.setServer(mqtt_server, 1883);
  client_mqtt.setCallback(callback);
  client_mqtt.subscribe("casa/led_activar");
  delay(2000);
}

void loop() {
  if (!client_mqtt.connected()) {
    reconnect();
  }
  client_mqtt.loop();
  temperatura = (int)dht.readTemperature();     
  humedad = (int)dht.readHumidity();            
  control_led(LED);
  if(digitalRead(BOTON) && aux_boton==0)
  {
    estado_led = 1;
    aux_boton = 1;
    while(digitalRead(BOTON)){}
  }
  if(digitalRead(BOTON) && aux_boton==1)
  {
    estado_led = 0;
    aux_boton = 0;
    while(digitalRead(BOTON)){}
  }
  tiempo_actual = millis();
  if(tiempo_actual - aux_tiempo > 900) {
    aux_tiempo = tiempo_actual;
    snprintf(temperatura_mqtt,4,"%d",temperatura);
    client_mqtt.publish("casa/temperatura", temperatura_mqtt);
    snprintf(humedad_mqtt,4,"%d",humedad);
    client_mqtt.publish("casa/humedad", humedad_mqtt);
    snprintf(estado_led_mqtt,2,"%1d",estado_led);
    client_mqtt.publish("casa/led_estado", estado_led_mqtt);
  }                   
}
void control_led(int pin)
{

  digitalWrite(pin,estado_led);
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje Recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if((char)payload[0] == '1') {
    estado_led = 1;
  } else {
    estado_led = 0;
  }
}
void reconnect() {
  while (!client_mqtt.connected()) {
    Serial.print("MQTT connection...");
    if (client_mqtt.connect("ESP8266Client")) {
      Serial.println("connected");
      client_mqtt.publish("casa/led_estado", "0");
      client_mqtt.subscribe("casa/led_activar");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client_mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
