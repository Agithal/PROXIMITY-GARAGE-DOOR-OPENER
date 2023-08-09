#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 49
#define SS_PIN 53
#define SENSOR_PIN 2
#define LED_PIN 13

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // I2C LCD module instance (adjust the address if needed)

const int motorPin1 = 3;  // Motor pin 1
const int motorPin2 = 4;  // Motor pin 2

void setup() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.begin(16, 2);  // Initialize the LCD module
  lcd.backlight();  // Turn on the backlight

  Serial.println("Ready to open the garage door");
}

void loop() {
  // Check for RFID tag
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // If RFID tag detected and no car is detected, open the garage door
    Serial.print("UID tag: ");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message: ");
    content.toUpperCase();
    lcd.clear();  // Clear the LCD display
    lcd.setCursor(0, 0);  // Set the cursor to the first line
    lcd.print("UID Tag:");
    lcd.setCursor(0, 1);  // Set the cursor to the second line
    lcd.print(content);

    if (digitalRead(SENSOR_PIN) == HIGH && content.substring(1) == "67 A8 76 62") {
      Serial.println("Access granted! Opening the garage door.");
      lcd.clear();
      lcd.print("Access granted!");
      lcd.setCursor(0, 1);
      lcd.print("Opening door...");
      digitalWrite(motorPin1, HIGH);
      digitalWrite(motorPin2, LOW);
      digitalWrite(LED_PIN, LOW);
      delay(1300); // wait for 1.3 seconds
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, LOW);
      delay(3000);
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, HIGH);
      delay(1300);
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, LOW);
      delay(1500);
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    } else {
      Serial.println("Door cannot be opened.");
      lcd.clear();
      lcd.print("Access denied!");
      lcd.setCursor(0, 1);
      lcd.print("Door locked.");
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      digitalWrite(LED_PIN, HIGH);
      delay(2000);
    }
  }

  // Check for car presence using IR sensor
  if (digitalRead(SENSOR_PIN) == LOW) {
    digitalWrite(LED_PIN, HIGH); // turn on LED
    lcd.clear();
    lcd.print("Car inside");
    lcd.setCursor(0, 1);
    lcd.print("Door locked.");
  } else {
    digitalWrite(LED_PIN, LOW); // turn off LED
    lcd.clear();
    lcd.print("No car inside");
    lcd.setCursor(0, 1);
    lcd.print("Door locked.");
  }
}

