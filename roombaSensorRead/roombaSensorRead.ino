#include <Roomba.h>
unsigned long timer_br;

Roomba roomba(&Serial2, Roomba::Baud115200);
bool bDone = false;
uint64_t songTimer;
unsigned int errors = 0;

uint8_t song[] = {62, 12, 66, 12, 69, 12, 74, 36};
uint8_t sensorCliffs[] = {28, 29, 30, 31};
    
void setup() {
  Serial.begin(115200);

  Serial.println("booted");
  initRoomba();


  songTimer = millis();
}

void initRoomba() {
  roomba.start();
  delay(50);
//  roomba.safeMode();
  delay(50);

  Serial.println("roomba inited");
  delay(1000);
//  roomba.baud(Roomba::Baud19200);
//  delay(1000);
}

void loop() {
//  if(!bDone && millis() > 5000){
//    playSong();
//  }

  if(millis() - songTimer > 50){
//    playSong();
    readCliffs();
//    readBumper();
    songTimer = millis();
  }
}

void readBumper(){
  uint8_t buf[10];
  
  bool ret = roomba.getSensors(7, buf, 1);
  check(ret == 1, "getSensors");
}

void readCliff(){
  uint8_t buf[2];
//  bool ret = roomba.getSensorsList(sensorsCliff, buf, 4);
//  Serial.println(Roomba::SensoCliffLeftSignal);
  bool ret = roomba.getSensors(Roomba::SensoCliffLeftSignal, buf, 2);
  if(ret == true){
    Serial.print(256*buf[0] + buf[1]);
    Serial.print(" : ");
    for(int i=0; i<2; i++){
      Serial.print(buf[i]);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("sensor poll failed");
  }
//  delay(10);
}

void readCliffs(){
  uint8_t buf[8];
//  bool ret = roomba.getSensorsList(sensorsCliff, buf, 4);
//  Serial.println(Roomba::SensoCliffLeftSignal);
  bool ret = roomba.getSensorsList(sensorCliffs, 4, buf, 8);
  if(ret == true){
//    Serial.print(256*buf[0] + buf[1]);
//    Serial.print(" : ");
//    Serial.print(256*buf[2] + buf[3]);
//    Serial.print(" : ");
//    Serial.print(256*buf[4] + buf[5]);
//    Serial.print(" : ");
    Serial.print(256*buf[6] + buf[7]);
    Serial.println();
  } else {
    Serial.println("sensor poll failed");
  }
//  delay(10);
}

void dock(){
  // send init Roomba 
  roomba.coverAndDock();
  delay(50);

  Serial.println("dock Roomba Sent!");
  bDone = true;
}

void playSong(){
  roomba.song(0, song, sizeof(song));
  roomba.playSong(0);
  Serial.println("song played");
  bDone = true;
}

void clean(){
  roomba.cover();
  delay(50);

  Serial.println("clean Roomba Sent!");
}

void check(boolean t, const char *m){
  if (!t)
    error(m);
}

void error(const char *m){
  Serial.print("Error: ");
  Serial.println(m);
  errors++;
}
  
