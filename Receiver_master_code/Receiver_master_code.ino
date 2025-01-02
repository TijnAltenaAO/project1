#include <ESP32Servo.h>
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

// Define servo object
Servo myServo;

int potPin = 34;
int buttonPin = 27;
int servoPin = 18;
int potValue;
int stepValue;
int levelTime;
volatile bool gamePaused = true;  // game paused or not.

// Interrupt Service Routine (ISR) for the button
void IRAM_ATTR handleButtonPress() {
  gamePaused = !gamePaused;  // Toggle game paused state
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);

  // Initialize UART1
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Replace RX1_PIN and TX1_PIN with your GPIOs

  // Initialize UART2
  Serial1.begin(9600, SERIAL_8N1, 25, 26);  // RX2_PIN = GPIO 25, TX2_PIN = GPIO 26

  pinMode(potPin, INPUT);

  // Attach interrupt to the button pin
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

  myServo.attach(SERVO_PIN, 500, 2500);  // 500 µs to 2500 µs corresponds to 0° to 180° (figure out if not limited to 120 or 160)

  // Initialize display
  if (!display.begin(SSD1306_I2C_ADDRESS, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  // Clear display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);  // Normal size
  display.setTextColor(SSD1306_WHITE);

  // Display text
  display.setCursor(0, 10);
  display.display();
}

void loop() {
  if (gamePaused) {
    // read potpin to gather level.
    potValue = analogRead(potPin);  // Read the potentiometer value (0-4095)

    // Map the raw value to 4 discrete steps: 0, 1, 2, 3
    stepValue = map(potValue, 0, 4095, 1, 3);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println(stepValue);
    display.display();

  } else {
    // send level to slaves, and generate and send angle.
    randomSeed(esp_random());  // Seed the random generator with hardware RNG
    int randomValue = random(0, 161);

    switch (stepValue) {
      case 1:
        levelTime = 1500;
        break;
      case 2:
        levelTime = 3000;
        break;
      case 3:
        levelTime = 4500;
        break;
    }

    Serial.print(levelTime);
    delay(levelTime);

    // also write angle to master servo.
    myServo.write(randomValue);
    Serial.print(randomValue);

    // read data from slaves to computate winner.
    String data1 = Serial2.readStringUntil('\n');
    Serial.println("ESP32-1: " + data1);

    String data2 = Serial1.readStringUntil('\n');
    Serial.println("ESP32-2: " + data2);
  }
}
