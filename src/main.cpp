/*
 _    _ _    _ __  __          _   _   _      ______  _____ _____ 
| |  | | |  | |  \/  |   /\   | \ | | | |    |  ____|/ ____/ ____|
| |__| | |  | | \  / |  /  \  |  \| | | |    | |__  | (___| (___  
|  __  | |  | | |\/| | / /\ \ | . ` | | |    |  __|  \___ \\___ \ 
| |  | | |__| | |  | |/ ____ \| |\  | | |____| |____ ____) |___) |
|_|  |_|\____/|_|  |_/_/    \_\_| \_| |______|______|_____/_____/ 
        
ESP8266 Master Con Audio i2s y comunicación esp_now
Honorino García Junio 2025
*/


#include <ESP8266WiFi.h>
#include <espnow.h>
#include <avdweb_Switch.h>
#include <ResponsiveAnalogRead.h>

#define Volumen A0

Switch BotonInicio (D4);
Switch BotonVolumen (D3);

ResponsiveAnalogRead Vol (Volumen, true);

float datosRaw = 0.0;
float datosSend = 0.0;


//Direccion MULTICAST
uint8_t broadcastAddress [] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//Estructura Mensaje
typedef struct struct_message{
  int play;
  float vol;
}struct_message;

struct_message myData;

//Funcion prototipo
float mapFloat (float x, float in_min, float in_max, float out_min, float out_max);

//CallBack Envío
void OnDataSent (uint8_t *mac_addr, uint8_t sendStatus){
  Serial.print ("ultimo paquete enviado: ");
  if(sendStatus == 0){
    Serial.println("Envio Correcto!");
  }
  else{
    Serial.println("Fallo Envío");
  }
}
/////////////////////////////


void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  //Inicio Esp-now
  if(esp_now_init() != 0){
    Serial.println("Error iniciando ESP_NOW");
    return;
  }
  else if(esp_now_init() == 0){
    Serial.println("ESP_NOW Iniciado");
  }

  esp_now_set_self_role (ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb (OnDataSent);

  //Registro de Pares
  esp_now_add_peer (broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

}

void loop() {
  BotonInicio.poll();
  BotonVolumen.poll();

  //Boton Play 
  if(BotonInicio.pushed()){
    Serial.println("Boton Play");
    myData.play = 1;
    esp_now_send (broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }


  //Boton Volumen
  if(BotonVolumen.pushed()){
    Vol.update();
    if(Vol.hasChanged()){
      datosRaw = Vol.getValue();
      datosSend = mapFloat(datosRaw, 0, 1023, 0, 1);
      Serial.print("Volumen: ");
      Serial.println(datosSend);
      myData.play = 3;
      myData.vol = datosSend;
      esp_now_send (broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    }
  }


  //Reset Pares
  if(BotonVolumen.longPress()){
    Serial.println("reset");
    myData.play = 9;
    esp_now_send (broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }
}
///////////////////////////////

//Función Entero to Float
float mapFloat (float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max -out_min) / (in_max - in_min) + out_min;
}