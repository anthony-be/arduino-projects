int led = LED_BUILTIN;

int loopPin = 2;
int relayPin = 5;

int maxTriggerTime = 30; //Time in seconds on which relay is open
unsigned long triggerTime = 0;

void setup() {
  //start serial connection
  //Serial.begin(9600);
  //configure pin 2 as an input and enable the internal pull-up resistor
  pinMode(loopPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  pinMode(relayPin, OUTPUT);

}

void loop() {
  //READ Loop
  boolean isLoopClosed = digitalRead(loopPin) == LOW;
  

  // Keep in mind the pull-up means the pushbutton's logic is inverted. It goes
  // HIGH when it's open, and LOW when it's pressed. Turn on pin 13 when the
  // button's pressed, and off when it's not:
  
  if( isLoopClosed == false ){
    triggerAlarm();
  }else{
    disableAlarm();
  }


//  Serial.print("Closed: ");
//  Serial.print(isLoopClosed);
//  Serial.print("  -  LED:   ");
//  Serial.print(digitalRead(led));
//  Serial.print("  - RELAY:  ");
//  Serial.println(digitalRead(relayPin));
}

void disableAlarm(){
  // LED
  lightLED(false);
  signalRelay(false);
  triggerTime = 0;
}

void triggerAlarm(){
  // LED
  lightLED(true);

  boolean relay = true;
  unsigned long currentMillis = millis();
  if(triggerTime == 0){
    triggerTime = currentMillis;
  }else{
    long diff = currentMillis - triggerTime;
    int seconds = diff/1000;
    if(seconds > maxTriggerTime){
      relay = false;
    }
  } 
  signalRelay(relay);
}

void lightLED(boolean turnON){
  turnON ? digitalWrite(led, HIGH) : digitalWrite(led, LOW);
}

void signalRelay( boolean signalON ){
  signalON ? digitalWrite(relayPin, HIGH) : digitalWrite(relayPin, LOW);
}
