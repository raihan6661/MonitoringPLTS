#include "deklarasi.h"

unsigned long lastLcdUpdate = 0;
unsigned long lastSendTime = 0;
unsigned long SolenoidTime = 0;
unsigned long PzemTime = 0;
int lcdState = 0;
const unsigned long lcdInterval = 2000;
const unsigned long sendInterval = 150000;
const unsigned long solenoidInterval = 30000;
const long pzemInterval = 180000;
unsigned long currentMillis;
unsigned long currentTime;
unsigned long lastTime;
unsigned long pulse_freq;

BLYNK_WRITE(V1)
{
  jadwalOn = param.asInt(); // assigning incoming value from pin V1 to a variable
}
BLYNK_WRITE(V2)
{
  jadwalOFF = param.asInt(); // assigning incoming value from pin V1 to a variable
}

void setup() {
    // Debugging Serial port
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    WiFi.begin(ssid, password);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
    pinMode(tegangan, INPUT); 
    pinMode(relaySolenoid, OUTPUT);
    pinMode(relayInverter, OUTPUT);
    pinMode(relayWaktu, OUTPUT);
    pinMode(relayPzem, OUTPUT);
    lcd.setCursor(7,1);
    lcd.print("Welcome");
    lcd.setCursor(6,2);
    lcd.print("Zulkhairi");
    Blynk.virtualWrite(V1, 1);
    jadwalOn = 1;
   lastTime = currentTime;

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }

    client.setInsecure();
    // Uncomment in order to reset the internal energy counter
    // pzem.resetEnergy()
}

void loop() {
    currentMillis = millis();
    Blynk.run();

    // Read the data from the sensor
    voltage = pzem.voltage();
    current = pzem.current();
    power = pzem.power();
    energy = pzem.energy();
    frequency = pzem.frequency();
    pf = pzem.pf();

    int nilaiTegangan = analogRead(tegangan);
    Vsensor = (nilaiTegangan * 5.0) / 5900.0; //rumus mengubah nilai baca sensor
    hasil = Vsensor / (R2 / (R1 + R2)); //hasil akhir
    arus = (Vsensor / R2) * faktorKalibrasi; // menghitung arus dengan faktor kalibrasi
    Blynk.virtualWrite(V0, hasil);
    Blynk.virtualWrite(V3, arus);
    Blynk.virtualWrite(V4, voltage);
    Blynk.virtualWrite(V5, current);
    Blynk.virtualWrite(V6, power);
    Blynk.virtualWrite(V7, energy);
    Blynk.virtualWrite(V8, pf);


    setInverter();
    waterFlowS();
    
    if (currentMillis - lastLcdUpdate >= lcdInterval) {
        lastLcdUpdate = currentMillis;
        lcdOn();
    }

    if (currentMillis - lastSendTime >= sendInterval) {
        lastSendTime = currentMillis;
        sendData(voltage, current, power, energy, frequency, pf, hasil, arus);
    }
}

void pulse (){
   pulse_freq++;
}

void waterFlowS(){
  currentTime = millis();
  if(currentTime >= (lastTime + 1000))
   {
      lastTime = currentTime; 
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      flow = (pulse_freq / 7.5); 
      pulse_freq = 0; // Reset Counter
      Serial.print(flow, 3); 
      Serial.println(" L/Min");
   }
}

void lcdOn(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Daya Out");
  lcd.setCursor(0,1);
  lcd.print("V = ");
  lcd.setCursor(4,1);
  lcd.print(voltage);
  lcd.setCursor(0,2);
  lcd.print("A =");
  lcd.setCursor(4,2);
  lcd.print(current);
  lcd.setCursor(11,0);
  lcd.print("Daya In");
  lcd.setCursor(11,1);
  lcd.print("V =");
  lcd.setCursor(14,1);
  lcd.print(hasil,2);
  lcd.setCursor(11,2);
  lcd.print("A =");
  lcd.setCursor(14,2);
  lcd.print(arus,2);
}

void setInverter(){
  if(jadwalOn == 1){
      digitalWrite(relayWaktu, HIGH);
      if (hasil <= 11.20){
        digitalWrite(relayInverter, LOW);
        digitalWrite(relayPzem, LOW);
      }else if(hasil >= 12.30){
        digitalWrite(relayInverter, HIGH);
        if (currentMillis - PzemTime >= pzemInterval) {
          PzemTime = currentMillis;
          digitalWrite(relayPzem, HIGH);
         }
      }
    }
    if (jadwalOFF == 1){
      digitalWrite(relayWaktu, LOW);
      digitalWrite(relayPzem, LOW);
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V2, 0);
      jadwalOn = 0;
      jadwalOFF = 0;
    }
}

void sendData(float voltage, float current, float power, float energy, float frequency, float pf, float teganganIn, float arusIn) {
    Serial.println("==========");
    Serial.print("Connecting to: ");
    Serial.println(host);

    if (!client.connect(host, httpsPort)) {
        Serial.println("Connection failed");
        return;
    } 

    String url = "/macros/s/" + GAS_ID + "/exec?tegangan=" + voltage + "&arus=" + current + 
                "&daya=" + power + "&energi=" + energy + "&frekuensi=" + frequency + "&powerfaktor=" + pf +"&TeganganIn=" + teganganIn + "&ArusIn=" + arusIn;
    Serial.print("Requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Request sent");

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("Headers received");
            break;
        }
    }

    // Optionally read and print the response
    while (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
    }
    client.stop();
}
