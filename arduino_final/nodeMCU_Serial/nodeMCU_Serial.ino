#include <math.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>      //Esp8266 라이브러리
#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리
#include <SoftwareSerial.h>
#include <string.h>

 // Gyro Sensor : RX,TX - D1,D2
 // Heart Sensor : RX,TX - D5,D6 
SoftwareSerial gyroSerial(D1, D2);
SoftwareSerial heartSerial(D5, D6);

int transfer_count = 0;
char host_ip_address[] = {"http://192.168.3.7:5000"};

void setup()
{
  //Serial 통신 Setting
  Serial.begin(9600); //PC - NodeMCU(my)
  gyroSerial.begin(9600); //NodeMCU(my) - Arduino(gyro)
  heartSerial.begin(9600); //NodeMCU(my) - Arduino(heart)

  //Wifi 통신 Setting
  WiFi.begin("MS_IPTIME_2G", "hyun0915");
  //WiFi.begin("aiot_project", "aiotproject");
  //WiFi.begin("KAACT Class 1-1 5G", "kaact4091");
  
  while (WiFi.status() != WL_CONNECTED) {  
 
    delay(500);
    Serial.println("Waiting for connection");
 
  }
  // Print the IP address
  Serial.println(WiFi.localIP());

}

void loop(){
  delay(2000);
  if (gyroSerial.available()) {
    http_toServer_gyro();
  }
  
  if (heartSerial.available()) {
    http_toServer_heart();
  }
}

void http_toServer_gyro(){
    int i;
    
    //gryoSerial에서 자이로센서값 읽어오기
    float angx_max, angy_max, angz_max;
    float x_ary[20], y_ary[20], z_ary[20];
    String json;
    char json_ch[500];

    for(i=0;i<20;i++){
      x_ary[i] = gyroSerial.read();
      y_ary[i] = gyroSerial.read();
      z_ary[i] = gyroSerial.read();
    }
    angx_max = gyroSerial.read();
    angy_max = gyroSerial.read();
    angz_max = gyroSerial.read();

      
    json = gyroSerial.readStringUntil('}');
    json.toCharArray(json_ch, sizeof(json_ch));
    
    //Serial.print("Gyro : ");
    //Serial.println(json);

    //읽어온 센서값 json형식 변환
    //DynamicJsonBuffer jsonBuffer;
    //JsonObject& JSONencoder = jsonBuffer.parseObject(json);

    //Json형식으로 묶어서 NodeMCU 전송(mySerial 출력) : dif_angX -> dif_angY -> dif_angZ
    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
    JsonObject& JSONencoder = JSONbuffer.createObject(); 

    JSONencoder ["x_gyro_max"] = angx_max;
    JSONencoder ["y_gyro_max"] = angy_max;
    JSONencoder ["z_gyro_max"] = angz_max;
    JSONencoder ["u_id"] = 138;

    //Json array 형식으로 key 생성 "x_gyro", "y_gryo", "z_gyro"
    JsonArray& x_gyro = JSONencoder.createNestedArray("x_gyro"); //JSON array
    JsonArray& y_gyro = JSONencoder.createNestedArray("y_gyro"); //JSON array
    JsonArray& z_gyro = JSONencoder.createNestedArray("z_gyro"); //JSON array

    //20개 값 묶기
    for(i=0;i<20;i++){
      x_gyro.add(x_ary[i]); //Add value to array
      y_gyro.add(y_ary[i]); //Add value to array
      z_gyro.add(z_ary[i]); //Add value to array
    }

    char JSONmessageBuffer[500];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println("#########Gyro");
    Serial.println(JSONmessageBuffer);

    //HTTP 통신으로 센서값 서버 전송 : NodeMCU ==> Server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;    //클래스 정의
      
      //char sever_api[] = {"/sensor/gyro/insert"};
      //strcat(host_ip_address,sever_api);

      //http.begin(host_ip_address);      //Uri 정의
      http.begin("http://192.168.3.7:5000/sensor/gyro/insert");
      http.addHeader("Content-Type", "application/json");  //헤더 정의
      
      int httpCode = http.POST(JSONmessageBuffer);   //Send the request
      //int httpCode = http.POST(json);   //일단 먼저 해보기!!
      String payload = http.getString();                                        
      
      Serial.println(httpCode);   //HTTP Status 값
      Serial.println(payload);    //데이터 값
      
      http.end();  //Close connection
    }
    else{
      Serial.println("Error in WiFi connection");
    }
}

void http_toServer_heart(){
  if (WiFi.status() == WL_CONNECTED) {
    int BPM;
    BPM = heartSerial.read();
    //Serial.print(BPM);

    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
    JsonObject& JSONencoder = JSONbuffer.createObject(); 
    char JSONmessageBuffer[300];
    
    JSONencoder ["bpm"] = BPM;    //평균값 Json 변환
    JSONencoder ["u_id"] = 138; 
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    
    transfer_count++;
    Serial.print("NodeMcu : Attempting to receive Json file ");
    Serial.println(transfer_count);
    Serial.println("...");
    Serial.println("...");
    Serial.println("NodeMcu : BPM Request Complete ");
    Serial.println(JSONmessageBuffer);

    Serial.println("");
    
    HTTPClient http;    //클래스 정의

    //char sever_api[] = {"/sensor/heart/insert"};
    //strcat(host_ip_address,sever_api);

    http.begin("http://192.168.3.7:5000/sensor/heart/insert");      //Uri 정의
    http.addHeader("Content-Type", "application/json");  //헤더 정의
    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
    String payload = http.getString();    
    Serial.println(httpCode);   //HTTP Status 값
    Serial.println(payload);    //데이터 값      
    http.end();  //Close connection
  }
  
  else{
    Serial.println("Error in WiFi connection");
  }
}
