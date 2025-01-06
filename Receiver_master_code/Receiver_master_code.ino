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
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);

  myServo.attach(servoPin, 500, 2500);  // 500 µs to 2500 µs corresponds to 0° to 180° (figure out if not limited to 120 or 160)

  // Initialize display
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
  display.display();
}

String decideWinner(int base, int val, int val1) {
  int diff = abs(val - base);
  int diff1 = abs(val1 - base);

  // Check if < suffices. maybe implement = case?
  return (diff < diff1) ? "p1" : "p2";
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
        levelTime = 5000;
        break;
      case 2:
        levelTime = 10000;
        break;
      case 3:
        levelTime = 15000;
        break;
    }

    Serial1.print(levelTime);
    Serial2.print(levelTime);

    // also write angle to master servo.
    myServo.write(randomValue);
    Serial1.print(randomValue);
    Serial2.print(randomValue);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println(randomValue);
    display.display();

    delay(levelTime);
    // read angles from slaves.
    while (Serial1.available() > 0) {
      Serial1.read();
    }
    while (Serial2.available() > 0) {
      Serial2.read();
    }

    int data1 = Serial1.readStringUntil('\n').toInt();
    int data2 = Serial2.readStringUntil('\n').toInt();
    
    Serial.println(data1);
    Serial.println(data2);

    // decide winner
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("W = " + decideWinner(randomValue, data1, data2));
    display.display();
    delay(5000);

    gamePaused = true;
  }
}
