#define BLYNK_TEMPLATE_ID "TMPL2_MmrwqRi"
#define BLYNK_TEMPLATE_NAME "Smart Power Monitoring Systems and Control"
#define BLYNK_AUTH_TOKEN "ARa6s6GBSUoULrdGbGrfixDs5RQoC3Xy"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <PZEM004Tv30.h>
#include <HardwareSerial.h>

// ---------- USER CONFIG ----------
char ssid[] = "Projects";
char pass[] = "Projects";

#define PZEM_RX_PIN 16   // ESP32 RX2 <- PZEM TX
#define PZEM_TX_PIN 17   // ESP32 TX2 -> PZEM RX

#define RELAY_LIGHT_PIN  26
#define RELAY_SOCKET_PIN 27

#define RELAY_ACTIVE_LOW true   // most relay modules trigger LOW to close
// ----------------------------------

HardwareSerial pzemSerial(2); // UART2
PZEM004Tv30 pzem(pzemSerial, PZEM_RX_PIN, PZEM_TX_PIN);

BlynkTimer timer;

bool lightState  = false;
bool socketState = false;

void applyRelay(int pin, bool state) {
  bool level = RELAY_ACTIVE_LOW ? !state : state;
  digitalWrite(pin, level);
}

// Blynk writes relay state when user toggles switch in app
BLYNK_WRITE(V6) {
  lightState = param.asInt();
  applyRelay(RELAY_LIGHT_PIN, lightState);
}

BLYNK_WRITE(V7) {
  socketState = param.asInt();
  applyRelay(RELAY_SOCKET_PIN, socketState);
}

// Sync relay states with app on reconnect
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V6, V7);
}

void sendPowerData() {
  float voltage   = pzem.voltage();
  float current   = pzem.current();
  float power     = pzem.power();
  float energy    = pzem.energy();
  float frequency = pzem.frequency();
  float pf        = pzem.pf();

  if (isnan(voltage)) {
    Serial.println("Error reading PZEM data");
    return;
  }

  Serial.printf("V:%.1fV  I:%.3fA  P:%.1fW  E:%.3fkWh  F:%.1fHz  PF:%.2f\n",
                voltage, current, power, energy, frequency, pf);

  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, energy);
  Blynk.virtualWrite(V4, frequency);
  Blynk.virtualWrite(V5, pf);
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_LIGHT_PIN, OUTPUT);
  pinMode(RELAY_SOCKET_PIN, OUTPUT);
  applyRelay(RELAY_LIGHT_PIN, false);
  applyRelay(RELAY_SOCKET_PIN, false);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read and push power data every 2 seconds
  timer.setInterval(2000L, sendPowerData);
}

void loop() {
  Blynk.run();
  timer.run();
}
