import controlP5.*;
import oscP5.*;
import netP5.*;

ControlP5 cp5;

OscP5 oscP5;
NetAddress roomba;

Roomba rb1, rb2;

void setupUI(){
  cp5 = new ControlP5(this);
  cp5.addButton("RB1_WAKEUP").setPosition(10, 10).setSize(80, 20);
  cp5.addButton("RB1_PASSIVE").setPosition(10, 35).setSize(80, 20);
  cp5.addButton("RB1_SLEEP").setPosition(10, 60).setSize(80, 20);
  cp5.addToggle("RB1_RUN").setPosition(95, 10).setSize(50, 70);
  
  cp5.addButton("RB2_WAKEUP").setPosition(10, 110).setSize(80, 20);
  cp5.addButton("RB2_PASSIVE").setPosition(10,135).setSize(80, 20);
  cp5.addButton("RB2_SLEEP").setPosition(10, 160).setSize(80, 20);
  cp5.addToggle("RB2_RUN").setPosition(95, 110).setSize(50, 70);
}
  

void setup() {
  size(800, 200);
  frameRate(60);
  oscP5 = new OscP5(this, 55555);
  
  rb1 = new Roomba("RB1");
  rb2 = new Roomba("RB2");

  roomba = new NetAddress("192.168.0.255", 54321);
  setupUI();
}

void draw() {
  background(0);
  
  rb1.draw(150, 0);
  rb2.draw(150, 100);
}

void oscEvent(OscMessage m) {
  if (m.checkAddrPattern("/RB1/reply")) {
    rb1.pingCheckTimer = millis();
  }
  if (m.checkAddrPattern("/RB1/sensorValue")) {
    for (int i=0; i<4; i++) {
      rb1.sensorValue[i] = m.get(i).longValue();
    }
  }
  if(m.checkAddrPattern("/RB1/turn")){
    rb1.driveAngle = m.get(0).intValue();
  }

  if(m.checkAddrPattern("/RB1/charge")){
    rb1.battCharge = m.get(0).longValue();
  }
}

public void controlEvent(ControlEvent e){
  String ctrName = e.getController().getName();
  
  // RB1
  if(ctrName == "RB1_WAKEUP")  {
    OscMessage m = new OscMessage("/RB1/wakeup");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if(ctrName == "RB1_PASSIVE")  {
    OscMessage m = new OscMessage("/RB1/passiveMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if(ctrName == "RB1_SLEEP")  {
    OscMessage m = new OscMessage("/RB1/sleepMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if(ctrName == "RB1_RUN"){
    OscMessage m = new OscMessage("/RB1/setRunning");
    if(e.getController().getValue() == 1.0)  m.add(1);
    else                                     m.add(0);
    oscP5.send(m, roomba);
  }
  
  // RB2
  if(ctrName == "RB2_WAKEUP")  {
    OscMessage m = new OscMessage("/RB2/wakeup");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if(ctrName == "RB2_PASSIVE")  {
    OscMessage m = new OscMessage("/RB2/passiveMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if(ctrName == "RB2_SLEEP")  {
    OscMessage m = new OscMessage("/RB2/sleepMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if(ctrName == "RB2_RUN"){
    OscMessage m = new OscMessage("/RB1/running");
    if(e.getController().getValue() == 1.0)  m.add(1);
    else                                     m.add(0);
    oscP5.send(m, roomba);
  }
}
