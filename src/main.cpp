#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_TCS34725.h>
#include <WiFi.h>
//#include <ESPAsyncWebServer.h>

const char *ssid = "Tu_SSID";
const char *password = "Tu_Contraseña";
const char *apSSID = "ESP32-AP";
const char *apPassword = "12345678";

int R = 80;
int B = 56;
int G = 75;
int INT = 12;
void medida_valor(void);

IPAddress apIP(192, 168, 4, 1);
// AsyncWebServer server(80);

int tipo_visualizacion = 0; // 0: visualización datos RGB, 1: visualización servicio 2: ERROR
int contador_suma_resta = 1;
// para que la pagina se actualice sola cada 2 segundos
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 2000;

struct Button
{
    const uint8_t PIN;
    bool pressed;
    unsigned long lastDebounceTime;
};

// Pines de los pulsadores
const int bt_state = 17;
const int bt_plus = 5;
const int bt_less = 18;

Button buttonsum = {bt_plus, 0};
Button buttonresta = {bt_less, 0};
Button buttoncambio = {bt_state, 0};

const unsigned long debounceDelay = 200;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_600MS, TCS34725_GAIN_1X);
LiquidCrystal_I2C lcd(0x3F, 16, 2); // Cambia la dirección I2C si es diferente (0x27 en la mayoría de los casos)

void IRAM_ATTR isr()
{
    // Empty ISR
    // no es necesario no hay respuesta tan rápida de los botones
}
void actualizarPantalla(int tipo_visualizacion, int contador_suma_resta);

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

void setup()
{
    Serial.begin(115200);
    Serial.println("Start");
    // pinMode(9, OUTPUT_OPEN_DRAIN);
    // pinMode(8, OUTPUT_OPEN_DRAIN);

    // Wire.setPins(8, 9);

    // Wire.begin();

    // if (!tcs.begin())
    // {
    //     Serial.println("Error al iniciar TCS34725");
    //     while (1)
    //         delay(1000);
    // }

    // // Inicializa el LCD
    // lcd.init();
    // lcd.backlight();

    // Serial.println("lcd_start");

    // Serial.println("Try Generating ");
    // Serial.println(apSSID);

    // WiFi.softAP(apSSID, apPassword);

    // // Establece la dirección IP estática para el AP
    // WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    // // Inicializa el servidor web
    // // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    // //           { request->send(200, "text/html", HTML); });

    // // Muestra un mensaje inicial en el LCD
    // lcd.setCursor(0, 0);
    // lcd.print("SENSOR  START");
    // lcd.setCursor(0, 1);
    // lcd.print("wifi conecting");

    // Serial.println("");
    // Serial.println("WiFi connected successfully");
    // Serial.print("Got IP: ");
    // IPAddress myIP = WiFi.softAPIP();
    // Serial.println(myIP);

    // server.begin();
    // Serial.println("HTTP server started");
    // delay(100);
    // lcd.setCursor(0, 1);
    // lcd.print("wifi started");

    // Set up button pins
    pinMode(buttonsum.PIN, INPUT_PULLUP);
    pinMode(buttonresta.PIN, INPUT_PULLUP);
    pinMode(buttoncambio.PIN, INPUT_PULLUP);

    // Attach interrupts
    attachInterrupt(buttonsum.PIN, isr, FALLING);
    attachInterrupt(buttonresta.PIN, isr, FALLING);
    attachInterrupt(buttoncambio.PIN, isr, FALLING);
}
unsigned long TiempoAhora;

void medida_valor(void)
{

    uint16_t r, g, b, c, colorTemp, lux;

    if (millis() > TiempoAhora + 1000)
    {
        TiempoAhora = millis();

        tcs.getRawData(&r, &g, &b, &c);
        colorTemp = tcs.calculateColorTemperature(r, g, b);
        lux = tcs.calculateLux(r, g, b);

        Serial.print("Temperatura color: ");
        Serial.print(colorTemp, DEC);
        Serial.println(" K");
        Serial.print("Lux : ");
        Serial.println(lux, DEC);
        Serial.print("Rojo: ");
        Serial.println(r, DEC);
        Serial.print("Verde: ");
        Serial.println(g, DEC);
        Serial.print("Azul: ");
        Serial.println(b, DEC);
        Serial.print("Clear: ");
        Serial.println(c, DEC);
        Serial.println(" ");

        R = r;
        G = g;
        B = b;
        INT = lux;
    }
}

