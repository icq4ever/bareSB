import controlP5.*;
import oscP5.*;
import netP5.*;

ControlP5 cp5;

OscP5 oscP5;
NetAddress roomba;

Roomba rb1, rb2;
PFont font;

void setupUI() {
  cp5 = new ControlP5(this);
  cp5.addButton("RB1_WAKEUP").setPosition(10, 10).setSize(80, 20);
  cp5.addButton("RB1_SLEEP").setPosition(10, 35).setSize(80, 20);
  cp5.addToggle("RB1_REPORT").setPosition(10, 60).setSize(80, 20);
  cp5.addButton("RB1_DOCK").setPosition(95, 10).setSize(50, 20);
  cp5.addToggle("RB1_RUN").setPosition(95, 35).setSize(50, 45);
  
  cp5.addButton("RB2_WAKEUP").setPosition(10, 110).setSize(80, 20);
  cp5.addButton("RB2_SLEEP").setPosition(10, 135).setSize(80, 20);
  cp5.addToggle("RB2_REPORT").setPosition(10, 160).setSize(80, 20);
  cp5.addButton("RB2_DOCK").setPosition(95, 110).setSize(50, 20);
  cp5.addToggle("RB2_RUN").setPosition(95, 135).setSize(50, 45);
}


void setup() {
  size(800, 200);
  frameRate(60);
  smooth();
  oscP5 = new OscP5(this, 55555);

  font = loadFont("Iosevka-Term-12.vlw"); 

  rb1 = new Roomba("RB1");
  rb2 = new Roomba("RB2");

  roomba = new NetAddress("192.168.0.255", 54321);
  setupUI();
  textFont(font);
}

void draw() {
  background(0);

  rb1.draw(150, 0);
  rb2.draw(150, 100);
}

void oscEvent(OscMessage m) {
  // RB1
  if (m.checkAddrPattern("/RB1/report")) {
    rb1.reportedMessage = m.get(0).stringValue();
    if (m.get(0).stringValue() == "wakeup")            rb1.roombaStatus = 1;
    else if (m.get(0).stringValue() == "safemode")     rb1.roombaStatus = 1;
    else if (m.get(0).stringValue() == "passivemode")  rb1.roombaStatus = 0;
    else                                               rb1.roombaStatus = -1;
  }
  if (m.checkAddrPattern("/RB1/currentMode")) {
    //println("roomba mode : " + m.get(0).intValue());
    rb1.roombaStatus = m.get(0).intValue();
  }
  if (m.checkAddrPattern("/RB1/reply")) {
    if (m.get(0).intValue() == 1)  rb1.commOK = true;
    else                          rb1.commOK = false;
    rb1.pingCheckTimer = millis();
  }
  if (m.checkAddrPattern("/RB1/sensorValue")) {
    for (int i=0; i<4; i++) {
      rb1.sensorValue[i] = m.get(i).longValue();
    }
  }
  if (m.checkAddrPattern("/RB1/turn")) {
    rb1.driveAngle = m.get(0).intValue();
  }
  if (m.checkAddrPattern("/RB1/charge")) {
    rb1.battCharge = m.get(0).longValue();
    rb1.battCapacity = m.get(1).longValue();
  }
  if (m.checkAddrPattern("/RB1/optCode")) {
    rb1.recivedOptCode[0] = m.get(0).intValue();
    rb1.recivedOptCode[1] = m.get(1).intValue();
  }

  // RB2
  if (m.checkAddrPattern("/RB2/report")) {
    rb2.reportedMessage = m.get(0).stringValue();
    if (m.get(0).stringValue() == "wakeup")            rb2.roombaStatus = 1;
    else if (m.get(0).stringValue() == "safemode")     rb2.roombaStatus = 1;
    else if (m.get(0).stringValue() == "passivemode")  rb2.roombaStatus = 0;
    else                                               rb2.roombaStatus = -1;
  }
  
  if (m.checkAddrPattern("/RB2/currentMode")) {
    //println("roomba mode : " + m.get(0).intValue());
    rb2.roombaStatus = m.get(0).intValue();
  }
  
  if (m.checkAddrPattern("/RB2/reply")) {
    if (m.get(0).intValue() == 1)  rb2.commOK = true;
    else                          rb2.commOK = false;
    rb2.pingCheckTimer = millis();
  }
  if (m.checkAddrPattern("/RB2/sensorValue")) {
    for (int i=0; i<4; i++) {
      rb2.sensorValue[i] = m.get(i).longValue();
    }
  }
  if (m.checkAddrPattern("/RB2/turn")) {
    rb2.driveAngle = m.get(0).intValue();
  }
  if (m.checkAddrPattern("/RB2/charge")) {
    rb2.battCharge = m.get(0).longValue();
    rb2.battCapacity = m.get(1).longValue();
  }
  if (m.checkAddrPattern("/RB2/optCode")) {
    rb2.recivedOptCode[0] = m.get(0).intValue();
    rb2.recivedOptCode[1] = m.get(1).intValue();
  }
}


public void controlEvent(ControlEvent e) {
  String ctrName = e.getController().getName();

  // RB1
  if (ctrName == "RB1_WAKEUP") {
    OscMessage m = new OscMessage("/RB1/wakeup");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB1_PASSIVE") {
    OscMessage m = new OscMessage("/RB1/passiveMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB1_SLEEP") {
    OscMessage m = new OscMessage("/RB1/sleepMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB1_REPORT") {
    if (e.getController().getValue() == 1.0) {
      rb1.reportOn = true;
    } else {
      rb1.reportOn = false;
      for (int i=0; i<rb1.sensorValue.length; i++) rb1.sensorValue[i] = 0;
    }
  }
  if (ctrName == "RB1_DOCK") {
    OscMessage m = new OscMessage("/RB1/docking");
    if (e.getController().getValue() == 1.0)  m.add(1);
    else                                     m.add(0);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB1_RUN") {
    OscMessage m = new OscMessage("/RB1/setRunning");
    if (e.getController().getValue() == 1.0)  m.add(1);
    else                                     m.add(0);
    oscP5.send(m, roomba);
  }



  // RB2
  if (ctrName == "RB2_WAKEUP") {
    OscMessage m = new OscMessage("/RB2/wakeup");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB2_PASSIVE") {
    OscMessage m = new OscMessage("/RB2/passiveMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB2_SLEEP") {
    OscMessage m = new OscMessage("/RB2/sleepMode");
    m.add(1);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB2_REPORT") {
    if (e.getController().getValue() == 1.0) {
      rb2.reportOn = true;
    } else {
      rb2.reportOn = false;
      for (int i=0; i<rb2.sensorValue.length; i++) rb2.sensorValue[i] = 0;
    }
  }
  if (ctrName == "RB2_DOCK") {
    OscMessage m = new OscMessage("/RB2/docking");
    if (e.getController().getValue() == 1.0)  m.add(1);
    else                                     m.add(0);
    oscP5.send(m, roomba);
  }
  if (ctrName == "RB2_RUN") {
    OscMessage m = new OscMessage("/RB2/setRunning");
    if (e.getController().getValue() == 1.0)  m.add(1);
    else                                     m.add(0);
    oscP5.send(m, roomba);
  }
}
