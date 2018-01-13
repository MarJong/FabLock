/* locking/unlocking system for the fablab cottbus e.v. door*/

#include <Arduino.h>
#include <A4988.h>

// using a 200-step motor
// pins used are: steps, DIR_pin, STEP_pin, enable_pin, -> not in use: MS1, MS2, MS3
A4988 stepper(200, 5, 9, 13);

#define Locked      6   // endstop for the locked position
#define Unlocked    7   // endstop for the unlocked position
#define Closed      8   // endstop for detecting whether the window 
#define rfidIdOk    3   // input from Raspberry PI
#define beep        10  // beeper
#define ledOpen     11  // shows the state of the open window
#define ledClosed   12  // shows the state of the closed window
#define ledPull     PORTC2
#define feedBackPi  PORTC3  // wenn 0 = alles ok, wenn 1 = Problem oder System arbeitet gerade
#define keySwitch   2       // key switch 

void setup() {
  Serial.begin(9600);     // uart init
  stepper.begin(120, 1);  // stepper init
  stepper.setSpeedProfile(LINEAR_SPEED, 100, 100);

  // pin init
  DDRC = 0xFF; 
  pinMode(Closed, INPUT_PULLUP);
  pinMode(Unlocked, INPUT_PULLUP);
  pinMode(Locked, INPUT_PULLUP);
  pinMode(rfidIdOk, INPUT_PULLUP);
  pinMode(beep, OUTPUT);
  pinMode(ledOpen, OUTPUT);
  pinMode(ledClosed, OUTPUT);
  pinMode(keySwitch, INPUT_PULLUP);
  
  // bring the system in the "closed" position
  stepper.enable();	
  while(digitalRead(Locked) == LOW){
    // wait for WindowClosed Endstop to be pressed
    if(digitalRead(Closed) == HIGH){
      Serial.println("closing");
      stepper.move(100); // locking window
    }
  }    
  stepper.disable();
  
  // start a checkup of the outputs
  analogWrite(beep, 10);
  digitalWrite(ledOpen, HIGH);
  digitalWrite(ledClosed, HIGH);
  PORTC |= (1 << ledPull);
  delay(1000);
  analogWrite(beep, 0);
  digitalWrite(ledOpen, LOW);
  digitalWrite(ledClosed, LOW);
  PORTC &= ~(1 << ledPull);
}

void loop() {
  // check the mechanism and if the window is closed
  if((digitalRead(Locked) == LOW) || (digitalRead(Unlocked) == HIGH) || (digitalRead(Closed) == LOW) || (digitalRead(keySwitch) == LOW)){
    Serial.println("Something is wrong / window not closed. Please close the window and bring the michanism in default position!");
    PORTC |= (1 << feedBackPi); //tell the Pi that the System is not ready
    analogWrite(beep, 10);
    PORTC |= (1 << ledPull);
    delay(1000);
    analogWrite(beep, 0);
    PORTC &= ~(1 << ledPull);
    delay(1000);
  }

  // everything is fine, system ready
  else {
    
    // wait for successfull rfid match through server
    if(digitalRead(rfidIdOk) == LOW) {
        PORTC |= (1 << feedBackPi); //tell the Pi that the system is working

        // open window
        stepper.enable();
        while(digitalRead(Unlocked) == LOW){
          Serial.println("Opening Window");
          stepper.move(-100); // unlocking window
        }// ends when EndStopUnlocked is pressed
        stepper.disable();

        // show "open" status
        digitalWrite(ledClosed,LOW);
        digitalWrite(ledOpen,HIGH);
        delay(10000); // wait for opening the window by hand

        // close window
        stepper.enable();
        while(digitalRead(Locked) == LOW){
          // wait for WindowClosed endstop to be pressed and for the key or that the timer has counted 10 minutes
          if((digitalRead(Closed) == HIGH) && (digitalRead(keySwitch) == HIGH)){
            Serial.println("closing");
            stepper.move(100); // locking mechanism
          }
          else {
            Serial.println("waiting to close");
            analogWrite(beep, 10);
            PORTC |= (1 << ledPull);
          }
        }    
        stepper.disable(); // don't stop cycle until window is locked

        // show "closed" status, tell the Pi that the system is done
        if (digitalRead(Locked) == HIGH){
          analogWrite(beep, 0);
          PORTC &= ~(1 << ledPull);
          digitalWrite(ledOpen,LOW);
          digitalWrite(ledClosed,HIGH);
          PORTC &= ~(1 << feedBackPi);
        }
    }

    //tell the Pi that the system is ready
    else {
      Serial.println("window closed\nwaiting for opening the window/for rfid match"); 
      PORTC &= ~(1 << feedBackPi); 
      stepper.disable();
    }
  }
}

