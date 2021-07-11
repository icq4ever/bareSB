#include <Roomba.h>
#include <ArduinoOSC.h>

String deviceName = "RB1";
bool bStatusLedOn = false;
uint64_t sensor[4];
uint64_t battVoltage, battCharge, battCapacity;
int ddPin = 4;
int roombaCommOK = 0;
int irOptCode[1] = {0};
///uint64_t sensorLeft, sensorRight;

//PID setup
uint8_t sensorArray[2] = {0, 0};
int driveAngle = 0;
bool bRunningOn = false;


// WiFi
const char* ssid = "icq4ever.asus";
const char* pwd = "hoon525!";
//const char* ssid = "icq4ever.iptime_2,4G";
//const char* pwd = "2684512345";
const IPAddress ip(192, 168, 0, 10);
const IPAddress gateway(192, 168, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

// Arduino OSC
const char* host = "192.168.0.255";
const int recv_port = 54321;
const int send_port = 55555;
const int publish_port = 54445;

// roomba
Roomba roomba(&Serial2, Roomba::Baud115200);
bool bDone = false;
uint64_t songTimer;
unsigned int errors = 0;

uint8_t bootupSong[] = {62, 12, 66, 12, 69, 12, 74, 36};
uint8_t sleepSong[]  = {74, 12, 69, 12, 66, 12, 62, 36};
uint8_t sensorCliffs[] = {28, 29, 30, 31};

int dockAvailable = 0;
uint8_t sensorIRCodes[] = {17};
uint64_t dockCheckingTimer;

int i;
float f;
String s;

// WDT
// 10 seconds WDT
//#define WDT_TIMEOUT 10/
uint64_t tt;
uint64_t cmdTimer;
uint64_t timerWdtChecked;



void setupOSC() {

#ifdef ESP_PLATFORM
  WiFi.disconnect(true, true); // disable wifi, erase ap info
  delay(2000);
  WiFi.mode(WIFI_STA);
#endif
  // WiFi.begin();
//  WiFi.setHo/stname("ESP-host");
  Serial.println(WiFi.dnsIP());
  // WiFi.begin();
  // while(WiFi.status() != WL_CONNECT_FAILED) { delay(100);}  // 접속실패가 되면
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
    OscWiFi.send(host, send_port, "/" + deviceName + "/sensorValue", sensor[0], sensor[1], sensor[2], sensor[3]);
//    OscWiFi.send(host, send_port, "/voltage", battVoltage);
    OscWiFi.send(host, send_port, "/" + deviceName + "/turn", driveAngle);
    OscWiFi.send(host, send_port, "/" + deviceName + "/charge", battCharge, battCapacity);
    OscWiFi.send(host, send_port, "/" + deviceName + "/optCode", dockAvailable, irOptCode[0]);
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
    if(m.arg<int>(0) == 1)  bRunningOn = true;
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
  delay(1000);
  //  roomba.baud(Roomba::Baud19200);
  //  delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(ddPin, OUTPUT);

  tt = millis();

  setupOSC();

  Serial.println("booted");
  initRoomba();

  songTimer = millis();
}

void loop() {
  if (!bDone && millis() > 5000) {
    playBootupSong();
  }
  OscWiFi.update();
  //  checkWDT();/

  if (millis() - songTimer > 50) {
    //    playSong();
    readCliffs();
    readBatteryCharge();
    readBatteryCapacity();
    checkDockAvailable();


    if(sensor[0] > 1200)  sensorArray[0] = 1;
    else                  sensorArray[0] = 0;

    if(sensor[3] > 1200)  sensorArray[1] = 1;
    else                  sensorArray[1] = 0;

    if      (sensorArray[0] == 0 && sensorArray[1] == 0)  driveAngle = 0;
    else if (sensorArray[0] == 1 && sensorArray[1] == 0)  driveAngle = 1;
    else if (sensorArray[0] == 0 && sensorArray[1] == 1)  driveAngle = -1;

    if(bRunningOn)  {
      if(driveAngle > 0)        roomba.drive(200, 10);
      else if (driveAngle < 0)  roomba.drive(200, -10);
      else                      roomba.drive(200, 0);
    } else {
      roomba.drive(0, 0);
    }

    songTimer = millis();
  }
}

void readBumper() {
  uint8_t buf[10];

  bool ret = roomba.getSensors(7, buf, 1);
  check(ret == 1, "getSensors");
}

void readBatteryVoltage(){
  uint8_t buf[2];

  bool ret = roomba.getSensors(22, buf, 2);
  if(ret){
    battVoltage = 256*buf[0] + buf[1];
  }
}

void readBatteryCharge(){
  uint8_t buf[2];

  bool ret = roomba.getSensors(25, buf, 2);
  if(ret){
    battCharge = 256*buf[0] + buf[1];
  }
}

void checkDockAvailable(){
  uint8_t buf[1];

  bool ret = roomba.getSensorsList(sensorIRCodes, 1, buf, 1);
  if(ret) {
    irOptCode[0] = (int)buf[0];
    if(irOptCode[0] == 161 || irOptCode[0] == 164 || irOptCode[0] == 168) {
       dockCheckingTimer = millis();
    }

    if(millis() - dockCheckingTimer < 700)  dockAvailable = 1;
    else                                    dockAvailable = 0;
    
    Serial.print((int)buf[0]);
    Serial.println();
  } else {
    Serial.println("cannot read sensor dock value");
  }
}

void readBatteryCapacity(){
  uint8_t buf[2];

  bool ret = roomba.getSensors(26, buf, 2);
  if(ret) battCapacity = 256*buf[0] + buf[1];
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
    Serial.println("sensor poll failed");
    roombaCommOK = 0;
  }
}

void wakeUp(){
  digitalWrite(ddPin, HIGH);
  delay(100);
  digitalWrite(ddPin, LOW);
  delay(500);
  digitalWrite(ddPin, HIGH);
  delay(2000);

  roomba.start();
  delay(50);
  roomba.safeMode();
  delay(50);

  playBootupSong();
}

void setSafeMode(){
  roomba.safeMode();
  delay(50);
}

void setPassiveMode(){
  roomba.start();
  delay(50);
}

void setSleepMode(){
  roomba.power();
  delay(50);
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
  if (!t)
    error(m);
}

void error(const char *m) {
  Serial.print("Error: ");
  Serial.println(m);
  errors++;
}
