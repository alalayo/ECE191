/*
  Transmit side of one way ultrasonic ranging using HC-SR04.
  Node sync is performed with 433 MHz OOK transmit receive pair via a sync sequence

  HC-SR04 Ping distance sensor:
  VCC to arduino 5v
  GND to arduino GND
  Echo to Arduino pin 7
  Trig to Arduino pin 8

  433 MHz Transmitter:
  Data to Arduino pin 9


*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio




#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin

#define LEDPin 13 // Onboard LED
#define BitPeriod 500   // Bit period for RF sync bits in microseconds

boolean syncSeq[16] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1} ; // Bit sequence for synchronization

int maximumRange = 200; // Maximum range needed (cm)
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance
long timeout = 50000;  // Ping duration timeout (uS)

// NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe

int high[1];
int low[1];

void setup() {
  Serial.begin (9600);

  radio.begin();
  radio.openWritingPipe(pipe);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
  digitalWrite(trigPin, LOW) ;
  radio.write(low, sizeof(low));
}

void loop() {
  digitalWrite(LEDPin, HIGH);
  TxRFSync() ;        // Send RF synchronization sequence
  digitalWrite(trigPin, HIGH);      // Start ping sequence
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, timeout);
  digitalWrite(LEDPin, LOW);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;

  if (distance >= maximumRange || distance <= minimumRange) {
    /* Send a negative number to computer */
    Serial.println("-1");
  }
  else {
    /* Send the distance to the computer using Serial protocol. */
    Serial.println(distance);
  }

  //Delay 100ms before next reading.
  delay(100);
}

// Function to write "syncSeq" bit sequence to RF transmit port
void TxRFSync()
{
  for (int k = 0; k < sizeof(syncSeq) ; k++)
  {
    radio.write(low, syncSeq[k]);
    delayMicroseconds(BitPeriod) ;
  }
    radio.write(low, sizeof(low));      // Turn off transmit at end of sequence
}
