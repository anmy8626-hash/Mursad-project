#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>
#include <HardwareSerial.h>
#include <math.h>

// =========================================================
// 1. تعريف المنافذ (GPIOs) - راجعيها مع المخطط الفعلي
// =========================================================
static const int PIN_STATUS_LED    = 21;   // GPIO_LED_STATUS في المخطط
static const int PIN_BUTTON        = 0;    // زر SW2 / BOOT في المخطط
static const int PIN_PM_RX         = 16;   // PM_RX في المخطط
static const int PIN_PM_TX         = 17;   // PM_TX في المخطط

static const int PIN_I2C_SDA       = 8;    // عدّلي حسب توصيل SDA لـ BMP280 و ADS1115
static const int PIN_I2C_SCL       = 9;    // عدّلي حسب توصيل SCL لـ BMP280 و ADS1115
static const int PIN_ONEWIRE_TEMP  = 4;    // عدّلي حسب توصيل DQ لحساس DS18B20

static const int PIN_ISOLATE_CTRL  = 6;    // خط العزل الهاردويري (Optocoupler / Relay)
static const int PIN_LAMP_CTRL     = 7;    // تحكم حمل اللمبة
static const int PIN_MOTOR_CTRL    = 15;   // تحكم حمل المحرك

// =========================================================
// 2. ثوابت التحقق الفيزيائي والعزل
// =========================================================
static const float MAX_DEVIATION_PCT      = 5.0f;   // عتبة الانحراف ±5%
static const float MAX_TEMP_C             = 65.0f;  // حد الحرارة الأقصى
static const float EXPECTED_Z_LAMP_OHM    = 240.0f; // المعاوقة المتوقعة لحمل اللمبة
static const float EXPECTED_Z_MOTOR_OHM   = 185.0f; // المعاوقة المتوقعة لحمل المحرك
static const unsigned long TELEMETRY_MS   = 250;    // إرسال الحالة كل 250 ms

// =========================================================
// 3. الكائنات وحالة النظام
// =========================================================
OneWire oneWire(PIN_ONEWIRE_TEMP);
DallasTemperature ds18b20(&oneWire);
Adafruit_BMP280 bmp;
Adafruit_ADS1115 ads;
HardwareSerial PowerMeterSerial(1);

bool bmpReady = false;
bool adsReady = false;

bool isIsolated = false;
bool lampState  = false;
bool motorState = false;
String statusReason = "NOMINAL";

float voltageV      = 220.0f;
float currentA      = 0.0f;
float activePowerW  = 0.0f;
float powerFactor   = 0.98f;
float tempC         = 28.0f;
float pressureBar   = 1.01f;

float zActual       = 0.0f;
float zExpected     = 0.0f;
float deviationPct  = 0.0f;

unsigned long lastTelemetryTime = 0;
String serialBuffer = "";

// =========================================================
// 4. دوال العزل والتحكم الفيزيائي
// =========================================================
void triggerIsolation(const String& reason) {
  isIsolated = true;
  statusReason = reason;

  // عزل فوري وفصل الأحمال
  digitalWrite(PIN_ISOLATE_CTRL, HIGH);
  digitalWrite(PIN_LAMP_CTRL, LOW);
  digitalWrite(PIN_MOTOR_CTRL, LOW);
  lampState = false;
  motorState = false;

  digitalWrite(PIN_STATUS_LED, HIGH);
}

void resetIsolation() {
  isIsolated = false;
  statusReason = "NOMINAL";
  deviationPct = 0.0f;

  digitalWrite(PIN_ISOLATE_CTRL, LOW);
  digitalWrite(PIN_STATUS_LED, LOW);
}

void setLamp(bool enable) {
  if (isIsolated && enable) {
    statusReason = "BLOCKED_BY_ISOLATION";
    return;
  }
  lampState = enable;
  digitalWrite(PIN_LAMP_CTRL, lampState ? HIGH : LOW);
}

void setMotor(bool enable) {
  if (isIsolated && enable) {
    statusReason = "BLOCKED_BY_ISOLATION";
    return;
  }
  motorState = enable;
  digitalWrite(PIN_MOTOR_CTRL, motorState ? HIGH : LOW);
}

// =========================================================
// 5. قراءة الحساسات والتحقق عبر التوأم الرقمي
// =========================================================
void readTelemetry() {
  // حساس الحرارة DS18B20
  ds18b20.requestTemperatures();
  float tDs = ds18b20.getTempCByIndex(0);
  if (tDs != DEVICE_DISCONNECTED_C && tDs > -50.0f && tDs < 150.0f) {
    tempC = tDs;
  }

  // حساس الضغط BMP280
  if (bmpReady) {
    float pPa = bmp.readPressure();
    if (!isnan(pPa) && pPa > 100.0f) {
      pressureBar = pPa / 100000.0f;
    }
  }

  // قراءة تناظرية عبر ADS1115 (القناة AIN0)
  if (adsReady) {
    int16_t raw0 = ads.readADC_SingleEnded(0);
    float vAdc = ads.computeVolts(raw0);
    // عدّلي معامل القياس حسب مجس التيار/الإشارة المتصل بـ AIN0
    float iMeas = fabs(vAdc) * 2.0f;
    if (iMeas >= 0.0f && iMeas < 100.0f) {
      currentA = iMeas;
    }
  } else {
    // تقدير مبدئي عند عدم توفر حساس تيار خارجي
    float estI = 0.0f;
    if (lampState)  estI += 0.90f;
    if (motorState) estI += 1.15f;
    currentA = estI;
  }

  activePowerW = voltageV * currentA * powerFactor;
}

