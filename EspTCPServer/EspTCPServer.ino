#include <WiFiServer.h>
#include <esp_wifi.h>

WiFiServer sv(555);
WiFiClient cl;

void setup(){
  serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("EspW1LSChulz", "")
  sv.begin();
  Serial.println("Servidor TCP iniciado.");
}


void setup() {


}
void tcp(){
  if(cl.connected()){
    if(cl.available() > 0){
      String req = "";
      while (cl.available() > 0){
        char z = cl.read();
        req +=z;
      }

      Serial.print("\nUm cliente enviou uma mensagem");
      Serial.print("\n...IP do cliente: ");
      Serial.print(cl.remoteIP());
      Serial.print("\n...IP do servidor: ");
      Serial.print(WiFi.softAPIP());
      Serial.print("...Mensagem do cliente: "+ req + "\n");

      
    }
  }
  
  else{
    cl = sv.available();
    delay(1);

  }
}
void loop() {
  // put your main code here, to run repeatedly:

}
