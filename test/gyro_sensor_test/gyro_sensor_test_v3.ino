#include <Wire.h>
#include <math.h>
//#include <ESP8266HTTPClient.h>
//#include <ESP8266WiFi.h>      //Esp8266 라이브러리
//#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리
#include <DHT.h>

#define DHTPIN 3
#define DHTTYPE DHT11

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

//*******gyro sensor
// I2C address of the L3G4200D.
// Use I2C scanner to find this value!
int L3G4200D_Address = 0x69;

// Can fine-tune this if you need to
float DPS_MULT = 0.0000085;

// Delta angles (raw input from gyro)
int x = 0;
int y = 0;
int z = 0;

// Actual angles
float angX = 0;
float angY = 0;
float angZ = 0;
float x_ary[20], y_ary[20], z_ary[20];

// Previous angles
float pre_angX = 0;
float pre_angY = 0;
float pre_angZ = 0;

// Differentiate angles
float dif_angX = 0;
float dif_angY = 0;
float dif_angZ = 0;

// Sum of Actual angles
float angX_sum = 0;
float angY_sum = 0;
float angZ_sum = 0;

// Max of Actual angles
float angX_max = 0;
float angY_max = 0;
float angZ_max = 0;

// Previous angles for calculation
float p_angX = 0;
float p_angY = 0;
float p_angZ = 0;

// Calibration values
int gyroLowX = 0;
int gyroLowY = 0;
int gyroLowZ = 0;
int gyroHighX = 0;
int gyroHighY = 0;
int gyroHighZ = 0;

// Used for calculating delta time
unsigned long pastMicros = 0;

//******heart sensor
int BPM[10] = {};
int BPM_SUM = 0;
int BPM_Average = 0;

//******temp DHT11 sensor
float h,t;

//void ICACHE_RAM_ATTR updateAngle();
void updateAngle();

DHT dht(DHTPIN, DHTTYPE); 

void setup()
{

  //dht.begin();
  Wire.begin(); //Wire.begin(D7,D6) SDA=D7, SCL=D6
  Serial.begin(9600);

  Serial.println("Starting up L3G4200D");
  setupL3G4200D(250); // Configure L3G4200  - 250, 500 or 2000 deg/sec

  delay(1000); // wait for the sensor to be ready 
  calibrate();
  //arduino
  attachInterrupt(0, updateAngle, RISING);
  
  //nodeMCU
  // 인터럽트 핀 번호: 1. D5=GPIO 14 success :2.D10=GPIO 1 X :2.D4=GPIO 2 x : 3.D2=GPIO 4 sucess
  //: 4.D0=GPIO 16 X :5.D1=GPIO 5 sucess :  6.D3=GPIO 0  X  7.D0=GPIO 16 X: 8:D4=GPIO 2 X
  //9:D6=GPIO 12 success 10:D7=GPIO 13 success 11.:D8=GPIO 15 success 12.D9=GPIO 3  X
  // 안되는 핀:D10, D4, D0, D3,D9
  //pinMode(D5,INPUT); //configure switch as INPUT
  //attachInterrupt(D5, updateAngle, RISING);
  //attachInterrupt(digitalPinToInterrupt(D5), updateAngle, RISING);
  
  pastMicros = micros();
                        
//  WiFi.begin("MS_IPTIME_2G", "hyun0915");
// 
//  while (WiFi.status() != WL_CONNECTED) {  
// 
//    delay(500);
//    Serial.println("Waiting for connection");
// 
//  }
//  // Print the IP address
//  Serial.println(WiFi.localIP());

}
void loop()
{   
    //temp_sensor();    
    gyro_sensor();


    //http_toServer_temp(h,t); 
    //http_toServer_heart(BPM_Average); 
    //http_toServer_gyro(); 
}

void heart_sensor(){
  BPM_SUM = 0;
  BPM_Average = 0;
  for (int count = 0; count < 10; count++){
    
    Wire.requestFrom(0xA0 >> 1, 1);    //  데이터 요청
    
    while(Wire.available()) {          //  응답 시 
        
        unsigned char heartbeat = Wire.read();   // heartbeat에 보내준 데이터 저장(심박수)

          BPM[count] = heartbeat;
          Serial.print("[");
          Serial.print(BPM[count]);         // 심장박동 수치 출력
          Serial.print("] "); 
          delay(300); 
          BPM_SUM = BPM_SUM + BPM[count] ;  // 평균구하기 위한 합 계산     
      }   
    }
    Serial.println("\n");
    BPM_Average = BPM_SUM / 10;
    Serial.print("BPM Average >> ");
    Serial.println(BPM_Average);
}

