#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// -----------------------------
// display
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

#define bt_plus 19
#define bt_less 18
#define bt_state 5

int R = 80;
int B = 56;
int G = 75;
int INT = 12;

int estado_valor = 0;

//----------------------------------------------

const char* ssid = "Mi Ada";  
const char* password = "telefono4ada"; 

WebServer server(80);  
void handle_root();

struct Button { //struct del pulsador
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};
 // seleccionamos el pin donde está el pulsador
Button buttonsum = {5, 0, false};
Button buttonresta = {18, 0, false};
Button buttoncambio = {17, 0, false};

void IRAM_ATTR isr() { //funcion para que la interrupción sea corta 
 if(digitalRead(buttonsum.PIN)==LOW) {
    buttonsum.numberKeyPresses += 1;
    Serial.println("boton suma apretado");
 }
  else if(digitalRead(buttonresta.PIN)==LOW) {
    buttonresta.numberKeyPresses += 1;
    Serial.println("boton resta apretado");
 }
 else if(digitalRead(buttoncambio.PIN)==LOW) {
    buttoncambio.numberKeyPresses += 1;
    Serial.println("boton cambio apretado");
 }

}

void setup() {
    // WIFI
    Serial.begin(115200);
    Serial.println("Try Connecting to ");
    Serial.println(ssid);


    WiFi.begin(ssid, password);


    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected successfully");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());  

    server.on("/", handle_root);

    server.begin();
    Serial.println("HTTP server started");
    delay(100); 

    //-------------------------------------------------------

    //BOTONES

    Serial.begin(115200);
    pinMode(buttonsum.PIN, INPUT_PULLUP);
    attachInterrupt(buttonsum.PIN, isr, FALLING); //interrupción en base al pin por pin, button1.PIN= clavija que hace la interrupción y debe monitorizar, 
    //isr= funcion que llama para hacer la interrupción, FALLING=los disparadores interrumpen cuando el pin va de HIGH a LOW
    pinMode(buttonresta.PIN, INPUT_PULLUP);
    attachInterrupt(buttonresta.PIN, isr, FALLING);

        pinMode(buttoncambio.PIN, INPUT_PULLUP);
    attachInterrupt(buttoncambio.PIN, isr, FALLING);


    //--------------- ------------------------------
    // display
    Serial.begin(115200);
    LCD.init();
    LCD.backlight();




    pinMode(bt_plus, INPUT_PULLUP);
    pinMode(bt_less, INPUT_PULLUP);
    pinMode(bt_state, INPUT_PULLUP);


    // Inicializar la pantalla en el estado inicial
    actualizarPantalla(R, B, G, INT, ssid, password);
    //---------------------------------------------

}

void loop() {
 server.handleClient();

 //------------ display-------------------------

   if (digitalRead(bt_plus) == LOW) {
    estado_valor++; // Aumentar el valor en 1
    if (estado_valor == 5) {
      estado_valor = 1; // Si llega a 5, volver a 1
    }
    if (estado_valor == 14) {
      estado_valor = 11; // Si llega a 14, volver a 11
    }
    actualizarPantalla(R, B, G, INT, ssid, password);
  }
  if (digitalRead(bt_less) == LOW) {
    estado_valor=estado_valor-1; // disminueix el valor en 1
    if (estado_valor < 0) {
      estado_valor = 4; // Si llega a 5, volver a 1
    }
    if (estado_valor < 10 && estado_valor > 8) {
      estado_valor = 13; // Si llega a 14, volver a 11
    }
    actualizarPantalla(R, B, G, INT, ssid, password);
  }
  if (digitalRead(bt_state) == LOW) {
    if (estado_valor <= 8) {
      estado_valor = 10; // Si llega a 5, volver a 1
    }
    if (estado_valor >= 9) {
      estado_valor = 0; // Si llega a 14, volver a 11
    }
    actualizarPantalla(R, B, G, INT, ssid, password);
  }
  delay(100);

  //------------------------
  
}




String HTML = "<!DOCTYPE html>\
<html>\
<head>\
<title> &#128516; Nuestra Pagina con ESP32  </title>\
<body style= 'background-color: #e0c89a;'> \
</head>\
<body>\
<hr>\
<h2>INSTRUMENTACIO </h2>\
<lu>\
<li>PRACTICA 1</li>\
</lu>\
<hr><h2>WIFI FUNCIONANDO </h2></body></html>";


void handle_root() {
 server.send(200, "text/html", HTML);
}

// ---- display -------------------
void actualizarPantalla(int R, int B, int G, int INT, const char* SSID, const char* pass) {
  LCD.clear(); // Limpiar la pantalla
  /*if (!Conexion_Sensor()) {
        LCD.setCursor(0, 0);
        LCD.print("Sensor no detectado");
        LCD.setCursor(0, 1);
        LCD.print("Verifique la conexión");
        return;  
  }*/
  switch (estado_valor) {
    case 0:
      LCD.setCursor(0, 0);
      LCD.print("Visualizacion");
      LCD.setCursor(0, 1);
      LCD.print("RGB");
      break;




    case 1:
      LCD.setCursor(0, 0);
      LCD.print("Datos Red ");
      LCD.setCursor(0, 1);
      LCD.print(R);
      break;




    case 2:
      LCD.setCursor(0, 0);
      LCD.print("Datos Green");
      LCD.setCursor(0, 1);
      LCD.print(G);
      break;




    case 3:
      LCD.setCursor(0, 0);
      LCD.print("Datos B ");
      LCD.setCursor(0, 1);
      LCD.print(B);
      break;




    case 4:
      LCD.setCursor(0, 0);
      LCD.print("Datos Y");
      LCD.setCursor(0, 1);
      LCD.print(INT);
      break;




    case 10:
      LCD.setCursor(0, 0);
      LCD.print("Datos Servicio");
      break;




    case 11:
      LCD.setCursor(0, 0);
      LCD.print("SS ID ");
      LCD.setCursor(0, 1);
      LCD.print(SSID);
      break;




    case 12:
      LCD.setCursor(0, 0);
      LCD.print("Password ");
      LCD.setCursor(0, 1);
      LCD.print(pass);
      break;




    case 13:
      LCD.setCursor(0, 0);
      LCD.print("IP ");
      LCD.setCursor(0, 1);
      LCD.print(WiFi.localIP());
      break;
  }
  Serial.print("estado_valor: ");
  Serial.println(estado_valor);
}


//----------------------------------
