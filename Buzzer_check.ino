#define BUZZER_PIN 25

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  for(int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
  
  delay(2000); // เว้นก่อนรอบใหม่
}
