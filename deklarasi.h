#define BLYNK_TEMPLATE_ID "TMPL8dcPRArG"
#define BLYNK_TEMPLATE_NAME "raihan"
#define BLYNK_AUTH_TOKEN "MxXknJCncXg5gXPeCW5tsnC2c87f1u0S"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <HTTPClient.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>

const char* ssid = "Zultv";
const char* password = "Zultva3.";
// const char* ssid = "GiJoe";
// const char* password = "12345678";

const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client;

// Replace with your Google Apps Script ID
String GAS_ID = "AKfycbwZaAweWyaQeMH37lOnMCx8umhNvEPnNGPeyhlsRtKtz2uAuyPyWAk-Z4y7qcLOhn9l9w";

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 17
#define PZEM_TX_PIN 16
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif

#if defined(ESP32)
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)
#else
PZEM004Tv30 pzem(PZEM_SERIAL);
#endif

LiquidCrystal_I2C lcd(0x27,20,4);
#define solenoid 18
#define waterPump 19
#define inverter 5

int tegangan = 39; // pin signal dari sensor masuk ke pin A0 arduino
float Vsensor = 0.0; //nilai masukan sensor
float hasil = 0.0; //nilai hasil rumus
float R1 = 30000.0; //30k ohm resistor (sesuai dengan nilai resistor di sensor)
float R2 = 7500.0; //7.5k ohm resistor (sesuai dengan nilai resistor di sensor)
float arus = 0.0; //nilai arus
float faktorKalibrasi = 2.40 / 0.000792; // faktor kalibrasi berdasarkan pengukuran

int jadwalHarian, jadwalGabungan;