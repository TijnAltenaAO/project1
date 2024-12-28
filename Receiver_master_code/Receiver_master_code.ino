void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);
  // Initialize UART1
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Replace RX1_PIN and TX1_PIN with your GPIOs
  
  // Initialize UART2
Serial1.begin(9600, SERIAL_8N1, 25, 26); // RX2_PIN = GPIO 25, TX2_PIN = GPIO 26

}

void loop() {

    String data1 = Serial2.readStringUntil('\n');
    Serial.println("ESP32-1: " + data1);


    String data2 = Serial1.readStringUntil('\n');
    Serial.println("ESP32-2: " + data2);
  // }
}