float calcExpectedImpedance() {
  if (!lampState && !motorState) return 0.0f;
  if (motorState && !lampState) return EXPECTED_Z_MOTOR_OHM;
  if (lampState && !motorState)  return EXPECTED_Z_LAMP_OHM;

  float invSum = (1.0f / EXPECTED_Z_LAMP_OHM) + (1.0f / EXPECTED_Z_MOTOR_OHM);
  return (invSum > 0.0f) ? (1.0f / invSum) : 0.0f;
}

void runDigitalTwinCheck() {
  zActual   = (currentA > 0.02f) ? (voltageV / currentA) : 0.0f;
  zExpected = calcExpectedImpedance();

  if (zActual > 0.0f && zExpected > 0.0f) {
    deviationPct = (fabs(zActual - zExpected) / zExpected) * 100.0f;
  } else {
    deviationPct = 0.0f;
  }

  if (!isIsolated) {
    if (tempC > MAX_TEMP_C) {
      triggerIsolation("OVER_TEMPERATURE");
    } else if (deviationPct > MAX_DEVIATION_PCT) {
      triggerIsolation("PHYSICAL_DEVIATION_GT_5PCT");
    } else {
      statusReason = "NOMINAL";
    }
  }
}

// =========================================================
// 6. الاتصال التسلسلي (JSON over USB Serial)
// =========================================================
void sendJsonState() {
  String json = "{";
  json += "\"v\":" + String(voltageV, 2) + ",";
  json += "\"i\":" + String(currentA, 2) + ",";
  json += "\"p\":" + String(activePowerW, 2) + ",";
  json += "\"pf\":" + String(powerFactor, 2) + ",";
  json += "\"temp\":" + String(tempC, 2) + ",";
  json += "\"press\":" + String(pressureBar, 3) + ",";
  json += "\"z_actual\":" + String(zActual, 2) + ",";
  json += "\"z_expected\":" + String(zExpected, 2) + ",";
  json += "\"dev_pct\":" + String(deviationPct, 2) + ",";
  json += "\"isolated\":" + String(isIsolated ? "true" : "false") + ",";
  json += "\"lamp\":" + String(lampState ? "true" : "false") + ",";
  json += "\"motor\":" + String(motorState ? "true" : "false") + ",";
  json += "\"reason\":\"" + statusReason + "\"";
  json += "}";

  Serial.println(json);
}

void processCommand(const String& rawLine) {
  String cmd = rawLine;
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd.indexOf("TRIGGER_ISOLATION") != -1) {
    triggerIsolation("COMMAND_ISOLATION");
  } else if (cmd.indexOf("RESET_ISOLATION") != -1) {
    resetIsolation();
  } else if (cmd.indexOf("SET_LAMP") != -1) {
    bool val = (cmd.indexOf("true") != -1 || cmd.indexOf(":1") != -1 || cmd.indexOf(" 1") != -1);
    setLamp(val);
  } else if (cmd.indexOf("SET_MOTOR") != -1) {
    bool val = (cmd.indexOf("true") != -1 || cmd.indexOf(":1") != -1 || cmd.indexOf(" 1") != -1);
    setMotor(val);
  }

  sendJsonState();
}

void pollSerial() {
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    if (ch == '\n') {
      processCommand(serialBuffer);
      serialBuffer = "";
    } else if (ch != '\r') {
      serialBuffer += ch;
    }
  }
}

// =========================================================
// 7. Setup & Loop
// =========================================================
void setup() {
  Serial.begin(115200);
  PowerMeterSerial.begin(9600, SERIAL_8N1, PIN_PM_RX, PIN_PM_TX);

  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_ISOLATE_CTRL, OUTPUT);
  pinMode(PIN_LAMP_CTRL, OUTPUT);
  pinMode(PIN_MOTOR_CTRL, OUTPUT);

  digitalWrite(PIN_STATUS_LED, LOW);
  digitalWrite(PIN_ISOLATE_CTRL, LOW);
  digitalWrite(PIN_LAMP_CTRL, LOW);
  digitalWrite(PIN_MOTOR_CTRL, LOW);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  ds18b20.begin();

  bmpReady = bmp.begin(0x76) || bmp.begin(0x77);
  adsReady = ads.begin();
  if (adsReady) {
    ads.setGain(GAIN_ONE);
  }
}

void loop() {
  pollSerial();

  if (digitalRead(PIN_BUTTON) == LOW && isIsolated) {
    delay(50);
    if (digitalRead(PIN_BUTTON) == LOW) {
      resetIsolation();
    }
  }

  unsigned long now = millis();
  if (now - lastTelemetryTime >= TELEMETRY_MS) {
    lastTelemetryTime = now;
    readTelemetry();
    runDigitalTwinCheck();
    sendJsonState();
  }
}