void temp_sensor(){
  h = dht.readHumidity();
  t = dht.readTemperature();
  
  if (isnan((int)h) || isnan((int)t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print("temp : ");
  Serial.println(t);
}


void gyro_sensor(){
    int cnt=0;
    int i=0;
    
    for(i=0;i<20;i++){
        cnt++;
        //이전 값 저장
        pre_angX = angX;
        pre_angY = angY;
        pre_angZ = angZ;

        //angX,angY,angZ 계산
        calc_actual_angle();
        x_ary[i]=angX;   
        y_ary[i]=angY;
        z_ary[i]=angZ;

        //이전값과의 차이 계싼
        dif_angX = angX - pre_angX;
        dif_angY = angY - pre_angY;
        dif_angZ = angZ - pre_angZ;

        //angX,angY,angZ 출력
        sendJson();

        //1초동안 측정값 누적 - 20개 data
        angX_sum += angX;
        angY_sum += angY;
        angZ_sum += angZ;

        //1초동안 측정값 최대값 계산 - 20개 data
        if(abs(dif_angX) > angX_max){
          angX_max = abs(dif_angX);
        }
      
        if(abs(dif_angY) > angY_max){
          angY_max = abs(dif_angY);
        }
        if(abs(dif_angZ) > angZ_max){
          angZ_max = abs(dif_angZ);
        }

        if(cnt==20){
          cnt=0;
          char str[50];
          //sprintf(str, "#######{\"x\":%d,\"y\":%d,\"z\":%d}", (int)(angX_max*1000), (int)(angY_max*1000), (int)(angZ_max*1000));
          //Serial.println(str);

          angX_sum=0;
          angY_sum=0;
          angZ_sum=0;

          angX_max=0;
          angY_max=0;
          angZ_max=0;
        }
           
        delay(50);
     }
}

//void http_toServer_gyro(){
//  int i=0;
//  if (WiFi.status() == WL_CONNECTED) {
//    
//    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
//    JsonObject& JSONencoder = JSONbuffer.createObject(); 
// 
////    JSONencoder ["x_gyro"] = x;
////    JSONencoder ["y_gyro"] = y;
////    JSONencoder ["z_gyro"] = z;
////    JSONencoder ["u_id"] = 1;
//    
//    //2개이상의 값을 가지는 JSON형식을 정의하기 위한 코드
//    JsonArray& x_gyro = JSONencoder.createNestedArray("x_gyro"); //JSON array
//    JsonArray& y_gyro = JSONencoder.createNestedArray("y_gyro"); //JSON array
//    JsonArray& z_gyro = JSONencoder.createNestedArray("z_gyro"); //JSON array
//    
//    for(i=0;i<20;i++){
//      x_gyro.add(x_ary[i]); //Add value to array
//      y_gyro.add(y_ary[i]); //Add value to array
//      z_gyro.add(z_ary[i]); //Add value to array
// 
//      char JSONmessageBuffer[300];
//      JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
//      Serial.println(JSONmessageBuffer);
//    }
// 
//    HTTPClient http;    //클래스 정의
// 
//    http.begin("http://192.168.3.4:5000/sensor/gyro/insert");      //Uri 정의
//    http.addHeader("Content-Type", "application/json");  //헤더 정의
// 
//    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
//    String payload = http.getString();                                        
// 
//    Serial.println(httpCode);   //HTTP Status 값
//    Serial.println(payload);    //데이터 값
// 
//    http.end();  //Close connection
// 
//  } else {
//    Serial.println("Error in WiFi connection");
//  }
//}
//
//void http_toServer_temp(float temp,float hum){
//  if (WiFi.status() == WL_CONNECTED) {
//    
//    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
//    JsonObject& JSONencoder = JSONbuffer.createObject(); 
// 
//    JSONencoder ["temp"] = x;
//    JSONencoder ["hum"] = y;
//    JSONencoder ["u_id"] = 1;
// 
//    char JSONmessageBuffer[300];
//    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
//    Serial.println(JSONmessageBuffer);
// 
//    HTTPClient http;    //클래스 정의
// 
//    http.begin("http://192.168.3.4:5000/sensor/temp/insert");      //Uri 정의
//    http.addHeader("Content-Type", "application/json");  //헤더 정의
// 
//    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
//    String payload = http.getString();                                        
// 
//    Serial.println(httpCode);   //HTTP Status 값
//    Serial.println(payload);    //데이터 값
// 
//    http.end();  //Close connection
// 
//  } else {
//    Serial.println("Error in WiFi connection");
//  }
//}

//void http_toServer_heart(int bpm){
//  if (WiFi.status() == WL_CONNECTED) {
//    
//    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
//    JsonObject& JSONencoder = JSONbuffer.createObject(); 
// 
//     JSONencoder ["bpm"] = bpm;    //평균값 Json 변환
//     JSONencoder ["u_id"] = 1;
// 
//    char JSONmessageBuffer[300];
//    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
//    Serial.println(JSONmessageBuffer);
// 
//    HTTPClient http;    //클래스 정의
// 
//    http.begin("http://192.168.3.4:5000/sensor/heart/insert");      //Uri 정의
//    http.addHeader("Content-Type", "application/json");  //헤더 정의
// 
//    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
//    String payload = http.getString();                                        
// 
//    Serial.println(httpCode);   //HTTP Status 값
//    Serial.println(payload);    //데이터 값
// 
//    http.end();  //Close connection
// 
//  } else {
//    Serial.println("Error in WiFi connection");
//  }
//}

void calc_actual_angle(){
  getGyroValues();

  // Calculate delta time
  float dt;
  if(micros() > pastMicros) //micros() overflows every ~70 minutes
    dt = (float) (micros()-pastMicros)/1000000.0;
  else
    dt = (float) ((4294967295-pastMicros)+micros())/1000000.0;


  // Calculate angles
  if(x >= gyroHighX || x <= gyroLowX) {
    angX += ((p_angX + (x * DPS_MULT))/2) * dt;
    p_angX = x * DPS_MULT;
  } else {
    p_angX = 0;
  }

  if(y >= gyroHighY || y <= gyroLowY) {
    angY += ((p_angY + (y * DPS_MULT))/2) * dt;
    p_angY = y * DPS_MULT;
  } else {
    p_angY = 0;
  }

  if(z >= gyroHighZ || z <= gyroLowZ) {
    angZ += ((p_angZ + (z * DPS_MULT))/2) * dt;
    p_angZ = z * DPS_MULT;
  } else {
    p_angZ = 0;
  }  
  pastMicros = micros();
}

void updateAngle()
{
  Serial.println("Interrupt!!");
  getGyroValues();
}

void calibrate()
{
  Serial.println("Calibrating gyro, don't move!");
  for(int i = 0; i < 200; i++) {
    getGyroValues();

    if(x > gyroHighX)
      gyroHighX = x;
    else if(x < gyroLowX)
      gyroLowX = x;

    if(y > gyroHighY)
      gyroHighY = y;
    else if(y < gyroLowY)
      gyroLowY = y;

    if(z > gyroHighZ)
      gyroHighZ = z;
    else if(z < gyroLowZ)
      gyroLowZ = z;
    
    delay(10);
  }
  Serial.println("Calibration complete.");
}

// Print angles to Serial (for use in Processing, for example)
void sendJson() {
    char json[40];
    sprintf(json, "{\"x\":%d,\"y\":%d,\"z\":%d,\"dif_x\":%d,\"dif_y\":%d,\"dif_z\":%d}", 
    (int)(angX*1000), (int)(angY*1000), (int)(angZ*1000),(int)((dif_angX)*1000), (int)((dif_angY)*1000), (int)((dif_angZ)*1000));
    //sprintf(json, "{\"x\":%d,\"y\":%d,\"z\":%d}", (int)((dif_angX)*1000), (int)((dif_angY)*1000), (int)((dif_angZ)*1000));
    Serial.println(json);
}

void getGyroValues() {
  byte xMSB = readRegister(L3G4200D_Address, 0x29);
  byte xLSB = readRegister(L3G4200D_Address, 0x28);
  x = ((xMSB << 8) | xLSB);

  byte yMSB = readRegister(L3G4200D_Address, 0x2B);
  byte yLSB = readRegister(L3G4200D_Address, 0x2A);
  y = ((yMSB << 8) | yLSB);

  byte zMSB = readRegister(L3G4200D_Address, 0x2D);
  byte zLSB = readRegister(L3G4200D_Address, 0x2C);
  z = ((zMSB << 8) | zLSB);
}

int setupL3G4200D(int scale) {
  //From  Jim Lindblom of Sparkfun's code

  // Enable x, y, z and turn off power down:
  writeRegister(L3G4200D_Address, CTRL_REG1, 0b00001111);

  // If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
  writeRegister(L3G4200D_Address, CTRL_REG2, 0b00000000);

  // Configure CTRL_REG3 to generate data ready interrupt on INT2
  // No interrupts used on INT1, if you'd like to configure INT1
  // or INT2 otherwise, consult the datasheet:
  writeRegister(L3G4200D_Address, CTRL_REG3, 0b00001000);

  // CTRL_REG4 controls the full-scale range, among other things:

  if(scale == 250) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00000000);
  } else if(scale == 500) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00010000);
  } else {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00110000);
  }

  // CTRL_REG5 controls high-pass filtering of outputs, use it
  // if you'd like:
  writeRegister(L3G4200D_Address, CTRL_REG5, 0b00000000);
}

void writeRegister(int deviceAddress, byte address, byte val)
{
    Wire.beginTransmission(deviceAddress); // start transmission to device 
    Wire.write(address);       // send register address
    Wire.write(val);         // send value to write
    Wire.endTransmission();     // end transmission
}

int readRegister(int deviceAddress, byte address)
{
    int v;
    Wire.beginTransmission(deviceAddress);
    Wire.write(address); // register to read
    Wire.endTransmission();

    Wire.requestFrom(deviceAddress, 1); // read a byte

    while(!Wire.available()) {
        // waiting
    }
    v = Wire.read();
    return v;
}
