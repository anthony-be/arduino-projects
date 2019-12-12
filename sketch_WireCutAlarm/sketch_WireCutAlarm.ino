/*

Author: anthony-be
Source: https://github.com/anthony-be/arduino-projects

++++++ Description ++++++

This sketch is made of:
 * a long conductive wire
 * a relay
 * a siren

Its goal is to constantly monitor the status of the wire, it must be Normally Close (NC).
When the wire is broken, it becomes Open, which triggers the relay for a specific amount of time. 
The relay itself opens the power supply of the alarm to make it louding.
In addition, the LED embedded on the board is turned ON when alarm is triggered.

The actual use case is to monitor an area, surrounded by an almost invisible copper wire.
If someone enters the monitored area, it will break or tear off the wire which will trigger a siren (via the relay).

++++++ Components & Wiring diagram ++++++

         
         +-----------------+
         |        R        |
         | [A]NO  E   -[B] |
         | [P]C   L   +[B] |
         | [ ]NC  A   S[B] |
         |        Y        |
         +-----------------+

         
         +----[PWR]-------------------| USB |--+
         |                            +-----+  |
         |         GND/RST2  [ ][ ]            |
         |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] |   C5 
         |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   C4 
         |                             AREF[ ] |
         |                              GND[W] |
         | [ ]N/C                    SCK/13[ ] |   B5        
         | [ ]IOREF                 MISO/12[ ] |   .         
         | [ ]RST                   MOSI/11[ ]~|   .         
         | [ ]3V3    +---+               10[ ]~|   .         
         | [R]5v    -| A |-               9[ ]~|   .         
         | [R]GND   -| R |-               8[ ] |   B0
         | [P]GND   -| D |-                    |
         | [P]Vin   -| U |-               7[ ] |   D7
         |          -| I |-               6[ ]~|   .
         | [ ]A0    -| N |-               5[R]~|   .
         | [ ]A1    -| O |-               4[ ] |   .
         | [ ]A2     +---+           INT1/3[ ]~|   .
         | [ ]A3                     INT0/2[W] |   .
         | [ ]A4/SDA  RST SCK MISO     TX>1[ ] |   .
         | [ ]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0
         |            [ ] [ ] [ ]              |
         |  UNO_R3    GND MOSI 5V  ____________/
          \_______________________/
   
   http://busyducks.com/ascii-art-arduinos

  --------------------------------------------------
   [B]    Arduino board
  --------------------------------------------------
  
  --------------------------------------------------
   [R]    Relay
  --------------------------------------------------
   The relay used is a Velleman VMA406, based on srd-05vdc-sl-c.
    
   * + (positive) of the relay is connected to the 5v PIN of the board
   * - (negative) of the relay is connected to a GND PIN of the board
   * S (signal) of the relay is connected to the D5 PIN of the board, configured as OUTPUT pin

   * NO (Normally Open) PIN of the relay is connected to the + (positive) PIN of the polarized siren/loud
   * C (Common) PIN of the relay is connected to the + (positive) of the power supply

  --------------------------------------------------
   [W]    Wire
  --------------------------------------------------
   Monitored wired is made of copper, generally used in winding, but any other conductive wire is ok.
   * One side of wire is connected to a GND pin of the board 
   * Other side of wire is connected to D2 pin of the board, configured as INPUT_PULLUP pin

  --------------------------------------------------
   [P]    Power
  --------------------------------------------------

   The main power supply of the installation. 
   
   Its voltage depends of the Siren/Loud that you plan to use, 
   but do not forget to consider also the maximum voltage allowed on Vin PIN of the board.
   
   * The + (positive) of the supply is connected to:
       * Vin PIN of the board
       * C (common) PIN of the Relay

   * The - (negative) of the supply is connected to:
       * GND PIN of the board 
       * - (negative) PIN of the polarized siren/loud
       

  --------------------------------------------------
   [A]    Siren/Loud
  --------------------------------------------------

    The siren/loud triggered by the relay when wire is broken. 

    * The + (positive) of the loud is connected to the NO (Normally Open) PIN of the relay
    * The - (negative) of the liud is connected to the - (negative) of the Power Supply

*/

