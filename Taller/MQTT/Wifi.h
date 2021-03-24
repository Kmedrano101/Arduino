#include <ESP8266WiFi.h>

// Parametros
const char* ssid = "MIWIFI_2G_ZVsF";
const char* password = "xzQx27xq";
const char* mqtt_server = "192.168.1.200"; // IP del servidor Broker MQTT

// Metodos
void configurar_wifi()
{
  Serial.begin(115200); 
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
