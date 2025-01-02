int potPin = 34;
int buttonPin = 27;
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
}

void loop() {
  if (gamePaused) {
    // read potpin to gather level.
    potValue = analogRead(potPin);  // Read the potentiometer value (0-4095)

    // Map the raw value to 4 discrete steps: 0, 1, 2, 3
    stepValue = map(potValue, 0, 4095, 1, 3);

  } else {
    // send level to slaves, and generate and send angle.
    randomSeed(esp_random());  // Seed the random generator with hardware RNG
    int randomValue = random(0, 161);

    Serial.print(randomValue);

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
    // read data from slaves to computate winner.
    String data1 = Serial2.readStringUntil('\n');
    Serial.println("ESP32-1: " + data1);

    String data2 = Serial1.readStringUntil('\n');
    Serial.println("ESP32-2: " + data2);
  }
}
