const int trig = 10;
const int echo = 9;

void setup() {
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

void loop() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH);
  float distance = duration * 0.034 / 2;

  Serial.println(distance); //วิธี1
  Serial.print("Distance in CM: ");
  Serial.println(duration / 58); //วิธี2
  delay(500);
}
