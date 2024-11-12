#include <Wire.h>
#include <LCDI2C.h>

// กำหนดที่อยู่ของ I2C ของจอ LCD
LCDI2C lcd(0x27, 16, 2); 

// กำหนดค่าคงที่
const int hallPin = 2; // ขาที่เชื่อมต่อกับเซ็นเซอร์ Hall
const int resetButtonPin = 3; // ขาที่เชื่อมต่อกับปุ่มรีเซ็ต
const float wheelCircumference = 205.0; // เส้นรอบวงของล้อ (เซนติเมตร)
volatile int pulseCount = 0; // นับจำนวนรอบที่เกิดขึ้น
float distanceKm = 0.0; // ระยะทางในกิโลเมตร
unsigned long lastPulseTime = 0; // เวลาของรอบล่าสุด
float speedKmh = 0.0; // ความเร็วใน km/h

// ฟังก์ชันสำหรับนับรอบ
void countPulse() {
  unsigned long currentTime = millis(); // เวลาปัจจุบัน
  unsigned long timeDifference = currentTime - lastPulseTime;

  // คำนวณความเร็ว (km/h) เมื่อมีการนับรอบ
  if (timeDifference > 0) { // ป้องกันการหารด้วย 0
    speedKmh = (wheelCircumference / 100000.0) * 3600.0 / (timeDifference / 1000.0);
  }

  // เพิ่มระยะทาง (km) ในแต่ละรอบ
  distanceKm += (wheelCircumference / 100000.0);

  pulseCount++;
  lastPulseTime = currentTime; // อัปเดตเวลา
}

void setup() {
  lcd.init(); 
  lcd.begin(16, 2); // เริ่มต้นจอ LCD โดยระบุขนาด 16x2
  lcd.backlight(); // เปิดไฟแบ็คไลท์ของจอ LCD
  pinMode(hallPin, INPUT_PULLUP); // ตั้งค่าเซ็นเซอร์ Hall เป็น INPUT
  pinMode(resetButtonPin, INPUT_PULLUP); // ตั้งค่าปุ่มรีเซ็ตเป็น INPUT
  
  // ข้อความเริ่มต้น
  lcd.print("Speed: 00 km/h");
  lcd.setCursor(0, 1); // ไปยังแถวที่ 2
  lcd.print("Distance: 00.00 km");

  attachInterrupt(digitalPinToInterrupt(hallPin), countPulse, RISING); // ติดตั้ง Interrupt
}

void loop() {
  // ตรวจสอบสถานะของปุ่มรีเซ็ต
  static bool lastResetButtonState = HIGH;
  bool resetButtonState = digitalRead(resetButtonPin);

  if (lastResetButtonState == HIGH && resetButtonState == LOW) {
    // รีเซ็ตค่าทั้งหมด
    pulseCount = 0;
    distanceKm = 0.0;
    speedKmh = 0.0;
    
    // แสดงค่าที่รีเซ็ต
    lcd.clear();
    lcd.print("Speed: 00 km/h");
    lcd.setCursor(0, 1); // ไปยังแถวที่ 2
    lcd.print("Distance: 00.00 km");
    delay(200); // เพิ่มเลทให้เพื่อหลีกเลี่ยงการอ่านค่าหลายครั้งจากการกดปุ่ม
  }

  lastResetButtonState = resetButtonState; // อัปเดตสถานะปุ่ม

  unsigned long currentTime = millis();
  
  // ตรวจสอบว่าผ่านไปนานกว่า 2 วินาทีโดยไม่มีรอบใหม่หรือไม่
  if (currentTime - lastPulseTime > 2000) {
    speedKmh = 0.0; // ตั้งค่าความเร็วเป็น 0.00
  }

  // แปลงความเร็วเป็นจำนวนเต็ม
  int speedInt = (int)speedKmh;

  // แยกค่าส่วนจำนวนเต็มและทศนิยมของ distanceKm
  int distanceInt = (int)distanceKm;
  int distanceDecimal = (int)((distanceKm - distanceInt) * 100); // คูณ 100 เพื่อได้ทศนิยมสองตำแหน่ง

  // แสดงค่าความเร็วบนจอ LCD (ไม่มีทศนิยม)
  lcd.setCursor(6, 0);
  lcd.print(speedInt);
  lcd.print(" km/h  ");

  // แสดงค่าระยะทางบนจอ LCD
  lcd.setCursor(10, 1);
  lcd.print(distanceInt);
  lcd.print(".");
  if (distanceDecimal < 10) lcd.print("0"); // เพิ่มเลข 0 ถ้าทศนิยมมีค่าน้อยกว่า 10
  lcd.print(distanceDecimal);
  lcd.print(" km  ");

  delay(500); // หน่วงเวลาการอัปเดตจอ LCD เล็กน้อยเพื่อให้แสดงผลได้อย่างต่อเนื่อง
}
