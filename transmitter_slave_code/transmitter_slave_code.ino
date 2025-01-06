#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;

// Define screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Define I2C address for ssd1306
#define OLED_ADDR 0x3C

// Create SSD1306 object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#include <ESP32Servo.h>

// Define servo object
Servo myServo;

// Define GPIO pin for the servo
#define SERVO_PIN 18


int servo_angle;

void setup() {
  Serial.begin(9600);
  myServo.attach(SERVO_PIN, 500, 2500);  // 500 µs to 2500 µs corresponds to 0° to 180°

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  if (!display.begin(SSD1306_PAGEADDR, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  // Clear display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(3);  // Normal size
  display.setTextColor(SSD1306_WHITE);

  // Display text
  display.setCursor(0, 10);
  display.println("Hello, ESP32!");
  display.display();
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  int xAng = map(AcX, minVal, maxVal, -90, 90);
  int yAng = map(AcY, minVal, maxVal, -90, 90);
  int zAng = map(AcZ, minVal, maxVal, -90, 90);
  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);

  // Serial.print("AngleX= ");

  if (x > 90 && x < 270) {
    int diffX90 = abs(x - 90);
    int diffX270 = abs(x - 270);

    if (diffX90 > diffX270) {
      Serial.println(diffX90);
      Serial.println(diffX270);
      Serial.println(x);
      x = 180;
    } else {
      // check if could be 0? weird behavior lol.
      x = 90;
    }
  }

  if (x <= 90) {
    x = map(x, 0, 90, 90, 0);
  } else if (x > 270) {
    x = map(x, 270, 360, 180, 90);
  }

  Serial.println(x);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println(x);
  display.display();

  myServo.write(x);
}
