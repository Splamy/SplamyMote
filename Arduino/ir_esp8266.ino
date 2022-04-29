#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <string>

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

const uint16_t kRecvPin = D1;
uint64_t key_value = 0;
bool toggle = false;
int lightOn = 0;

const int greenPin = D3;
const int redPin = D4;
const int irPin = D2;
const int blueButton = D5;

IRrecv irrecv(kRecvPin);
IRsend irsend(irPin, false, true);

decode_results results;

const uint64_t codes[] = {
	0xF700FF,
	0xF7807F,
	0xF740BF,
	0xF7C03F,
	0xF720DF,
	0xF7A05F,
	0xF7609F,
	0xF7E01F,
	0xF710EF,
	0xF7906F,
	0xF750AF,
	0xF7D02F,
	0xF730CF,
	0xF7B04F,
	0xF7708F,
	0xF7F00F,
	0xF708F7,
	0xF78877,
	0xF748B7,
	0xF7C837,
	0xF728D7,
	0xF7A857,
	0xF76897,
	0xF7E817,
};

const uint16_t sendOn[]{
	4500, 2250, 250, 250, 250, 750, 250, 250, 250, 250, 250, 250, 250, 750,
	250, 250, 250, 250, 250, 750, 250, 250, 250, 750, 250, 750, 250, 750, 250,
	250, 250, 750, 250, 750, 250, 250, 250, 250, 250, 750, 250, 750, 250, 250,
	250, 250, 250, 250, 250, 250, 250, 750, 250, 750, 250, 250, 250, 250, 250,
	750, 250, 750, 250, 750, 250, 750, 250};

const uint16_t sendOn2[]{
	9000, 4500, 500, 500, 500, 1500, 500, 500, 500, 500, 500, 500, 500, 1500,
	500, 500, 500, 500, 500, 1500, 500, 500, 500, 1500, 500, 1500, 500, 1500,
	500, 500, 500, 1500, 500, 1500, 500, 500, 500, 500, 500, 1500, 500, 1500,
	500, 500, 500, 500, 500, 500, 500, 500, 500, 1500, 500, 1500, 500, 500,
	500, 500, 500, 1500, 500, 1500, 500, 1500, 500, 1500, 500};

IRAM_ATTR void buttonPressed()
{
	toggle = true;
}

uint16_t round_to_nearest(uint16_t value, uint16_t nearest)
{
	return (value + (nearest / 2)) / nearest * nearest;
}

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(115200);
	irrecv.enableIRIn(); // Start the receiver
	irsend.begin();
	while (!Serial) // Wait for the serial connection to be establised.
		delay(50);
	Serial.println();
	Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
	Serial.println(kRecvPin);
	pinMode(greenPin, OUTPUT);
	pinMode(redPin, OUTPUT);
	pinMode(blueButton, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(blueButton), buttonPressed, RISING);
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
		//Serial.println(resultToTimingInfo(&results));
		//yield(); // Feed the WDT (again)
		// Output the results as source code
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