void loop()
{
    unsigned long currentMillis = millis();

    // controla boton sumar 
    if (digitalRead(buttonsum.PIN) == LOW && (currentMillis - buttonsum.lastDebounceTime >= debounceDelay))
    {
        buttonsum.lastDebounceTime = currentMillis;
        buttonsum.pressed = true;
        Serial.println("Boton suma presionado :");
        contador_suma_resta++;
        if (contador_suma_resta == 6 && tipo_visualizacion == 0)
        {
            contador_suma_resta = 1;
        }
        if (contador_suma_resta == 4 && tipo_visualizacion == 1)
        {
            contador_suma_resta = 1;
        }
        actualizarPantallaEstatica(tipo_visualizacion, contador_suma_resta);
        actualizarPantalla(tipo_visualizacion, contador_suma_resta);
    }
    else
        buttonsum.pressed = false;

    // controla boton resta 
    if (digitalRead(buttonresta.PIN) == LOW && (currentMillis - buttonresta.lastDebounceTime >= debounceDelay))
    {
        buttonresta.lastDebounceTime = currentMillis;
        buttonresta.pressed = true;
        Serial.println("Boton resta presionado :");
        contador_suma_resta = contador_suma_resta - 1;
        if (contador_suma_resta == 0 && tipo_visualizacion == 0)
        {
            contador_suma_resta = 5;
        }
        if (contador_suma_resta == 0 && tipo_visualizacion == 1)
        {
            contador_suma_resta = 3;
        }
        actualizarPantallaEstatica(tipo_visualizacion, contador_suma_resta);
        actualizarPantalla(tipo_visualizacion, contador_suma_resta);
    }
    else
        buttonresta.pressed = false;

    // controla boton cambio
    if (digitalRead(buttoncambio.PIN) == LOW && (currentMillis - buttoncambio.lastDebounceTime >= debounceDelay))
    {
        buttoncambio.lastDebounceTime = currentMillis;
        buttoncambio.pressed = true;
        Serial.println("Boton cambio presionado :");
        if(tipo_visualizacion==0){
            tipo_visualizacion=1;
            contador_suma_resta=1;
        }
        else{
            tipo_visualizacion=0;
            contador_suma_resta=1;
        }
        actualizarPantallaEstatica(tipo_visualizacion, contador_suma_resta);
        actualizarPantalla(tipo_visualizacion, contador_suma_resta);
    }
    else
        buttoncambio.pressed = false;

    delay(100);

    // actualiza cada 2 segundos si te molesta para hacer pruebas comentarla
    if (currentMillis - lastUpdateTime >= updateInterval)
    {
        actualizarPantalla(tipo_visualizacion, contador_suma_resta);
        lastUpdateTime = currentMillis; // Reset del tiempo
    }
}

void mostrar_texto(const char* parte_sup, const char* parte_inf){ //muestra por pantalla valores
  LCD.setCursor(0, 0);
  LCD.print(parte_sup);
  LCD.setCursor(0, 1);
  LCD.print(parte_inf);
}

void mostar_datosestatico(const char* dato/*, int valor*/) { // muestra por pantalla textos
  LCD.setCursor(0, 0);
  LCD.print(dato);
  //LCD.setCursor(0, 1);
  //LCD.print(valor);
}

void mostar_datos(/*const char* dato,*/ int valor) { // muestra por pantalla textos
  //LCD.setCursor(0, 0);
  //LCD.print(dato);
  LCD.setCursor(0, 1);
  LCD.print(valor);
}

void actualizarPantallaEstatica(int tipo_visualizacion, int contador_suma_resta) {
  LCD.clear();
  // Limpiar la pantalla, este es el mensaje estático de la pantalla
  /*if (!Conexion_Sensor()) {
        LCD.setCursor(0, 0);
        LCD.print("Sensor no detectado");
        LCD.setCursor(0, 1);
        LCD.print("Verifique la conexión");
        return;
  }*/
  if(tipo_visualizacion == 0){
    switch (contador_suma_resta) { //visualizacion por pantalla de datos y sus valores
      case 1:
          LCD.setCursor(0, 0);
          LCD.print("Vista R  G  B ");
          break;

      case 2:
        mostar_datosestatico("  Valor de red");
        break;

      case 3:
        mostar_datosestatico(" Valor de green");
        break;

      case 4:
        mostar_datosestatico(" Valor de blue");
        break;

      case 5:
        mostar_datosestatico(" Valor de luma");
        break;

      case 6:
        mostar_datosestatico(" Valor de croma");
        break;
    }
  }
  else{
    switch(contador_suma_resta){
      case 1:
      mostrar_texto("     Datos", "    Servicio");
      break;
      case 2:
      mostar_datosestatico("      SSID");
      mostar_datos(SSID);
      break;

      case 3:
      mostrar_texto("    Password", Pass);
      break;

      case 4:
      mostar_datosestatico("       IP");
      mostar_datos(IP);
      break;
    }
  } 
}

void actualizarPantalla(int tipo_visualizacion, int contador_suma_resta) {
  // Limpiar la pantalla
  /*if (!Conexion_Sensor()) {
        LCD.setCursor(0, 0);
        LCD.print("Sensor no detectado");
        LCD.setCursor(0, 1);
        LCD.print("Verifique la conexión");
        return;
  }*/

  if(tipo_visualizacion == 0){
    switch (contador_suma_resta) { //visualizacion por pantalla de datos y sus valores
    case 0:
          LCD.setCursor(0, 1);
          LCD.print("   ");
          LCD.print(R);
          LCD.print(" ");
          LCD.print(G);
          LCD.print(" ");
          LCD.print(B);
          break;

      case 1:
        mostar_datos(R);
        break;

      case 2:
        mostar_datos(G);
        break;

      case 3:
      
        mostar_datos(B);
        break;

      case 4:
      
        mostar_datos(Y);
        break;
        
      case 5:
        mostar_datos(C);
        break;
    }
    else{
      case 1:
        mostar_datos(SSID);
        break;

      case 2:
        LCD.setCursor(0, 1);
        LCD.print(Pass);
        break;

      case 3:
  
        mostar_datos(IP);
        break;
    }
  }
}


