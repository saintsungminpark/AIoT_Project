#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 6); // RX, TX
// 연결 5v 3.3v 둘다 됨. i2c
// scl sda vcc gnd
// A5  A4  5v  gnd

int transfer_count = 0;

#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리
StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
JsonObject& JSONencoder = JSONbuffer.createObject(); 
char JSONmessageBuffer[300];


int BPM[10] = {};
int BPM_SUM = 0;
int BPM_Average = 0;

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600); //NodeMCU(your) - NodeMCU(my)
    Serial.println("heart rate sensor:");
    Wire.begin();
}

void loop() {

   for (int count = 0; count < 10; count++){
    
    Wire.requestFrom(0xA0 >> 1, 1);    //  데이터 요청
    
    while(Wire.available()) {          //  응답 시 
        
        unsigned char heartbeat = Wire.read();   // heartbeat에 보내준 데이터 저장(심박수)

          BPM[count] = heartbeat;
          Serial.print (BPM[count]);
          Serial.print(" ");
          //mySerial.write(BPM[count);         // 심장박동 수치 출력
          
          //mySerial.print(x);
          //mySerial.print(y);
          //mySerial.print(z);
          
          delay(100); 
          BPM_SUM = BPM_SUM + BPM[count] ;  // 평균구하기 위한 합 계산

      }
        
    }
     
     
//     BPM arr size : sizeof(BPM)/ sizeof(int)
       BPM_Average = BPM_SUM / (sizeof(BPM)/ sizeof(int));
       transfer_count++;
       Serial.println(" ");
       Serial.print("Uno : BPM Data transfer success ");
       Serial.println(transfer_count);
       Serial.print("Uno : BPM Average is ");
       Serial.println(BPM_Average); // 값 print출력
       mySerial.write(BPM_Average);
       Serial.println("\n");
      BPM_SUM = 0;
      BPM_Average = 0;
      
    delay(1000); 
}
