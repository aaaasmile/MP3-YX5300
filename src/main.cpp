#include <Arduino.h>

int Ledpin = LED_BUILTIN; // sul mio esp8266 è il pin marcato con D4, come mai?
                          // nella documentazione di AZ-Delivery: Bei der Lolin V3 liegt sie allerdings an "GPIO2 / D4".

void setup() {
  // put your setup code here, to run once:
  pinMode(Ledpin, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Ciao ");
  digitalWrite(Ledpin, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  // but actually the LED is on; this is because
  // it is active low on the ESP-01)
  delay(500);
  Serial.print("Mondo\n");
  digitalWrite(Ledpin, LOW);  // Turn the LED off by making the voltage HIGH
  delay(500);              
}