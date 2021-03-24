/*
  Descripción: Proyecto de Protocolo MQTT para Internet de las Cosas con ESP8266 y Node-RED
  Autor: Kevin Medrano Ayala
  Fecha: XX/XX/XX
*/
// Directivas de preprocesador
#include <PubSubClient.h>
#define LED 4
#define BOTON 16
// Variables, estructuras, declaraciones
 
const char* ssid = "MIWIFI_2G_ZVsF";
const char* password = "xzQx27xq";
const char* mqtt_server = "192.168.1.200";

WiFiClient espClient;

PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

int Estado_Led = 0;
int aux_boton = 0;
unsigned long auxTiempo=0;


void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BOTON, INPUT_PULLDOWN_16);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(2000);
}

// the loop function runs over and over again forever
void loop() {
  if(digitalRead(BOTON) && aux_boton==0)
  {
    Estado_Led = 1;
    aux_boton = 1;
    while(digitalRead(BOTON)){}
  }
  if(digitalRead(BOTON) && aux_boton==1)
  {
    Estado_Led = 0;
    aux_boton = 0;
    while(digitalRead(BOTON)){}
  }
  control_Led(LED);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.subscribe("casa/led1_active");
  //client.subscribe("casa/led1_1");
  unsigned long now = millis();
  if(now - lastMsg > 900) {
    lastMsg = now;
    if (Estado_Led)
    {
      client.publish("casa/led1_state", "1");
    }
    else
    {
      client.publish("casa/led1_state", "0");
    }
    
  }                   
}

void control_Led(int pin)
{
  digitalWrite(pin,Estado_Led);
}
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Switch on the LED if an 1 was received as first character
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == '1') {
    Estado_Led = 1;   // Turn the LED on (Note that LOW is the voltage level
  }if ((char)payload[0] == '0') {
    Estado_Led = 0;   // Turn the LED on (Note that LOW is the voltage level
  }

}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.publish("casa/led1_state", "0");
      // ... and resubscribe
      client.subscribe("casa/led1_active");
      //client.subscribe("casa/led1_1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}