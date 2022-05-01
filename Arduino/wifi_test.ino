#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AES_config.h>
#include <AES.h>
#include <AESLib.h>
#include <xbase64.h>
#include <secrets.h>


WiFiUDP Udp;
unsigned int localUdpPort = 4210;					  // local port to listen on
char incomingPacket[255];							  // buffer for incoming packets
char replyPacket[] = "Hi there! Got the message :-)"; // a reply string to send back

void setup()
{
	Serial.begin(115200);
	Serial.println();

	WiFi.begin(wlan_ssid, wlan_pass);

	Serial.print("Connecting");
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println();

	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());

	Udp.begin(localUdpPort);
	Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void loop()
{
	int packetSize = Udp.parsePacket();
	if (packetSize)
	{
		// receive incoming UDP packets
		Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
		int len = Udp.read(incomingPacket, sizeof(incomingPacket));
		if (len > 0)
		{
			incomingPacket[len] = 0;
		}
		Serial.printf("UDP packet contents: %s\n", incomingPacket);

		// send back a reply, to the IP address and port we got the packet from
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		Udp.write(replyPacket);
		Udp.endPacket();
	}
}
