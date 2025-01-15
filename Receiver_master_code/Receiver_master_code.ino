#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>
#include <WiFi.h>

#define MPU9250_ADDR 0x68
// Define screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Define I2C address for ssd1306
#define OLED_ADDR 0x3C

// Create SSD1306 object and servo object.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo myServo;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  int x;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;

// Create an array with all the structures
struct_message boardsStruct[2] = { board1, board2 };

int potPin = 34;
int buttonPin = 27;
int servoPin = 18;
int winsP1 = 0;
int winsP2 = 0;
int rounds = 0;
int potValue;
int stepValue;
int levelTime;

volatile bool gamePaused = true;  // game paused or not.

// Interrupt Service Routine (ISR) for the button
void IRAM_ATTR handleButtonPress() {
  gamePaused = !gamePaused;  // Toggle game paused state
}

// callback function that will be executed when data is received
int OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  boardsStruct[myData.id - 1].x = myData.x;
  return boardsStruct[myData.id - 1].x;
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(500);

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  pinMode(potPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

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
  display.println("BIER");
  display.display();
  delay(1000);
}

String decideWinner(int base, int val, int val1) {
  int diff = abs(val - base);
  int diff1 = abs(val1 - base);

  // Check if < suffices. maybe implement = case?
  if (diff == diff1) {
    return "None";
  }

  return (diff < diff1) ? "p1" : "p2";
}

void startGameCountdown() {
  for (int i = 3; i > 0; i--) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println(i);
    display.display();

    delay(1000);

    if (i == 1) {
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("Go!");
      display.display();

      delay(1000);
    }
  }
}

void displayRandomValue() {
  display.clearDisplay();
  display.setTextSize(3);  // Normal size
  display.setCursor(0, 10);
  display.println(randomValue - 90);
  display.display();
}

void loop() {
  if (gamePaused) {
    // read potpin to gather level.
    potValue = analogRead(potPin);  // Read the potentiometer value (0-4095)
    // Map the raw value to 5 discrete steps: 1, 2, 3, 4 and 5.
    stepValue = map(potValue, 0, 4095, 1, 5);

    // show level and wins of players when not playing.
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("Level: ");
    display.println(stepValue);
    display.println("Wins: ");
    display.print(winsP1);
    display.print(" - ");
    display.print(winsP2);
    display.display();
  } else {
    // uncomment to reset win counter after pressing play again.
    // winsP1 = 0;
    // winsP2 = 0;

    // quick statemachine to check for level.
    switch (stepValue) {
      case 1:
        levelTime = 5000;
        rounds = 3;
        break;
      case 2:
        levelTime = 2500;
        rounds = 5;
        break;
      case 3:
        levelTime = 1000;
        rounds = 10;
        break;
      case 4:
        levelTime = 500;
        rounds = 10;
        break;
      case 5:
        levelTime = 250;
        rounds = 10;
        break;
    }

    startGameCountdown();


    for (int i = 0; i < rounds; i++) {
      // send level to slaves, and generate and send angle.
      randomSeed(esp_random());  // Seed the random generator with hardware RNG
      int randomValue = random(0, 180);
      myServo.write(randomValue);  // also write angle to master servo.

      displayRandomValue();

      delay(levelTime);  // delay for desired levelTime

      // read angles from slaves.
      int data1 = boardsStruct[0].x;
      int data2 = boardsStruct[1].x;

      Serial.println(data1 - 90);  // log for testing purposes.
      Serial.println(data2 - 90);

      // decide winner
      String winner = decideWinner(randomValue, data1, data2);

      if (winner == "None") {
        display.clearDisplay();
        display.setCursor(0, 10);
        display.println("Quitte");
        display.setTextSize(2);  // Normal size
        display.print(winsP1);
        display.print(" - ");
        display.print(winsP2);
        display.display();
      } else {
        display.clearDisplay();
        display.setCursor(0, 10);
        display.println(winner + " wint");
        display.setTextSize(2);  // Normal size
        display.print(winsP1);
        display.print(" - ");
        display.print(winsP2);
        display.display();
      }

      delay(levelTime);

      // check if possible to include in decision function. possible scope issues kept me from doing it before.
      if (winner == "p1") {
        winsP1++;
      } else if (winner == "p2") {
        winsP2++;
      }

      gamePaused = true;
    }
  }
}
