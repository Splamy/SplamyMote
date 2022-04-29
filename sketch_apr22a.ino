#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <string>

const uint16_t kRecvPin = 5;
unsigned long key_value = 0;

const int greenPin = 0; //connected to GPIO pin 0 (D3 on a NodeMCU board).
const int redPin = 2; //connected to GPIO pin 2 (D4 on a NodeMCU board).

IRrecv irrecv(kRecvPin);

decode_results results;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
}
// the loop function runs over and over again forever
void loop() {
  if (irrecv.decode(&results)) {
    // print() & println() can't handle printing long longs. (uint64_t)
    serialPrintUint64(results.value, HEX);
    //Serial.println(std::to_string(results.value).c_str());
    Serial.println("");

    digitalWrite(greenPin, HIGH);
    delay(2000);
    digitalWrite(greenPin, LOW);
    
    key_value = results.value;
    irrecv.resume();  // Receive the next value
  }
  delay(100);
}
