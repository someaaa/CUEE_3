/******************** BLYNK CONFIG ********************/
#define BLYNK_TEMPLATE_ID "TMPL6TEgwENGm"
#define BLYNK_TEMPLATE_NAME "Smart Workspace"
#define BLYNK_AUTH_TOKEN "HV4QRqU0J1fyCdLAEwOCiwAECP3dBmfB"
#define BLYNK_PRINT Serial

/******************** LIBRARY ********************/
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

/******************** WIFI ********************/
char ssid[] = "Redmi";
char pass[] = "asdfghjkl";

/******************** PIN CONFIG ********************/
#define TRIG_EYE 5
#define ECHO_EYE 18

#define TRIG_BACK_UP 32
#define ECHO_BACK_UP 33

#define TRIG_BACK_LOW 25
#define ECHO_BACK_LOW 26

#define PIN_LIGHT 34

#define BUZZER 13
#define LED_SYS 2
#define LED_ALERT_2 4
#define LED_ALERT_3 15

#define BTN 27

/******************** LCD ********************/
LiquidCrystal_PCF8574 lcd(0x27);

/******************** SYSTEM STATE ********************/
bool systemActive = false;

// Work
bool isSitting = false;
unsigned long sessionStart = 0;
unsigned long totalWorkAccumulated = 0;

// Eye
unsigned long closeStart = 0;
unsigned long closeTotal = 0;
bool isClose = false;

// Hunch
unsigned long hunchStart = 0;
unsigned long hunchCount = 0;
bool isHunch = false;

// Blynk
unsigned long lastSend = 0;

/******************** ALERT FLAGS ********************/
bool alertEye = false;
bool alertHunch = false;
bool alertLight = false;
bool alertWork = false;

/******************** THRESHOLD ********************/
#define CLOSE_THRESHOLD 40
#define CLOSE_DELAY 3000

#define HUNCH_DIFF 15

#define LIGHT_THRESHOLD 500

#define WORK_LIMIT 3600000

/******************** LCD SYSTEM ********************/
String lastL1 = "", lastL2 = "";
unsigned long lastLCDSwitch = 0;
int lcdIndex = 0;
#define LCD_INTERVAL 2000

/******************** BUZZER ********************/
enum BeepType { NONE, EYE, HUNCH, WORK };
BeepType currentBeep = NONE;
unsigned long beepStart = 0;

void handleBuzzer(BeepType type) {
  unsigned long now = millis();
  switch (type) {
    case EYE:
      if (now - beepStart < 300) digitalWrite(BUZZER, HIGH);
      else if (now - beepStart < 600) digitalWrite(BUZZER, LOW);
      else beepStart = now;
      break;
    case HUNCH:
      if (now - beepStart < 200) digitalWrite(BUZZER, HIGH);
      else if (now - beepStart < 400) digitalWrite(BUZZER, LOW);
      else if (now - beepStart < 600) digitalWrite(BUZZER, HIGH);
      else if (now - beepStart < 800) digitalWrite(BUZZER, LOW);
      else beepStart = now;
      break;
    case WORK:
      if (now - beepStart < 800) digitalWrite(BUZZER, HIGH);
      else if (now - beepStart < 1600) digitalWrite(BUZZER, LOW);
      else beepStart = now;
      break;
    default:
      digitalWrite(BUZZER, LOW);
      break;
  }
}

/******************** ULTRASONIC ********************/
float getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 25000);
  if (duration == 0) return 400;
  return duration * 0.034 / 2;
}

/******************** LCD ********************/
void updateLCD(String l1, String l2) {
  if (l1 != lastL1 || l2 != lastL2) {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(l1);

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(l2);

    lastL1 = l1;
    lastL2 = l2;
  }
}

/******************** SERIAL DEBUG ********************/
void printDebug(float eyeDist, float backUp, float backLow, float lux) {
  Serial.println("====== SYSTEM STATUS ======");
  Serial.print("System: ");
  Serial.println(systemActive ? "ON" : "OFF");

  Serial.print("Eye Distance: "); Serial.print(eyeDist); Serial.println(" cm");
  Serial.print("BackUp: "); Serial.print(backUp);
  Serial.print(" | BackLow: "); Serial.println(backLow);

  Serial.print("Light: "); Serial.println(lux);

  Serial.print("Sitting: "); Serial.println(isSitting ? "YES" : "NO");
  Serial.print("Close: "); Serial.println(isClose ? "YES" : "NO");
  Serial.print("Hunch Count: "); Serial.println(hunchCount);

  Serial.print("Alert: ");
  if (alertEye) Serial.print("[EYE] ");
  if (alertHunch) Serial.print("[HUNCH] ");
  if (alertLight) Serial.print("[LIGHT] ");
  if (alertWork) Serial.print("[WORK] ");
  if (!alertEye && !alertHunch && !alertLight && !alertWork) Serial.print("NONE");

  Serial.println("\n===========================\n");
  delay(1000);
}

