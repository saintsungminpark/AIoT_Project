#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 6); // RX, TX
// 연결 5v 3.3v 둘다 됨. i2c
// scl sda vcc gnd
// A5  A4  5v  gnd


/*
#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리
StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
JsonObject& JSONencoder = JSONbuffer.createObject(); 
char JSONmessageBuffer[300];
*/

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
          mySerial.print(BPM[count]);         // 심장박동 수치 출력
          
          //mySerial.print(x);
          //mySerial.print(y);
          //mySerial.print(z);
          Serial.println("mySerial"); 
          delay(300); 
          BPM_SUM = BPM_SUM + BPM[count] ;  // 평균구하기 위한 합 계산

      }
        
    }
     Serial.println("send to mySerial done\n");
//     BPM arr size : sizeof(BPM)/ sizeof(int)
//     BPM_Average = BPM_SUM / (sizeof(BPM)/ sizeof(int));
//     Serial.print("BPM Average >> ");
//     Serial.println(BPM_Average);
//
//     JSONencoder ["bpm"] = BPM_Average;    //평균값 Json 변환
//     JSONencoder ["u_id"] = 1;
//     
//     JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
//     Serial.println(JSONmessageBuffer); // 값 print출력

     //우노에서만 작동하므로 와이파이 센서 부착 후 실험해야 함
      BPM_SUM = 0;
      BPM_Average = 0;
      
    delay(1000);
}
