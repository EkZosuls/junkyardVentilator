/* Reclaimed item - low cost ventilator sensor test code
 *  Aleks Zosuls. Boston University circuts class, 2020.
 *  **************Disclaimer*****************
 *  This software is incomplete, untested and 
 *  provided as an example only. The author is not 
 *  able to determine the suitabilty of this code for any application.
 *  DO NOT USE THIS CODE FOR ANY LIFE SUPPORT OR MEDICAL APPLICATIONS!!!
 */

#define WINDOW_LENGTH 600
//This is the number of points for the DC average filter
#define TIDAL_VOLUME 1200
//this is the accumulator magnitude calibrated to one breath

const int LED = 13; 
const int RELAY = 7; //pin connected to the relay that switched from 
                //inhale to exhale
const int MAF = 0; //pin connected to air flow sensor analog
const int integratorMax = 100; //max number of integrations before a 
          //reset. Do this in case of error.
int incomingByte = 0; // for incoming serial data
int breathing = 0; //flag to control breath cycle
int window[WINDOW_LENGTH] = {0};  //holds values to be averaged
int windowIndex = 0;  //where we are in the circular average buffer
long windowSum;   // sum of the values to be averaged
long initialCon = 0;  //this is our "zero" for the integrator reset
int i;      //iterator for a loop
bool inhale = true; //state of the system, inhale or exhale
long accumulator = 0; //integrator memory
int ADCRaw;         //analog to digital converter value
int integrations = 0; //number of integration steps or samples

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600); //intantiate the serial port to talk to computer
pinMode(RELAY,OUTPUT);  //set up the output ports
pinMode(LED,OUTPUT);
}

void loop() {

  if (Serial.available() > 0) {
    //*****************Check for user input************
    // read the incoming byte:
    incomingByte = Serial.read();
  }

  switch (incomingByte) {

      case('b'):  //start the inhale cycle
        breathing = 1;
        digitalWrite(RELAY,HIGH); //turn on forced inhale pump
        digitalWrite(LED,HIGH);
        break;
      case('s'):  //stop the ventilation
        breathing = 0;
        digitalWrite(RELAY, LOW); //turn off pump, dump air
        digitalWrite(LED, LOW);
        break;
      default:
        break;
    }

   if(breathing){  // if ventilator is on...
      window[windowIndex] = ADCRaw; //read the current MAF into averager
      windowIndex++;  //iterate the DC average index
      windowIndex = windowIndex % WINDOW_LENGTH; //mod math wrap
      integrations++;
      if (integrations > integratorMax){ /*if we integrated for more time 
        than expected then reset integrator and compute new zero value
        we do this if our algorithm got into trouble. This is likely to 
        happen at the startup and if there is a cough or loss of pressure*/
        accumulator = 0; 
        windowSum = 0;
        //calculate the long term moving average to find the 'zero'
        for(i = 0; i < WINDOW_LENGTH; i++) {
          windowSum = windowSum + window[i];
        }
        initialCon = windowSum / WINDOW_LENGTH; //this is the new starting point of the integrator
        digitalWrite(RELAY, LOW);
        digitalWrite(LED, LOW);
        Serial.println("resetting integrator");
        integrations = 0;
      }
      //read the ADC to feed the integrator
     ADCRaw = analogRead(MAF);
     Serial.println(ADCRaw - initialCon);
     if(inhale) {
       digitalWrite(RELAY,HIGH);
       digitalWrite(LED,HIGH);
     }
     else {
      digitalWrite(RELAY, LOW);
      digitalWrite(LED, LOW);
     }
     //this is the integration. Abs make it work + and -
     accumulator = accumulator + abs(ADCRaw - initialCon);
     Serial.println(accumulator);
     //if we integrate to the desired volume
     if (abs(accumulator) > TIDAL_VOLUME) {
       inhale = !inhale;  //change direction state
       accumulator = 0;
       integrations = 0;
       windowSum = 0;
       //update the zero and reset integrator 
       for(i = 0; i < WINDOW_LENGTH; i++) {
         windowSum = windowSum + window[i];
       }
       initialCon = windowSum / WINDOW_LENGTH;
       Serial.println("---------------flip switch----------------");
      }
    
    
   } //end of if( breathing)
   
   else {  // if not ventilating then find average signal
      digitalWrite(RELAY, LOW);
      digitalWrite(LED, LOW);
      inhale = true;  //always set the inhale to be one when loafing
      accumulator = 0;
      ADCRaw = analogRead(MAF);
      window[windowIndex] = ADCRaw;
      windowIndex++;
      windowIndex = windowIndex % WINDOW_LENGTH;
      windowSum = 0;
      for(i = 0; i < WINDOW_LENGTH; i++) {
        windowSum = windowSum + window[i];
      }
      initialCon = windowSum / WINDOW_LENGTH;
      Serial.println(initialCon);  
      
   }  //end of else not ventilating
    
  delay(50);  //very important delay. This approximates the sample rate
 /*  **************Disclaimer*****************
 *  This software is incomplete, untested and 
 *  provided as an example only. The author is not 
 *  able to determine the suitabilty of this code for any application.
 *  DO NOT USE THIS CODE FOR ANY LIFE SUPPORT OR MEDICAL APPLICATIONS!!!
 */

}  //end of loop()
