#include <Wire.h>
//#include <ESP8266HTTPClient.h>
//#include <ESP8266WiFi.h>      //Esp8266 라이브러리
//#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

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

void ICACHE_RAM_ATTR updateAngle();

void setup()
{
  Wire.begin(D3,D4); //SDA=D7, SCL=D6
  Serial.begin(115200);

  Serial.println("Starting up L3G4200D");
  setupL3G4200D(250); // Configure L3G4200  - 250, 500 or 2000 deg/sec

  delay(1000); // wait for the sensor to be ready 
  calibrate();
  //attachInterrupt(0, updateAngle, RISING);
  // 인터럽트 핀 번호: 1. D5=GPIO 14 success :2.D10=GPIO 1 X :2.D4=GPIO 2 x : 3.D2=GPIO 4 sucess
  //: 4.D0=GPIO 16 X :5.D1=GPIO 5 sucess :  6.D3=GPIO 0  X  7.D0=GPIO 16 X: 8:D4=GPIO 2 X
  //9:D6=GPIO 12 success 10:D7=GPIO 13 success 11.:D8=GPIO 15 success 12.D9=GPIO 3  X
  // 안되는 핀:D10, D4, D0, D3,D9
  //pinMode(D5,INPUT); //configure switch as INPUT
  attachInterrupt(D5, updateAngle, RISING);
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
    int i;
    
    calc_actual_angle();
    sendJson();
    delay(50);

    //http_toServer(1.01,1.02,1.03); 
}

//void http_toServer(float x,float y, float z){
//  if (WiFi.status() == WL_CONNECTED) {
//    
//    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
//    JsonObject& JSONencoder = JSONbuffer.createObject(); 
// 
//    JSONencoder ["x_gyro"] = x;
//    JSONencoder ["y_gyro"] = y;
//    JSONencoder ["z_gyro"] = z;
//    JSONencoder ["u_id"] = 1;
//    
//    
//    //JsonArray& hum = JSONencoder.createNestedArray("hum"); //JSON array
//    //hum.add(h); //Add value to array
//    
//    //2개이상의 값을 가지는 JSON형식을 정의하기 위한 코드
// 
//    //JsonArray& temp = JSONencoder.createNestedArray("temp"); //JSON array
//    //temp.add(t); //Add value to array
// 
//    char JSONmessageBuffer[300];
//    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
//    Serial.println(JSONmessageBuffer);
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
    sprintf(json, "{\"x\":%d,\"y\":%d,\"z\":%d}", (int)(angX*1000), (int)(angY*1000), (int)(angZ*1000));
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
