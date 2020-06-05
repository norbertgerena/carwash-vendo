#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))    //clear bit in byte at sfr adress
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))     //set bit in byte at sfr adress

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//pins to use
const int coinPin = 2;
const int button1 = 3, button2 = 4, button3 = 5;
const int relay1 = 10, relay2 = 11, relay3 = 12;

//variables used in the Interrupt Service Routine (ISR) need to be 'volatile'
volatile long unsigned eventTime;
volatile long unsigned previousEventTime;
volatile long unsigned timeSinceLastEvent;
volatile byte portDstatus;
volatile byte eventFlag;

void setup(){
  
  //Serial.begin(9600);//start serial communication
  
  //define the pins:
  pinMode(button1, INPUT);//define pin as INPUT  
  digitalWrite(button1, HIGH);//connect the internal pull-up resistor - this will yield a HIGH readout if the switch is open.
  pinMode(button2, INPUT);
  digitalWrite(button2, HIGH);
  pinMode(button3, INPUT);
  digitalWrite(button3, HIGH);
 
  //setting up the interrupt: (see chapter 12 in the ATmega328p data sheet for an explanation which register does what etc...)
  sbi (PCICR,PCIE2);//enable interrupt PCI2
  sbi (PCMSK2,PCINT21); //button3
  sbi (PCMSK2,PCINT20); //button2
  sbi (PCMSK2,PCINT19); //button1

  //lcd init
  lcd.begin(16, 2);

  //coin slot machine
  attachInterrupt(digitalPinToInterrupt(coinPin), coinInterrupt, RISING);
}


void loop()
{
  
  if (eventFlag == 1 && (eventTime-previousEventTime>100)){//button event causes eventFlag to be set in ISR. We only consider events occurring 100ms after the last one.
                                                           //This debounces the button.
    timeSinceLastEvent = eventTime - previousEventTime;//calculate the time since the last event
    previousEventTime=eventTime;//note the present event time as last time for the next event.
    Serial.print(timeSinceLastEvent);//print the elapsed time (in ms) on the serial monitor
    Serial.println(" ms");
    Serial.println(portDstatus,BIN);//print the state of Port D as a BINary number
                                    //in a 'real' program you would evaluate portDstatus and find out which button was pressed.
                                    //This could be done in a 'case' strcuture or with if....then
    eventFlag = 0;//reset the event flag for the next event
  }
}



ISR (PCINT2_vect)//Interrupt Service Routine. This code is executed whenever a button event (either pressing or releasing) occurs.
{
  
  portDstatus=PIND;//PIND is the register that contains the momentary status of the pins of Port D (pins 0-7)
                   
  eventTime=millis();//note the time of the ISR call  
  eventFlag=1;//set the event flag that tells the main loop that a button was pressed.
}

void coinInterrupt(){
  
}
