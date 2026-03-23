/***************************************************
 * TEMT6000 Light Sensor Test Code (ESP32)
 * Pin: GPIO 34 (Analog)
 ***************************************************/

#define LIGHT_SENSOR_PIN 34

void setup() {
  Serial.begin(115200);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  Serial.println("TEMT6000 Test Starting...");
}

void loop() {
  // 1. อ่านค่า Raw จาก ADC (0 - 4095)
  int rawValue = analogRead(LIGHT_SENSOR_PIN);

  // 2. แปลงค่าเป็นแรงดันไฟฟ้า (Voltage)
  float voltage = (rawValue / 4095.0) * 3.3;

  // 3. คำนวณค่า Lux ตาม Datasheet
  // สมมติใช้ Load Resistor (RL) บนโมดูลขนาด 10k Ohm (10,000 Ohm)
  // กระแส I = V / R
  float amps = voltage / 10000.0;     // หน่วยเป็น Amps
  float microAmps = amps * 1000000.0; // แปลงเป็น Microamps (uA)
  
  // จาก Datasheet: 10uA ≈ 20 lux (ดังนั้น 1uA ≈ 2 lux)
  float lux = microAmps * 2.0;

  // แสดงผลผ่าน Serial Monitor
  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("V | Lux: ");
  Serial.print(lux, 2);
  Serial.println(" lx");

  delay(500); // อ่านค่าทุกๆ 0.5 วินาที
}
