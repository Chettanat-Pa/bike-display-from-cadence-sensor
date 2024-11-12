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
unsigned long currentTime = 0; // เวลาปัจจุบัน
float speedKmh = 0.0; // ความเร็วใน km/h

// ฟังก์ชันสำหรับนับรอบ
void countPulse() {
  pulseCount++;
}

void setup() {
  lcd.init(); 
  lcd.begin(16, 2); // เริ่มต้นจอ LCD โดยระบุขนาด 16x2
  lcd.backlight(); // เปิดไฟแบ็คไลท์ของจอ LCD
  pinMode(hallPin, INPUT_PULLUP); // ตั้งค่าเซ็นเซอร์ Hall เป็น INPUT
  pinMode(resetButtonPin, INPUT_PULLUP); // ตั้งค่าปุ่มรีเซ็ตเป็น INPUT
  
  // ข้อความเริ่มต้น
  lcd.print("Speed: 0.00 km/h");
  lcd.setCursor(0, 1); // ไปยังแถวที่ 2
  lcd.print("Distance: 00.00 km");

  attachInterrupt(digitalPinToInterrupt(hallPin), countPulse, RISING); // ติดตั้ง Interrupt
}

void loop() {
  currentTime = millis(); // เวลาปัจจุบัน

  // ตรวจสอบสถานะของปุ่มรีเซ็ต
  static bool lastResetButtonState = HIGH;
  bool resetButtonState = digitalRead(resetButtonPin);

  if (lastResetButtonState == HIGH && resetButtonState == LOW) {
    // รีเซ็ตค่าทั้งหมด
    pulseCount = 0;
    distanceKm = 0.0;
    speedKmh = 0.0;
    
    // แสดงค่าที่รีเซ็ต
    //lcd.clear();
    lcd.print("Speed: 00.00 km/h");
    lcd.setCursor(0, 1); // ไปยังแถวที่ 2
    lcd.print("Distance: 0.00 km");
    delay(200); // เพิ่มเลทให้เพื่อหลีกเลี่ยงการอ่านค่าหลายครั้งจากการกดปุ่ม
  }

  lastResetButtonState = resetButtonState; // อัปเดตสถานะปุ่ม

  // คำนวณความเร็วทุก 1 วินาที
  //if (currentTime - lastPulseTime >= 1000) {
  if (currentTime - lastPulseTime >= 1000) {
    speedKmh = (pulseCount * wheelCircumference / 100000.0) * 3600.0; // คำนวณความเร็ว (km/h)
    //speedMps = (wheelCircumference / 10000.0)
    distanceKm += (pulseCount * wheelCircumference / 100000.0); // คำนวณระยะทาง (km)

    // แสดงผลบนจอ LCD
    lcd.setCursor(7, 0);
    lcd.print(int(speedKmh));
    lcd.print(" km/h "); // เพิ่มช่องว่างเพื่อทำให้ตัวเลขไม่เบลอ
    lcd.setCursor(10, 1);
    lcd.print(distanceKm);
    lcd.print(" km  "); // เพิ่มช่องว่างเพื่อทำให้ตัวเลขไม่เบลอ

    // รีเซ็ตค่าการนับรอบ
    pulseCount = 0;
    lastPulseTime = currentTime; // อัปเดตเวลา
  }
}