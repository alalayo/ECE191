
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio



#define syncStateA 10
#define syncStateB 11
#define debugPinC 12

#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin
#define rfRxPin 9 // rf Receive input pin
#define LEDPin 13 // Onboard LED

// Parameters for sync detection
#define minBitPeriod 450  // Low threshold for bit period (uS)
#define maxBitPeriod 550  // Max threshold for bit period (uS)
#define minSyncBits 8     // Min number of valid 1 plus 0 transitions before 1 plus 1

int maximumRange = 200; // Maximum range needed (cm)
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance
long timeout = 50000;  // Ping duration timeout

int received [1];

void setup() {
  pinMode(syncStateA, OUTPUT) ;
  pinMode(syncStateB, OUTPUT) ;
  pinMode(debugPinC, OUTPUT) ;

  Serial.begin (9600);

  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(rfRxPin, INPUT);
  pinMode(LEDPin, OUTPUT) ;
  digitalWrite(LEDPin, LOW) ;
  digitalWrite(trigPin, LOW) ;
}

void loop() {
  RxRFSync() ;    // Function blocks until sync sequence is detected
  digitalWrite(LEDPin,HIGH) ;
  digitalWrite(trigPin, HIGH);    // Start ping sequence
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, timeout);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 29.1 ;

  if (distance >= maximumRange || distance <= minimumRange) {
    /* Send a negative number to computer */
    Serial.println("-1");
  }
  else {
    /* Send the distance to the computer using Serial protocol */
    Serial.println(distance);
  }
  digitalWrite(LEDPin,LOW) ;
}

// Function to detect "syncSeq" bit sequence on RF receive port
void RxRFSync()
{
  long int lastBitTime = 0 ;    // holds time of last bit transition (uS)
  int BitCount = 0 ;            // counts number of valid bits detected

  boolean synced = false ;
  boolean lastBit = LOW ;  // holds last bit detected

  digitalWrite(syncStateA, LOW) ; digitalWrite(syncStateB, LOW) ; digitalWrite(debugPinC, LOW) ;

  while (!synced)
  {
    while (radio.Read(received) == lastBit) { } // Block until bit transition
    int currBitTime = micros() ;
    int bitPeriod = currBitTime - lastBitTime ;

    if ((bitPeriod > (2 * minBitPeriod)) && (bitPeriod < (2 * maxBitPeriod)) && (lastBit == HIGH) && (BitCount > minSyncBits))
    {
      // Valid completion of sync sequence
      synced = true ;
      digitalWrite(syncStateA, HIGH) ; digitalWrite(syncStateB, HIGH) ; digitalWrite(debugPinC, lastBit) ;
    }
    else
    {
      lastBit = !lastBit ;
      lastBitTime = currBitTime ;
      if ((bitPeriod > minBitPeriod) && (bitPeriod < maxBitPeriod))
      {
        // Valid single bit detected, increment valid bit count and look for next bit
        BitCount += 1 ;     // increment valid bit count
        digitalWrite(syncStateA, LOW) ; digitalWrite(syncStateB, HIGH) ; digitalWrite(debugPinC, lastBit) ;
      }
      else
      {
        // Invalid bit detected, reset valid bit count and look for next bit
        BitCount = 0 ;
        digitalWrite(syncStateA, HIGH) ; digitalWrite(syncStateB, LOW) ; digitalWrite(debugPinC, lastBit) ;
      }
    }
  }
}
