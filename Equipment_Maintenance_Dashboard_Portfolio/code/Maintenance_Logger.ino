#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal.h>

// ---- Pins ----
const int PIN_THERMISTOR = A0;   // voltage divider mid-node
const int PIN_TILT       = 6;    // tilt switch to GND, using INPUT_PULLUP
const int PIN_DHT        = 2;    // DHT11 data
const int PIN_BTN        = 4;    // push-button to GND, using INPUT_PULLUP

// ---- Peripherals ----
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);
LiquidCrystal lcd(9, 10, 11, 12, 7, 8); // rs,en,d4,d5,d6,d7

// ---- Thermistor constants ----
const float SERIES_RESISTOR = 10000.0;  // 10k
const float THERM_NOMINAL   = 10000.0;  // R@25C
const float TEMP_NOMINAL    = 25.0;     // 25C
const float B_COEFFICIENT   = 3950.0;
const int   ADC_MAX         = 1023;

// ---- Tilt debounce / vibration window ----
bool tiltLast = HIGH;
unsigned long lastTiltChangeMs = 0;
const unsigned long DEBOUNCE_MS = 30;
unsigned int vibrationEventsWindow = 0;

// ---- Button (toggle equipment state) ----
bool equipOn = false;
bool btnLast = HIGH;
unsigned long lastBtnChangeMs = 0;

// ---- Runtime accumulation ----
unsigned long equipOnStartMs = 0;
unsigned long accumulatedRunMs = 0;

// ---- Reporting ----
const unsigned long REPORT_MS = 5000;
unsigned long lastReport = 0;

// ---- LCD refresh ----
const unsigned long LCD_MS = 500;
unsigned long lastLCD = 0;

float readThermistorC() {
  int adc = analogRead(PIN_THERMISTOR);
  if (adc <= 0) adc = 1;
  if (adc >= 1023) adc = 1022;

  float v = (float)adc / ADC_MAX;
  float resistance = SERIES_RESISTOR * (1.0 / v - 1.0);

  float steinhart = resistance / THERM_NOMINAL;
  steinhart = log(steinhart);
  steinhart /= B_COEFFICIENT;
  steinhart += 1.0 / (TEMP_NOMINAL + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  return steinhart;
}

void printLCDLine(uint8_t row, const String &text) {
  lcd.setCursor(0, row);
  lcd.print("                "); // clear row (16 spaces)
  lcd.setCursor(0, row);
  lcd.print(text);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_TILT, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  tiltLast = digitalRead(PIN_TILT);
  btnLast  = digitalRead(PIN_BTN);

  dht.begin();

  lcd.begin(16, 2);
  printLCDLine(0, "Maint Dashboard");
  printLCDLine(1, "Starting...");
  delay(800);
}

void loop() {
  unsigned long now = millis();

  // ---- Button toggle (debounced) ----
  bool btnNow = digitalRead(PIN_BTN);
  if (btnNow != btnLast && (now - lastBtnChangeMs) > 50) {
    lastBtnChangeMs = now;
    btnLast = btnNow;
    if (btnNow == LOW) { // pressed
      if (!equipOn) {
        equipOn = true;
        equipOnStartMs = now;
      } else {
        equipOn = false;
        accumulatedRunMs += now - equipOnStartMs;
      }
    }
  }

  // ---- Tilt/vibration (debounced) ----
  bool tiltNow = digitalRead(PIN_TILT);
  if (tiltNow != tiltLast && (now - lastTiltChangeMs) > DEBOUNCE_MS) {
    lastTiltChangeMs = now;
    tiltLast = tiltNow;
    // Count vibration only while equipment is ON
    if (tiltNow == LOW && equipOn) vibrationEventsWindow++;
  }

  // ---- Read sensors ----
  float equipTempC = readThermistorC();
  float ambientC = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (isnan(ambientC)) ambientC = -127.0;
  if (isnan(humidity)) humidity = -1.0;

  // ---- LCD update ----
  if (now - lastLCD > LCD_MS) {
    lastLCD = now;

    float runtimeMin = (accumulatedRunMs + (equipOn ? (now - equipOnStartMs) : 0)) / 60000.0;

    // Line 1: T=xx.xC V=##
    String l1 = "T=";
    l1 += String(equipTempC, 1);
    l1 += "C V=";
    l1 += String(vibrationEventsWindow);
    printLCDLine(0, l1);

    // Line 2: E:ON/OFF Rt=xx.xm
    String l2 = (equipOn ? "E:ON " : "E:OFF");
    l2 += " Rt=";
    l2 += String(runtimeMin, 1);
    l2 += "m";
    printLCDLine(1, l2);
  }

  // ---- Serial JSON report (every 5 s) ----
  if (now - lastReport >= REPORT_MS) {
    lastReport = now;

    float runtimeMinutes = (accumulatedRunMs + (equipOn ? (now - equipOnStartMs) : 0)) / 60000.0;

    Serial.print("{");
    Serial.print("\"asset_id\":\"A1000\",");
    Serial.print("\"timestamp_ms\":"); Serial.print(now); Serial.print(",");
    Serial.print("\"temp_c\":"); Serial.print(equipTempC, 2); Serial.print(",");
    Serial.print("\"ambient_c\":"); Serial.print(ambientC, 2); Serial.print(",");
    Serial.print("\"humidity\":"); Serial.print(humidity, 1); Serial.print(",");
    Serial.print("\"vibration_events\":"); Serial.print(vibrationEventsWindow); Serial.print(",");
    Serial.print("\"equipment_on\":"); Serial.print(equipOn ? "true" : "false"); Serial.print(",");
    Serial.print("\"runtime_minutes\":"); Serial.print(runtimeMinutes, 2);
    Serial.println("}");

    vibrationEventsWindow = 0; // reset window counter
  }
}
