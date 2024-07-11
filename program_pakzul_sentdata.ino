#include "deklarasi.h"

unsigned long lastLcdUpdate = 0;
unsigned long lastSendTime = 0;
unsigned long SolenoidTime = 0;
unsigned long WaterPumpTime = 0;
int lcdState = 0;
const unsigned long lcdInterval = 15000;
const unsigned long sendInterval = 150000;
const unsigned long solenoidInterval = 30000;
const unsigned long waterPumpInterval = 30000;

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
    pinMode(solenoid, OUTPUT);
    pinMode(waterPump, OUTPUT);
    pinMode(inverter, OUTPUT);
    lcd.setCursor(7,1);
    lcd.print("Welcome");
    lcd.setCursor(6,2);
    lcd.print("Zulkhairi");

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }

    client.setInsecure();
    // Uncomment in order to reset the internal energy counter
    // pzem.resetEnergy()
}

void loop() {
    unsigned long currentMillis = millis();
    Blynk.run();

    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    int nilaiTegangan = analogRead(tegangan);
    Vsensor = (nilaiTegangan * 5.0) / 5900.0; //rumus mengubah nilai baca sensor
    hasil = Vsensor / (R2 / (R1 + R2)); //hasil akhir
    arus = (Vsensor / R2) * faktorKalibrasi; // menghitung arus dengan faktor kalibrasi

    if (jadwalOn == 1){
      digitalWrite(inverter, LOW);
    }
    if(jadwalOFF == 1){
      digitalWrite(inverter, HIGH);
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V2, 0);
      jadwalOn = 0;
      jadwalOFF = 0;
    }

    if (currentMillis - lastLcdUpdate >= lcdInterval) {
        lastLcdUpdate = currentMillis;
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

    if (currentMillis - lastSendTime >= sendInterval) {
        lastSendTime = currentMillis;
        sendData(voltage, current, power, energy, frequency, pf, hasil, arus);
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
