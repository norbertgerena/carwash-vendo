#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))    //clear bit in byte at sfr adress
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))     //set bit in byte at sfr adress
#define seconds() (millis()/1000)  //capture secods

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//pins to use
const int coinPin = 2;
const int button1 = 8, button2 = A0, button3 = 4;
const int relay1 = 10, relay2 = 11, relay3 = 12;
const int button1pushed = 0b00110000;
const int button2pushed = 0b00101000;
const int button3pushed = 0b00011000;
//variables used in the Interrupt Service Routine (ISR) need to be 'volatile'
volatile long unsigned eventTime;
volatile long unsigned previousEventTime;
volatile long unsigned timeSinceLastEvent;
volatile byte portDstatus;
volatile byte eventFlag1, eventFlag2, eventFlag3;
//additonal variables
unsigned long timeSpent = 0, startTime = 0;
volatile long unsigned timeLeft = 0;
int coinTimeVal = 5;  //how much time per coin
int event;
bool event1running = false, event2running = false, event3running =false;

void setup(){
  
  Serial.begin(9600);//start serial communication
  
  //define the pins:
  pinMode(button1, INPUT);//define pin as INPUT  
  digitalWrite(button1, HIGH);//connect the internal pull-up resistor - this will yield a HIGH readout if the switch is open.
  pinMode(button2, INPUT);
  digitalWrite(button2, HIGH);
  pinMode(button3, INPUT);
  digitalWrite(button3, HIGH);
  pinMode(relay1, OUTPUT);
  digitalWrite(relay1, HIGH);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay2, HIGH);
  pinMode(relay3, OUTPUT);
  digitalWrite(relay3, HIGH);
  
  //setting up the interrupt: (see chapter 12 in the ATmega328p data sheet for an explanation which register does what etc...)
  cli();
  PCICR |= 0b00000111;    // turn on all ports
  PCMSK0 |= 0b00000001;    // turn on pin PB0, which is PCINT0, pin 8 
  PCMSK1 |= 0b00000001;    // turn on pin PC4, which is PCINT12, pin A0
  PCMSK2 |= 0b00010000;    // turn on pin 4
  sei();
  //lcd init
  lcd.begin(16, 2);

  //coin slot machine
  attachInterrupt(digitalPinToInterrupt(coinPin), coinInterrupt, RISING);
}


void loop()
{
  if (timeLeft <= 0){
    if (timeLeft < 0){timeLeft=0;}
    welcomeInsertCoinMessage();
    if (event1running){
      switchRelay(relay1, event1running);
      event1running ^= event1running;  
    }
    else if (event2running){
      switchRelay(relay2, event2running);
      event2running ^= event2running;  
    }
    else if (event3running){
      switchRelay(relay3, event3running);
      event3running ^= event3running;  
    }
  }
  else {
    printTime(timeLeft);
    if (eventFlag1 == 1 && (eventTime-previousEventTime>100)){
      previousEventTime=eventTime;//note the present event time as last time for the next event
      startTime = seconds();
      switchRelay(relay1, event1running);
      event1running ^= event1running;
      if (event1running){
        printStatus("Wash...");  
      }else{
        printStatus("Washing paused"); 
      }
      eventFlag1 = 0;//reset the event flag for the next event
    }
    else if (eventFlag2 == 1 && (eventTime-previousEventTime>100)){
      previousEventTime=eventTime;//note the present event time as last time for the next event
      startTime = seconds();
      switchRelay(relay2, event2running);
      event2running ^= event2running;
      if (event2running){
        printStatus("Wash...");  
      }else{
        printStatus("Washing paused"); 
      }
      eventFlag2 = 0;//reset the event flag for the next event
    }
    else if (eventFlag3 == 1 && (eventTime-previousEventTime>100)){
      previousEventTime=eventTime;//note the present event time as last time for the next event
      startTime = seconds();
      switchRelay(relay3, event3running);
      event3running ^= event3running;
      if (event3running){
        printStatus("Wash...");  
      }else{
        printStatus("Washing paused"); 
      }
      eventFlag3 = 0;//reset the event flag for the next event
    }
    if (event1running || event2running || event3running){
      timeSpent = seconds() - startTime;
      timeLeft -= timeSpent;
    } else{
      startTime = seconds();
    }
  }
}



ISR (PCINT0_vect){
  eventTime=millis();//note the time of the ISR call  
  eventFlag1=1;//set the event flag that tells the main loop that a button was pressed.
}

ISR (PCINT1_vect){
  eventTime=millis();//note the time of the ISR call  
  eventFlag2=1;//set the event flag that tells the main loop that a button was pressed.
}

ISR (PCINT3_vect){
  eventTime=millis();//note the time of the ISR call  
  eventFlag3=1;//set the event flag that tells the main loop that a button was pressed.
}
void coinInterrupt(){
  timeLeft += coinTimeVal; 
}

void welcomeInsertCoinMessage(){
    lcd.setCursor(4, 0); //Start at character 4 on line 0
    lcd.print("WELCOME!");
    lcd.setCursor(0, 1);
    lcd.print(" ->Insert Coin<-");
    lcd.clear();
  }

void printTime(int timer){
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(timer);
//    delay(100);
//    lcd.setCursor(0, 1);
//    lcd.print(statusAction); 
}

void printStatus(String s){
  lcd.setCursor(0, 1);
  lcd.print(s);
}
void switchRelay(int relaynum, bool onOff){
  digitalWrite(relaynum, onOff);
  }
