
/* maker remote version3 by chiangmai maker club */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define buffer_size 100
#define PCF8591 (0x90 >> 1) // I2C bus address

#define period_udp 10
#define period_led 50


/* Set these to your desired credentials. */
const char *ssid = "kkao_server";
const char *password = "12345678";

IPAddress local_ip(192, 168, 5, 1);

unsigned int localPort = 12345;
int count = 0;
int ch1, ch2, ch3, ch4;
int _ch1, _ch2, _ch3, _ch4;

unsigned long time_now, time_prev_udp, time_prev_led;

char packetBuffer[buffer_size] = {0}; //buffer to hold incoming and outgoing packets
WiFiUDP udp;

void Read_adc();
void Read_adc_offset();
void Sent_UDP();
void d1_toggle();

void setup()
{
  Wire.begin();

  Wire.setClock(40000);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(16, 1);
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  digitalWrite(16, 0);
  // d1_on();

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  while (digitalRead(2))
  {
    Read_adc_offset();
    delay(50);
  }
  time_now = millis();
  time_prev_udp = time_now;
  time_prev_led = time_now;

}

void loop()
{
  time_now = millis();


  if (time_now - time_prev_udp >= period_udp)
  {
    time_prev_udp = time_now;
    digitalWrite(12, 1);		// debug
    if (digitalRead(2))
    {
      Read_adc();
    } else {
      Read_adc_offset();
    }
    Sent_UDP();
    digitalWrite(12, 0);    // debug

  }

  if (time_now - time_prev_led >= period_led)
  {
    time_prev_led = time_now;
    d1_toggle();
  }






  delay(1);
}
void Sent_UDP()
{
  packetBuffer[0] = (int8_t)ch1;
  packetBuffer[1] = (int8_t)ch2;
  packetBuffer[2] = (int8_t)ch3;
  packetBuffer[3] = (int8_t)ch4;
  packetBuffer[4] = '/0';

  udp.beginPacket(local_ip, localPort);
  udp.write(packetBuffer,5);
  udp.endPacket();


  memset(packetBuffer, 0, buffer_size);
}

void Read_adc_offset()
{
  Wire.beginTransmission(PCF8591); // wake up PCF8591
  Wire.write(0x04); // control byte - read ADC0 then auto-increment
  Wire.endTransmission(); // end tranmission
  Wire.requestFrom(PCF8591, 5);

  _ch4 = Wire.read();
  _ch4 = Wire.read();
  _ch1 = Wire.read();
  _ch2 = Wire.read();
  _ch3 = Wire.read();
}

void Read_adc()
{
  Wire.beginTransmission(PCF8591); // wake up PCF8591
  Wire.write(0x04); // control byte - read ADC0 then auto-increment
  Wire.endTransmission(); // end tranmission
  Wire.requestFrom(PCF8591, 5);

  int b_ch4 = Wire.read();
  b_ch4 = Wire.read();
  int b_ch1 = Wire.read();
  int b_ch2 = Wire.read();
  int b_ch3 = Wire.read();

  ch1 =  b_ch1 - _ch1;
  ch2 =  b_ch2 - _ch2;
  ch3 =  (-b_ch3 + _ch3);  // inversed
  ch3 = (float)ch3 / 1.5f;
  ch4 =  b_ch4 - _ch4;

  Serial.println(ch1);
  Serial.println(ch2);
  Serial.println(ch3);
  Serial.println(ch4);
  Serial.println("");
  Serial.println("");
}

void d1_toggle()
{
  static uint8_t _led = 0;
  digitalWrite(16, _led);
  _led = !_led;
}