/******************** SETUP ********************/
void setup() {
  Serial.begin(9600);
  Serial.println("System Booting...");

  lcd.begin(16, 2);
  lcd.setBacklight(255);

  pinMode(TRIG_EYE, OUTPUT); digitalWrite(TRIG_EYE, LOW);
  pinMode(ECHO_EYE, INPUT);

  pinMode(TRIG_BACK_UP, OUTPUT); digitalWrite(TRIG_BACK_UP, LOW);
  pinMode(ECHO_BACK_UP, INPUT);

  pinMode(TRIG_BACK_LOW, OUTPUT); digitalWrite(TRIG_BACK_LOW, LOW);
  pinMode(ECHO_BACK_LOW, INPUT);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_SYS, OUTPUT);
  pinMode(LED_ALERT_2, OUTPUT);
  pinMode(LED_ALERT_3, OUTPUT);

  pinMode(BTN, INPUT_PULLUP);

  WiFi.begin(ssid, pass);
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) {
    delay(500);
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  if (WiFi.status() == WL_CONNECTED) Blynk.connect(3000);
}

/******************** LOOP ********************/
void loop() {
  if (Blynk.connected()) Blynk.run();

  /******** BUTTON ********/
  static bool lastBtn = HIGH;
  bool nowBtn = digitalRead(BTN);

  if (lastBtn == HIGH && nowBtn == LOW) {
    systemActive = !systemActive;

    Serial.print("Button -> System: ");
    Serial.println(systemActive ? "ON" : "OFF");

    digitalWrite(LED_SYS, systemActive);

    if (systemActive)
      updateLCD("System: ON", "Monitoring...");
    else {
      updateLCD("System: OFF", "Good Bye!");
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED_ALERT_2, LOW);
      digitalWrite(LED_ALERT_3, LOW);
    }

    delay(250);
  }
  lastBtn = nowBtn;

  if (!systemActive) return;

  alertEye = alertHunch = alertLight = alertWork = false;

  float eyeDist = getDistance(TRIG_EYE, ECHO_EYE);
  delay(50);
  float backUp = getDistance(TRIG_BACK_UP, ECHO_BACK_UP);
  delay(50);
  float backLow = getDistance(TRIG_BACK_LOW, ECHO_BACK_LOW);

  int lightRaw = analogRead(PIN_LIGHT);
  float lux = (lightRaw / 4095.0) * 3.3 * 1000;

  /******** EYE ********/
  if (eyeDist < CLOSE_THRESHOLD && eyeDist > 2) {
    if (!isClose) {
      closeStart = millis();
      isClose = true;
    }
    if (millis() - closeStart > CLOSE_DELAY) {
      alertEye = true;
      Serial.println("⚠️ Too Close!");
    }
  } else {
    if (isClose) closeTotal += (millis() - closeStart);
    isClose = false;
  }

  /******** POSTURE ********/
  if (backLow < 50) {
    if (!isSitting) {
      sessionStart = millis();
      isSitting = true;
    }

    if (backUp - backLow > HUNCH_DIFF) {
      if (!isHunch) {
        hunchStart = millis();
        isHunch = true;
      }
      if (millis() - hunchStart > 5000) {
        hunchCount++;
        alertHunch = true;
        Serial.println("⚠️ Bad Posture!");
        isHunch = false;
      }
    } else isHunch = false;

    if (millis() - sessionStart > WORK_LIMIT) {
      alertWork = true;
      Serial.println("⚠️ Work too long!");
    }

  } else {
    if (isSitting) {
      totalWorkAccumulated += (millis() - sessionStart);
      isSitting = false;
    }
  }

  /******** LIGHT ********/
  if (lux < LIGHT_THRESHOLD) {
    alertLight = true;
    digitalWrite(LED_ALERT_3, HIGH);
    Serial.println("⚠️ Low Light!");
  } else digitalWrite(LED_ALERT_3, LOW);

  printDebug(eyeDist, backUp, backLow, lux);

  /******** OUTPUT ********/
  if (alertEye) currentBeep = EYE;
  else if (alertHunch) currentBeep = HUNCH;
  else if (alertWork) currentBeep = WORK;
  else currentBeep = NONE;

  digitalWrite(LED_ALERT_2, (alertEye || alertHunch || alertWork));

  handleBuzzer(currentBeep);

  /******** BLYNK ********/
  if (millis() - lastSend > 5000) {
    lastSend = millis();

    unsigned long currentTotalWork =
      totalWorkAccumulated + (isSitting ? (millis() - sessionStart) : 0);

    unsigned long currentCloseTotal =
      closeTotal + (isClose ? (millis() - closeStart) : 0);

    if (Blynk.connected()) {
      Blynk.virtualWrite(V0, currentTotalWork / 60000);
      Blynk.virtualWrite(V1, currentCloseTotal / 60000);
      Blynk.virtualWrite(V2, hunchCount);
    }
  }
}