// -------------------------------
//     Constants
// -------------------------------
// The PIN of the embedded LED
const int led = LED_BUILTIN; 

// The PIN of the monitored wire
const int loopPin = 2; 

// The PIN to the Relay's signal 
const int relayPin = 5; 

//Time in seconds on which relay is open, in case of alarm
const int maxTriggerTime = 30; 

// -------------------------------
//     Global variable
// -------------------------------

unsigned long triggerTime = 0;

// -------------------------------
//     Setup
// -------------------------------
void setup() {
  //start serial connection
  //Serial.begin(9600);

  // Configure loopPin as an input and enable the internal pull-up resistor
  pinMode(loopPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  pinMode(relayPin, OUTPUT);

}

// -------------------------------
//     Loop
// -------------------------------
/*
 * 1. Read the current status of the wire
 * 2. If wire is not Closed, trigger the alarm
 * 2a. If wire is Closed and alarm has been triggered, disable the alarm and reset trigger
 * 2b. If wire is Closed and no alarm has been triggered, do nothing
 * 3. Sleep system for 250 ms
 * 
 */
void loop() {
  //READ Loop
  boolean isLoopClosed = digitalRead(loopPin) == LOW;
  // Keep in mind the pull-up means the pushbutton's logic is inverted. It goes
  // HIGH when it's open, and LOW when it's pressed.

  
  
  if( isLoopClosed == false ){
    triggerAlarm();
  }else if(triggerTime != 0){
    disableAlarm();
  }


//  Serial.print("Closed: ");
//  Serial.print(isLoopClosed);
//  Serial.print("  -  LED:   ");
//  Serial.print(digitalRead(led));
//  Serial.print("  - RELAY:  ");
//  Serial.println(digitalRead(relayPin));

  delay(250); //Sleep for 250ms
}

/*
 * Stop the triggering of the alarm.
 * Turn OFF the relay's signal and clear the trigger time.
 * 
 * Note: The LED is not turned OFF to keep track that alarm has been triggered once since the startup of the board.
 */
void disableAlarm(){
  // Keep the LED turned ON if alarm has been trigger once
  //lightLED(false);

  // Stop the relay's signal
  signalRelay(false);
  triggerTime = 0;
}

/*
 * Trigger the alarm:
 *  - Turn on the LED
 *  - Send signal to relay, while max trigger time has not been reached
 */
void triggerAlarm(){
  lightLED(true);

  boolean relay = checkWithMaxTriggerTime();
  signalRelay(relay);
}

/*
 * Check whether the maximum trigger time has been reached or not:
 * 
 * 1. Retrieve the current time with the help of millis() function
 * 2. If trigger time has not yet been initialized, initialze it with current time
 * 3. Return true -> Trigger can occur
 * 
 * Alternative:
 * 2a. If trigger time has already been initialized, calculate the difference (in seconds) between the current time and the initial trigger time
 * 3a. Return true if difference in seconds is less or equals to max triger time -> Trigger can occur
 * 3b. Return false if difference in seconds is greater than max trigger time -> Trigger cannot occur
 */
boolean checkWithMaxTriggerTime(){
  boolean trigger = true;
  unsigned long currentMillis = millis();
  if(triggerTime == 0){
    triggerTime = currentMillis;
  }else{
    long diff = currentMillis - triggerTime;
    int seconds = diff/1000;
    if(seconds > maxTriggerTime){
      trigger = false;
    }
  }
  return trigger;
}

/*
 * Turn ON or OFF the LED depending on given boolean parameter value:
 *  - true: turn ON the LED
 *  - false: turn OFF the LED
 */
void lightLED(boolean turnON){
  turnON ? digitalWrite(led, HIGH) : digitalWrite(led, LOW);
}

/*
 * Turn ON or OFF the Relay depending on given boolean parameter value:
 *  - true: turn ON the Relay
 *  - false: turn OFF the Relay
 */
void signalRelay( boolean signalON ){
  signalON ? digitalWrite(relayPin, HIGH) : digitalWrite(relayPin, LOW);
}
