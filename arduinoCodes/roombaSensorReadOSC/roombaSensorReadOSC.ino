#include <Roomba.h>
#include <ArduinoOSC.h>

String deviceName = "RB2";
bool bStatusLedOn = false;
uint64_t sensor[4];

int ddPin = 4;
int pinLed1 = 36;
int pinLed2 = 37;
int roombaCommOK = 0;
int irOptCode[1] = {0};

//PID setup
uint8_t sensorArray[2] = {0, 0};
int driveAngle = 0;
bool bRunningOn = false;
bool bLastRunningOn = false;

// WiFi
const char* ssid = "icq4ever.asus";
const char* pwd = "hoon525!";
//const char* ssid = "icq4ever.iptime_2,4G";
//const char* pwd = "2684512345";
const IPAddress ip(192, 168, 0, 11);
const IPAddress gateway(192, 168, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

// Arduino OSC
const char* host = "192.168.0.255";
const int recv_port = 54321;
const int send_port = 55555;
const int publish_port = 54445;
uint8_t currentMode = 0;

// roomba
Roomba roomba(&Serial2, Roomba::Baud115200);
bool bDone = false;
uint64_t reportTimer;
unsigned int errors = 0;

uint8_t bootupSong[] = {62, 12, 66, 12, 69, 12, 74, 36};
uint8_t sleepSong[]  = {74, 12, 69, 12, 66, 12, 62, 36};
uint8_t sensorCliffs[] = {28, 29, 30, 31};
uint8_t streamPackitIDs[] = {35, 17, 21, 25, 26, 28, 29, 30, 31};

int dockAvailable = 0;
uint64_t dockCheckingTimer;

int i;
float f;
String s;

uint64_t cliffsensorsValue[4];
uint8_t currentOIMode;
uint64_t battVoltage, battCharge, battCapacity;
uint8_t chargeState;
uint8_t omniSensorIRCode;

// WDT
// 10 seconds WDT
//#define WDT_TIMEOUT 10/
uint64_t tt;
uint64_t cmdTimer;
uint64_t timerWdtChecked;

// PID Controller
float Kp = 25;
float Ki = 0.15;
float Kd = 1000;
float errorValue = 0;
float P = 0;
float I = 0;
float D = 0;
float PIDValue = 0;
float pErrorValue = 0;
int turnValue = 0;

void drunkenTurn() {
  if (sensor[0] > 500)  sensorArray[0] = 1;
  else                  sensorArray[0] = 0;

  if (sensor[3] > 500)  sensorArray[1] = 1;
  else                  sensorArray[1] = 0;

  if      (sensorArray[0] == 0 && sensorArray[1] == 0)  driveAngle = 0;
  else if (sensorArray[0] == 1 && sensorArray[1] == 0)  driveAngle = 1;
  else if (sensorArray[0] == 0 && sensorArray[1] == 1)  driveAngle = -1;

  if (bRunningOn)  {
    if (driveAngle > 0)       roomba.drive(200, 10);
    else if (driveAngle < 0)  roomba.drive(200, -10);
    else                      roomba.drive(200, 0);
  } else {
    roomba.drive(0, 0);
  }
}

void directTurn(){
  if(sensor[0] > 500 || sensor[3] > 500){
//    errorValue = map(sensor[0]-sensor[3], -500, 500, -10, 10);
//    P = errorValue;
//    I = I + errorValue;;
//    D = errorValue - pErrorValue;
//    PIDValue = (Kp * P) + (Ki * I) + (Kd * D);
//    pErrorValue = errorValue;
  
//    turnValue = constrain(map(PIDValue, -1500, 1500, -80, 80), -80, 80);
    turnValue = constrain(map(sensor[0] - sensor[3], -800, 800, -80, 80), -80, 80);
    roomba.driveDirect(150 - turnValue, 150 + turnValue);
  } else {
    roomba.driveDirect(150, 150);
  }
}

void pidTurn(){
  if(sensor[0] > 500 || sensor[3] > 500){
  errorValue = map(sensor[0]-sensor[3], -500, 500, -10, 10);
  P = errorValue;
  I = I + errorValue;;
  D = errorValue - pErrorValue;
  PIDValue = (Kp * P) + (Ki * I) + (Kd * D);
  pErrorValue = errorValue;

  turnValue = map(PIDValue, -1500, 1500, -5, 5);
  roomba.drive(200, turnValue);
  } else {
    roomba.drive(200, 0);
  }
}

void checkOIMode() {
  uint8_t buf[1];
  bool ret = roomba.getSensors(35, buf, 1);
  currentMode = (unsigned int)buf[0];
  if(!ret)  Serial.println("check OIMode failed!");
  else      Serial.print("OIMode : ");
  Serial.println(currentMode);
  OscWiFi.send(host, send_port, "/" + deviceName + "/currentMode", (unsigned int)buf[0]);
}

void readBatteryVoltage() {
  uint8_t buf[2];

  bool ret = roomba.getSensors(Roomba::SensorVoltage, buf, 2);
  if (ret) {
    battVoltage = 256 * buf[1] + buf[0];
  }
}

void readBatteryCharge() {
  uint8_t buf[2];

  bool ret = roomba.getSensors(Roomba::SensorBatteryCharge, buf, 2);
  if (ret) {
    //    battCharge = 256 * buf[1] + buf[0];
    battCharge = uint16_t(buf[0]) << 8 | uint16_t(buf[1]);
  }
}

void readBatteryCapacity() {
  uint8_t buf[2];

  bool ret = roomba.getSensors(Roomba::SensorBatteryCapacity, buf, 2);
  if (ret) {
    //    battCapacity = 256 * buf[1] + buf[0];
    battCapacity = uint16_t(buf[0]) << 8 | uint16_t(buf[1]);
  }
}

void readChargingState() {
  uint8_t buf[1];
  bool ret = roomba.getSensors(21, buf, 1);
  if (ret) {
    OscWiFi.send(host, send_port, "/" + deviceName + "/chargingState", (int)buf[0]);
  }
}

void checkDockAvailable() {
  uint8_t buf[1];
  bool ret = roomba.getSensors(17, buf, 1);
  if (ret) {
    irOptCode[0] = (int)buf[0];
    if (irOptCode[0] == 161 || irOptCode[0] == 164 || irOptCode[0] == 168) {
      dockCheckingTimer = millis();
    }

    if (millis() - dockCheckingTimer < 700)  dockAvailable = 1;
    else                                     dockAvailable = 0;

    //    Serial.print((int)buf[0]);
    //    Serial.println();
  } else {
    //    Serial.println("cannot read sensor dock value");
  }
}



void readCliffs() {
  uint8_t buf[8];
  bool ret = roomba.getSensorsList(sensorCliffs, 4, buf, 8);
  if (ret == true) {
    sensor[0] = 256 * buf[0] + buf[1];
    sensor[1] = 256 * buf[2] + buf[3];
    sensor[2] = 256 * buf[4] + buf[5];
    sensor[3] = 256 * buf[6] + buf[7];
    roombaCommOK = 1;
  } else {
    //    Serial.println("sensor poll failed");
    roombaCommOK = 0;
  }
}

void wakeUp() {
  digitalWrite(ddPin, HIGH);
  delay(100);
  digitalWrite(ddPin, LOW);
  delay(500);
  digitalWrite(ddPin, HIGH);
  delay(1000);

  roomba.start();
  delay(50);
  roomba.safeMode();
  delay(50);

  playBootupSong();
  OscWiFi.send(host, send_port, "/" + deviceName + "/report", "wakeup");
}

void pollingSensors() {
  uint8_t buf[15];

  bool ret = roomba.pollSensors(buf, 15);

  //  uint8_t streamPackitIDs[] = {35, 17, 21, 25, 26, 28, 29, 30, 31};
  //  unsigned int cliffsensorsValue[4];
  //  unsigned int currentOIMode;
  //  unsigned int battCharge, battCapacity;
  //  unsigned int chargeState;
  //  unsigned int omniSensorIORCode;

  currentOIMode = buf[0];
  omniSensorIRCode = buf[1];
  chargeState = buf[2];
  battCharge = 256 * buf[4] + buf[3];
  battCapacity = 256 * buf[6] + buf[5];
  chargeState = buf[7];
  cliffsensorsValue[0] = 256 * buf[9] + buf[8];
  cliffsensorsValue[1] = 256 * buf[11] + buf[10];
  cliffsensorsValue[2] = 256 * buf[13] + buf[12];
  cliffsensorsValue[3] = 256 * buf[15] + buf[14];
}

void setSafeMode() {
  roomba.safeMode();
  delay(50);
  OscWiFi.send(host, send_port, "/" + deviceName + "/report", "safemode");
}

void setPassiveMode() {
  roomba.start();
  delay(50);
  OscWiFi.send(host, send_port, "/" + deviceName + "/report", "passivemode");
}

void setSleepMode() {
  roomba.power();
  delay(50);
  OscWiFi.send(host, send_port, "/" + deviceName + "/report", "sleepmode");
}

void dock() {
  // send init Roomba
  roomba.coverAndDock();
  delay(50);

  Serial.println("dock Roomba Sent!");
  bDone = true;
}



void playBootupSong() {
  roomba.song(0, bootupSong, sizeof(bootupSong));
  roomba.playSong(0);
  Serial.println("song played");
  bDone = true;
}

void playSleepSong() {
  roomba.song(0, sleepSong, sizeof(sleepSong));
  roomba.playSong(0);
  Serial.println("song played");
  bDone = true;
}

void clean() {
  roomba.cover();
  delay(50);
  Serial.println("clean Roomba Sent!");
}

void check(boolean t, const char *m) {
  if (!t) error(m);
}

void error(const char *m) {
  Serial.print("Error: ");
  Serial.println(m);
  errors++;
}

void setupOSC() {

#ifdef ESP_PLATFORM
  WiFi.disconnect(true, true); // disable wifi, erase ap info
  delay(2000);
  WiFi.mode(WIFI_STA);
#endif

  WiFi.begin(ssid, pwd);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.print("WiFi connected, IP = "); Serial.println(WiFi.localIP());

  // subscribe osc message
  OscWiFi.subscribe(recv_port, "/" + deviceName + "/ping", [](const OscMessage & m) {
    bStatusLedOn = !bStatusLedOn;
    OscWiFi.send(host, send_port, "/" + deviceName + "/reply", roombaCommOK);
  });

  OscWiFi.subscribe(recv_port, "/" + deviceName + "/requestSensor", [](const OscMessage & m) {
    if (currentMode != 0 || currentMode != 63) {
      OscWiFi.send(host, send_port, "/" + deviceName + "/sensorValue", sensor[0], sensor[1], sensor[2], sensor[3]);
      OscWiFi.send(host, send_port, "/" + deviceName + "/turn", driveAngle);
      OscWiFi.send(host, send_port, "/" + deviceName + "/charge", battCharge, battCapacity);
      OscWiFi.send(host, send_port, "/" + deviceName + "/optCode", dockAvailable, irOptCode[0]);
      OscWiFi.send(host, send_port, "/" + deviceName + "/pidValue", PIDValue, turnValue);
    }
  });

  OscWiFi.subscribe(recv_port, "/" + deviceName + "/wakeup", [](const OscMessage & m) {
    wakeUp();
  });

  OscWiFi.subscribe(recv_port, "/" + deviceName + "/safeMode", [](const OscMessage & m) {
    setSafeMode();
  });

  OscWiFi.subscribe(recv_port, "/" + deviceName + "/passiveMode", [](const OscMessage & m) {
    setPassiveMode();
  });

  OscWiFi.subscribe(recv_port, "/" + deviceName + "/sleepMode", [](const OscMessage & m) {
    playSleepSong();
    delay(1000);
    setSleepMode();
  });

  OscWiFi.subscribe(recv_port, "/" + deviceName + "/setRunning", [](const OscMessage & m) {
    if (m.arg<int>(0) == 1)  bRunningOn = true;
    else                    bRunningOn = false;
  });

  OscWiFi.subscribe(recv_port, "/" + deviceName + "/docking", [](const OscMessage & m) {
    roomba.coverAndDock();
    delay(50);
  });
}

void initRoomba() {
  roomba.start();
  delay(50);
  roomba.safeMode();
  delay(50);

  Serial.println("roomba inited");
  playBootupSong();
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(ddPin, OUTPUT);

  tt = millis();

  setupOSC();

  Serial.println("booted");
  initRoomba();
  //  roomba.stream(streamPackitIDs, 9);
  roomba.streamCommand(Roomba::StreamCommandPause);

  reportTimer = millis();
}

void loop() {
  OscWiFi.update();

  if (millis() - reportTimer > 100) {
    checkOIMode();
    if (currentMode != 0 || currentMode != 63) {
      readCliffs();
      readBatteryCharge();
      readBatteryCapacity();
      checkDockAvailable();

      if (bRunningOn)  {
        directTurn();
//        pidTurn();
//      drunkenTurn();
      }
    }
    reportTimer = millis();
  }
  

  if(bLastRunningOn != bRunningOn && bRunningOn == false){
    roomba.driveDirect(0, 0);
  }
  bLastRunningOn = bRunningOn;
}
