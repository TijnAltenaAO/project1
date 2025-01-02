#include <MPU9250_WE.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MPU9250_ADDR 0x68
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

MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

int servo_angle;

void setup() {
  Serial.begin(9600);
  myServo.attach(SERVO_PIN, 500, 2500);  // 500 µs to 2500 µs corresponds to 0° to 180°


  Wire.begin();
  if (!myMPU9250.init()) {
    Serial.println("MPU9250 does not respond");
  } else {
    Serial.println("MPU9250 is connected");
  }

  Serial.println("Position you MPU9250 flat and don't move it - calibrating...");
  delay(1000);
  myMPU9250.autoOffsets();
  Serial.println("Done!");
  myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);
  myMPU9250.enableAccDLPF(true);
  myMPU9250.setAccDLPF(MPU9250_DLPF_6);

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
  xyzFloat angle = myMPU9250.getAngles();
// maybe write logic so it cant go from 90->-Q, yk.
  Serial.print("Angle z   = ");
  Serial.print(angle.z);
  Serial.println();

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println(angle.x);
  display.display();

  myServo.write(angle.x + 90);
  delay(100);
}
