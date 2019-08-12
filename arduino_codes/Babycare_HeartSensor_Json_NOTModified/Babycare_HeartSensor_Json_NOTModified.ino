
/*  Getting_BPM_to_Monitor prints the BPM to the Serial Monitor, using the least lines of code and PulseSensor Library.
 *  Tutorial Webpage: https://pulsesensor.com/pages/getting-advanced
 *
--------Use This Sketch To------------------------------------------
1) Displays user's live and changing BPM, Beats Per Minute, in Arduino's native Serial Monitor.
2) Print: "♥  A HeartBeat Happened !" when a beat is detected, live.
2) Learn about using a PulseSensor Library "Object".
4) Blinks LED on PIN 13 with user's Heartbeat.
--------------------------------------------------------------------*/




#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   

//  Variables
const int PulseWire = 0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 


////데이터값 저장 변수////
int pulse_data[10];
int count; 
int pulse_sum;
int pulse_average;

                               
PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"


void setup() {   

  Serial.begin(9600);          // For Serial Monitor

  // Configure the PulseSensor object, by assigning our variables to it. 
  
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
}



void loop() {

   // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
  //자동출력 함수
  for (int count = 0; count < 10; count++){
   
    int myBPM = 0;
    if (myBPM = pulseSensor.getBeatsPerMinute(0)){
     if (pulseSensor.isInsideBeat()) {            // Constantly test to see if "a beat happened". 
       Serial.print("심장박동 감지 >> "); // If test is "true", print a message "a heartbeat happened".
       Serial.print("BPM: ");                        // Print phrase "BPM: " 
       Serial.println(myBPM);
  
      }
      else if (myBPM = pulseSensor.getBeatsPerMinute(0)) {            // Constantly test to see if "a beat happened". 
       Serial.print("감지되지 않음 >>"); // If test is "true", print a message "a heartbeat happened".
       Serial.print("BPM: ");// Print phrase "BPM: " 
       myBPM = 0;
       Serial.println(myBPM);
  
      }
      // Print the value inside of myBPM.  
    pulse_data[count] = myBPM;
    delay(100);
   
    }
    
  }
/////////** 심장박동 데이터 10개의 배열 안에 저장 **///////////  
  for (count = 0; count < 10; count ++ ){
  //Serial.print("Pulse_data to convert Average >>");
  Serial.print("[");
  Serial.print(pulse_data[count]);
  Serial.print("] ");
  delay(100);
  }
  Serial.print("\n");
////////////////////////////////////////////////////////  

//////////////** 심장박동 데이터 평균값 **//////////////////
for (count = 0; count < 10; count++){
  pulse_sum += pulse_data[count];
}
pulse_average = pulse_sum / 10; 
Serial.print("심장박동 평균치 >> ");
Serial.println(pulse_average);
pulse_average = 0;
pulse_sum = 0;
delay(3000);  
/* 
  delay(400);
  pulse_data[10] = {0};
  for(int i = 0; i < 10; i++){
  Serial.print("[");
  Serial.print(pulse_data[i]);
  Serial.print("] ");
  delay(100);
  }
  Serial.print("\n");
  //Pulse_data += myBPM;


*/
}

  
