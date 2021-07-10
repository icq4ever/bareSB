#include <Roomba.h>

const float rf69_FREQ   = 433.0;
unsigned long timer_br;

Roomba roomba(&Serial2, Roomba::Baud115200);
bool bDone = false;

void setup() {
  Serial.begin(115200);

  Serial.println("booted");
  initRoomba();

  if(millis()>5000) {
    clean();
  }
}

void initRoomba() {
  roomba.start();
  delay(50);
  roomba.safeMode();
  delay(50);

  Serial.println("roomba inited");
}

void loop() {
  if(!bDone && millis() > 5000){
    dock();
  }
}

void dock(){
  // send init Roomba 
  roomba.coverAndDock();
  delay(50);

  Serial.println("dock Roomba Sent!");
  bDone = true;
}

void clean(){
  roomba.cover();
  delay(50);

  Serial.println("clean Roomba Sent!");
}
  
