#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <WiFiUdp.h>
#include <string>
#include <secrets.h>

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

bool toggle = false;
const uint16_t kRecvPin = D1;
const int greenPin = D3;
const int redPin = D4;
const int irPin = D2;
const int blueButton = D5;

uint8_t udpRecvBuffer[1500];
uint8_t udpSendBuffer[1500];
uint16_t irSendBuffer[128];
decode_results irDecodeBuffer;

IRrecv irrecv(kRecvPin);
IRsend irsend(irPin, false, true);

enum class Season
{
    Summer,
    Spring,
    Winter,
    Autumn
}; 

IRAM_ATTR void buttonPressed()
{
	toggle = true;
}

uint16_t round_to_nearest(uint16_t value, uint16_t nearest)
{
	return (value + (nearest / 2)) / nearest * nearest;
}

void blink_led(int pin, int times)
{
	for (int i = 0; i < times; i++)
	{
		digitalWrite(pin, HIGH);
		delay(100);
		digitalWrite(pin, LOW);
		delay(100);
	}
}

// the setup function runs once when you press reset or power the board
void setup()
{
	pinMode(greenPin, OUTPUT);
	pinMode(redPin, OUTPUT);
	pinMode(blueButton, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(blueButton), buttonPressed, RISING);

	Serial.begin(115200);
	irrecv.enableIRIn(); // Start the receiver
	irsend.begin();
	WiFi.begin(wlan_ssid, wlan_pass);

	while (!Serial)
	{
		blink_led(redPin, 1);
		delay(50);
	}

	blink_led(greenPin, 1);

	while (WiFi.status() != WL_CONNECTED)
	{
		blink_led(redPin, 1);
		delay(500);
	}

	blink_led(greenPin, 2);

	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());
}

// the loop function runs over and over again forever
void loop()
{
	if (toggle)
	{
		toggle = false;
		// lightOn = lightOn == 0 ? 1 : 0;
		// digitalWrite(redPin, lightOn == 1 ? HIGH : LOW);

		digitalWrite(redPin, HIGH);
		irsend.sendRaw(sendOn2, sizeof(sendOn2) / sizeof(uint16_t), 38);
		digitalWrite(redPin, LOW);
		delay(500);
	}

	if (irrecv.decode(&results))
	{
		// print() & println() can't handle printing long longs. (uint64_t)
		// serialPrintUint64(results.value, HEX);
		// Serial.print(" : ");

		Serial.print(resultToHumanReadableBasic(&results));
		yield();
		// Serial.println(resultToTimingInfo(&results));
		// yield(); // Feed the WDT (again)
		//  Output the results as source code
		Serial.println(resultToSourceCode(&results));
		Serial.println(""); // Blank line between entries
		yield();

		for (auto i = 0; i < results.rawlen; i++)
			results.rawbuf[i] = round_to_nearest(results.rawbuf[i], 250);

		Serial.println(resultToSourceCode(&results));
		Serial.println(""); // Blank line between entries
		yield();

		// for (auto i = 0; i < results.rawlen; i++)
		// {
		// 	Serial.print(results.rawbuf[i], DEC);
		// 	Serial.print(',');
		// }
		// Serial.println("");

		// size_t index = 0xFFFFFFFF;
		// size_t i = 0;
		// for (const auto code : codes)
		// {
		// 	if (results.value == code)
		// 	{
		// 		index = i;
		// 		break;
		// 	}
		// 	i++;
		// }

		digitalWrite(greenPin, HIGH);
		delay(100);
		digitalWrite(greenPin, LOW);

		// delay(50);
		// digitalWrite(greenPin, HIGH);

		// irsend.sendRaw(const_cast<const uint16_t *>(results.rawbuf), results.rawlen, 38);

		// digitalWrite(greenPin, LOW);

		key_value = results.value;

		irrecv.resume(); // Receive the next value
	}
	delay(100);
}
