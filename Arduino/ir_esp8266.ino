#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <WiFiUdp.h>
#include <string>
#include <secrets.h>

// #pragma GCC diagnostic error "-Wall"
// #pragma GCC diagnostic error "-Wextra"

enum class WorkState : uint8_t
{
	IPWait = 0,
	IRSend,
	IRLearn,
};
enum class PackMode : uint8_t
{
	Invalid = 0,
	BitMask = 1,
	RawU16 = 2,
};
enum class Pkg : uint8_t
{
	Invalid = 0,
	IpInit = 1 /* Host -> MCU */,
	IpOk = 2 /* Host <- MCU */,
	Set_IRLearn = 3 /* Host -> MCU */,
	Set_IRSend = 4 /* Host -> MCU */,
	IR_Data = 5 /* Host <-> MCU */,
};

template <typename T>
struct ArrView
{
	T *ptr;
	uint16_t count;

public:
	ArrView() : ptr(nullptr), count(0) {}
};

struct __attribute__((packed)) IRMessage
{
	static constexpr size_t DataSize = 256;
	Pkg pkg_type;
	PackMode pack_mode;
	uint16_t data_length;
	std::array<uint8_t, DataSize> data;

	template <typename T>
	ArrView<T> get()
	{
		ArrView<T> av;
		av.ptr = reinterpret_cast<T *>(data.data());
		av.count = (uint16_t)(std::min(DataSize, (size_t)data_length) / sizeof(T));
		return av;
	}

	template <typename T>
	void set(T volatile *t, size_t count)
	{
		T *write_buf = reinterpret_cast<T *>(data.data());
		for (int i = 0; i < count; i++)
			write_buf[i] = t[i];
		data_length = count * sizeof(T);
	}

	size_t get_size() const { return 4 + data_length; }
};

bool toggle = false;
WorkState state = WorkState::IPWait;

const uint16_t kRecvPin = D1;
const int greenPin = D3;
const int redPin = D4;
const int irPin = D2;
const int blueButton = D5;

const uint16_t localUdpPort = 58914;
IPAddress remoteIp;
uint16_t remotePort;
IRMessage udpRecvBuffer;
IRMessage udpSendBuffer;
uint16_t irSendBuffer[128];
decode_results irDecodeBuffer;

WiFiUDP Udp;
IRrecv irrecv(kRecvPin);
IRsend irsend(irPin, false, true);

uint8_t blink_count[32];
unsigned long blink_time[32];

IRAM_ATTR void buttonPressed()
{
	toggle = true;
}

void blink_led_delayed(int pin, int times)
{
	for (int i = 0; i < times; i++)
	{
		digitalWrite(pin, HIGH);
		delay(100);
		digitalWrite(pin, LOW);
		delay(100);
	}
}

void blink_led(int pin, int times)
{
	if (times == 0)
		return;
	blink_time[pin] = millis();
	blink_count[pin] = times * 2;
	digitalWrite(pin, HIGH);
}

void trigger_blinks()
{
	auto m = millis();
	for (size_t i = 0; i < sizeof(blink_count); i++)
	{
		auto bc = blink_count[i];
		if (bc > 0 && m - blink_time[i] > 100)
		{
			bc--;
			blink_time[i] = m;
			blink_count[i] = bc;

			if (bc == 0)
			{
				digitalWrite(i, LOW);
			}
			else
			{
				digitalWrite(i, bc % 2 == 0 ? HIGH : LOW);
			}
		}
	}
}

// the setup function runs once when you press reset or power the board
void setup()
{
	memset(blink_count, 0, sizeof(blink_count));
	memset(blink_time, 0, sizeof(blink_time));
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
		blink_led_delayed(redPin, 1);
		delay(50);
	}

	blink_led_delayed(greenPin, 1);

	while (WiFi.status() != WL_CONNECTED)
	{
		blink_led_delayed(redPin, 1);
		delay(500);
	}

	blink_led_delayed(greenPin, 2);
	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());

	Udp.begin(localUdpPort);
}

// the loop function runs over and over again forever
void loop()
{
	trigger_blinks();
	if (toggle)
	{
		toggle = false;
		blink_led(greenPin, 1);
		delay(500);
	}
	else if (state == WorkState::IRLearn)
	{
		// Serial.print("state == WorkState::IRLearn");
		if (irrecv.decode(&irDecodeBuffer))
		{
			Serial.print(resultToSourceCode(&irDecodeBuffer));

			udpSendBuffer.pkg_type = Pkg::IR_Data;
			udpSendBuffer.pack_mode = PackMode::RawU16;
			udpSendBuffer.set(irDecodeBuffer.rawbuf, irDecodeBuffer.rawlen);
			blink_led(greenPin, 2);

			Udp.beginPacket(remoteIp, remotePort);
			Udp.write(reinterpret_cast<uint8_t *>(&udpSendBuffer), udpSendBuffer.get_size());
			Udp.endPacket();

			irrecv.resume(); // Receive the next value
		}
	}
	else if (state == WorkState::IRSend)
	{
		// Serial.print("state == WorkState::IRSend");
	}

	int packetSize = Udp.parsePacket();
	if (packetSize)
	{
		int len = Udp.read(reinterpret_cast<uint8_t *>(&udpRecvBuffer), sizeof(udpRecvBuffer));
		Serial.print("Got pkg ");
		Serial.println(len);

		if (len < 4)
		{
			blink_led(redPin, 4);
			return;
		}

		if (state == WorkState::IPWait && udpRecvBuffer.pkg_type != Pkg::IpInit)
		{
			blink_led(redPin, 2);
			return;
		}

		switch (udpRecvBuffer.pkg_type)
		{
		case Pkg::IpInit:
			remoteIp = Udp.remoteIP();
			remotePort = Udp.remotePort();
			state = WorkState::IRSend;
			blink_led(greenPin, 1);
			break;
		case Pkg::Set_IRSend:
			state = WorkState::IRSend;
			blink_led(greenPin, 1);
			break;
		case Pkg::Set_IRLearn:
			state = WorkState::IRLearn;
			blink_led(greenPin, 1);
			break;

		case Pkg::IR_Data:
			if (state != WorkState::IRSend)
				break;

			if (udpRecvBuffer.pack_mode == PackMode::RawU16)
			{
				auto rawView = udpRecvBuffer.get<uint16_t>();
				irsend.sendRaw(rawView.ptr, rawView.count, 38);
			}
			else if (udpRecvBuffer.pack_mode == PackMode::BitMask)
			{
				blink_led(redPin, 3);
			}

		default:
			break;
		}
	}
	else
	{
		delay(10);
	}
}
