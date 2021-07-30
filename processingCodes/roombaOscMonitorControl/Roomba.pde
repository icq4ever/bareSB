class Roomba {
  String name;
  boolean reportOn;
  double pingTimer, requestTimer;
  double pingCheckTimer;
  boolean pingOK = false;
  long sensorValue[] = new long[4];
  long battCharge, battCapacity;
  int driveAngle = 0;
  boolean commOK = false;
  int recivedOptCode[] = new int[3];
  String currentModeString = "off";
  String currentChargingState = "unknown";
  
  int roombaStatus = 0;  
  String reportedMessage = "";

  Roomba(String _name) {
    name = _name;
    pingTimer =  requestTimer = millis();
  }

  void draw(float _x, float _y) {
    switch(roombaStatus){
      case 0 : 
      case 63:
      rb1.currentModeString = "off";
      break;
    case 1 :
      rb1.currentModeString = "passive";
      break;
    case 2:
      rb1.currentModeString = "safe";
      break;
    case 3 :
      rb1.currentModeString = "full";
      break;
    default:
      rb1.currentModeString = "unknown";
      break;
    }

    if (millis() - pingTimer > 500) {
      sendPing();
      pingTimer = millis();
    }
    
    if(reportOn) {
      if (millis() - requestTimer > 200) {
        sendRequestMessage();
        requestTimer = millis();
      }
    }

    // ping check
    if (millis() - pingCheckTimer > 5000) pingOK = false;
    else                                  pingOK = true;

    pushMatrix();
    translate(_x, _y);

    fill(255);
    noStroke();
    textAlign(RIGHT);
    for (int i=0; i<4; i++) {
      stroke(255);
      strokeWeight(1);
      noFill();
      rect(140, i*20+10, 420, 10); 
      fill(255);
      text(int(sensorValue[i]), 128, i*20+20);
      fill(255, 255, 0);
      noStroke();
      rect(140, i*20+10, map(sensorValue[i], 0, 4095, 0, 420), 10);
    }
    
    noStroke();
    if (commOK)   fill(0, 255, 0);
    else          fill(255, 0, 0);
    ellipse(45, 45, 70, 70);
    
    noFill();
    strokeWeight(3);
    if (pingOK)   stroke(0, 255, 0);
    else          stroke(255, 0, 0);
    ellipse(45, 45, 70, 70);

    // esp32 status
    textAlign(LEFT, CENTER);
    fill(255);

    if (commOK) {
      fill(0);
      text(this.name + "\nCOMM\nOK", 30, 45);
    } else {
      fill(255);
      text(this.name + "\nCOMM\nXX", 30, 45);
    }

    fill(255, 255, 0);
    textAlign(RIGHT);
    text((int)battCharge, 605, 20);
    text("/ " + (int)battCapacity, 605, 40);
    textAlign(LEFT);
    text("mAh", 610, 20);
    text("mAh", 610, 40);
    
    //reported
    fill(0, 255, 0);
    textAlign(LEFT);
    text(currentModeString, 580, 60);
    
    fill(0, 255, 255);
    textAlign(LEFT);
    if(driveAngle > 0)        text(">>", 580, 80);
    else if(driveAngle < 0)   text("<<", 580, 80);
    else                      text("--", 580, 80);
    
    popMatrix();
  }

  void sendPing() {
    OscMessage m = new OscMessage("/" + name + "/ping");
    m.add(1);
    oscP5.send(m, roomba);
  }

  void sendRequestMessage() {
    OscMessage m = new OscMessage("/" + name + "/requestSensor");
    m.add(1);
    oscP5.send(m, roomba);
  }
}
