#define BUZZER_PIN 25

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  digitalWrite(BUZZER_PIN, HIGH); // ติด
  delay(1000);

  digitalWrite(BUZZER_PIN, LOW);  // ดับ
  delay(1000);
}
