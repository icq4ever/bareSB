class Roomba {
  String name;
  double pingTimer, requestTimer;
  double pingCheckTimer;
  boolean pingOK = false;
  long sensorValue[] = new long[4];
  long battCharge;
  int driveAngle = 0;

  Roomba(String _name) {
    name = _name;
    pingTimer =  requestTimer = millis();
  }

  void draw(float _x, float _y) {
    if (millis() - pingTimer > 200) {
      sendPing();
      pingTimer = millis();
    }
    if (millis() - requestTimer > 100) {
      sendRequestMessage();
      requestTimer = millis();
    }

    // ping check
    if (millis() - pingCheckTimer >5000) pingOK = false;
    else                                 pingOK = true;

    pushMatrix();
    translate(_x, _y);

    fill(255);
    noStroke();
    textAlign(RIGHT);
    for (int i=0; i<4; i++) {
      stroke(255);
      noFill();
      rect(140, i*20+10, 420, 10); 
      fill(255);
      text(int(sensorValue[i]), 128, i*20+20);
      fill(255, 255, 0);
      noStroke();
      rect(140, i*20+10, map(sensorValue[i], 0, 3000, 0, 420), 10);
    }

    if (pingOK)    fill(0, 255, 0);
    else          fill(255, 0, 0);

    ellipse(45, 45, 70, 70);

    // esp32 status
    textAlign(LEFT, CENTER);
    fill(255);
    if (pingOK) {
      fill(0);
      text(this.name + "\nCOMM\nOK", 30, 45);
    } else {
      fill(255);
      text(this.name + "\nCOMM\nXX", 30, 45);
    }

    fill(255, 255, 0);
    textAlign(RIGHT);
    text((int)battCharge, 605, 20);
    textAlign(LEFT);
    text("mAh", 610, 20);
    
    fill(0, 255, 255);
    testAlign(LEFT);
    if(driveAngle > 0)        text(">>", 580, 40);
    else if(driveAngle < 0)   text("<<", 580, 40);
    else                      text("--", 580, 40);
    
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
