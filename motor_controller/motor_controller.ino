#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>

Servo myservo;
Adafruit_MotorShield AFMS(0x60);
Adafruit_StepperMotor *myStepper1 = AFMS.getStepper(200,1);
Adafruit_StepperMotor *myStepper2 = AFMS.getStepper(200,2); 

#define rDiff -100
#define cDiff -138
#define u -139
#define val1 6 
#define val2 130

int minFeedback;
int maxFeedback;
int M1r, M1c, M2r, M2c, Cr, Cc, cstlKr, cstlKc;  // cstlKr is the final row of the king. cstlKc is the final column of the king.
int pr, pc;
int pointNum;
int state = 0;

void forwardstep1() {myStepper1->onestep(FORWARD, DOUBLE);}
void backwardstep1() {myStepper1->onestep(BACKWARD, DOUBLE);}
void forwardstep2() {myStepper2->onestep(FORWARD, DOUBLE);}
void backwardstep2() {myStepper2->onestep(BACKWARD, DOUBLE);}
AccelStepper stepper1(forwardstep1, backwardstep1);
AccelStepper stepper2(forwardstep2, backwardstep2);

void setup() {
  Serial.begin(9600);
  AFMS.begin();
  TWBR = ((F_CPU /400000l) - 16) / 2;
  stepper1.setMaxSpeed(120);  
  stepper1.setAcceleration(200);
  stepper1.moveTo(0);
  stepper2.setMaxSpeed(120);
  stepper2.setAcceleration(200);
  stepper2.moveTo(0);
  myservo.attach(9);
  calibrate();
  Seek(val2);
  delay(500);
}

void calibrate() {
  myservo.write(0);
  delay(2000); 
  minFeedback = analogRead(A0);
  myservo.write(135);
  delay(2000); 
  maxFeedback = analogRead(A0);
}

void Seek(int pos) {
  int target = map(pos, 0, 135, minFeedback, maxFeedback); 
  while(abs(analogRead(A0) - target) > 3)
    myservo.write(pos);
}

bool isHoming() {
  if(M1r == 255 && M1c == 255 && M2r == 0 && M2c == 0) {
    return 1;
  }
  return 0;
}

bool isMove() {
  if(M1r != 255 && M1c != 255 && M2r != 255 && M2c != 255 && isknight() == 0 && cstlKr == 255 && cstlKc == 255) {
    return 1;
  }
  return 0;
}

bool isknight() {
  if(abs(M1r - M2r) == 1 && abs(M1c - M2c) == 2)
    return 1;
  else if(abs(M1r - M2r) == 2 && abs(M1c - M2c) == 1)
    return 1; 
  else 
    return 0;
}

bool isCapture() {
  if(Cr != 255 && Cc != 255) {
    return 1;
  }
  return 0;
}

void movePoint(int r, int c) {
    pr = rDiff + r*u;
    pc = cDiff + c*u;
}

void capturePoint(int pointNum) {
  if(pointNum == 1) {  //the first point
    pr = (rDiff + Cr*u) - u/2;
    pc = (cDiff + Cc*u);
  }
  else if(pointNum == 2) {  //the second point
    pr = ((rDiff + Cr*u) - u/2);
    pc = cDiff + 7.9*u;
  }
}

void knightMovePoint(int PointNum) {
  if(pointNum == 1) {  //the first point
    int dr = M2r - M1r;
    int dc = M2c - M1c;
    if(abs(dr) == 2) {
      pr = (rDiff + M1r*u) + (dr/4.0)*u;
      pc = (cDiff + M1c*u) + (dc/2.0)*u;
    }
    else if(abs(dr) ==1) {
      pr = (rDiff + M1r*u) + (dr/2.0)*u;
      pc = (cDiff + M1c*u) + (dc/4.0)*u;
    }
  }
  else if(pointNum == 2) {  //the second point
    int dr = M2r - M1r;
    int dc = M2c - M1c;
    if(abs(dr) == 2) {
      pr = ((rDiff + M1r*u) + (dr/4.0)*u) + (dr/2.0)*u;
      pc = ((cDiff + M1c*u) + (dc/2.0)*u);
    }
    else if(abs(dr) == 1) {
      pr = ((rDiff + M1r*u) + (dr/2.0)*u);
      pc = ((cDiff + M1c*u) + (dc/4.0)*u) + (dc/2.0)*u;
    }
  }
  else if(pointNum == 3) {  //the third point
    pr = rDiff + M2r*u;
    pc = cDiff + M2c*u;
  }
}

void castleKPoint(int pointNum) {
  int Kr = cstlKr, Kc = 4;
  if(pointNum == 1) {  //the first point
    pr = rDiff + Kr*u;
    pc = cDiff + Kc*u;
  }
  else if(pointNum == 2) {  //the second point
    pr = rDiff + cstlKr*u;
    pc = cDiff + cstlKc*u;
  }
}

void castlePoint(int pointNum) {
  int colDir = (M2c - M1c)/abs(M2c - M1c);
  int rowDir = -1;
  if(pointNum == 1) {  //the first point
    pr = rDiff + (M1r + rowDir/2.0)*u;
    pc = cDiff + (M1c + colDir/2.0)*u;
  }
  else if(pointNum == 2) {  //the second point
    pr = rDiff + (M1r + rowDir/2.0)*u;
    pc = cDiff + (M2c - colDir/2.0)*u;
  }
  else if(pointNum == 3) {  //the third point
    pr = rDiff + M2r*u;
    pc = cDiff + M2c*u;    
  }
}

void oneMove() {
  stepper1.moveTo(pr);
  stepper2.moveTo(pc);
  while(stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}

void serialEvent() {
  while(Serial.available() < 8);
  M1r = Serial.read();
  M1c = Serial.read();
  M2r = Serial.read();
  M2c = Serial.read();
  Cr = Serial.read();
  Cc = Serial.read();
  cstlKr = Serial.read();
  cstlKc = Serial.read();
  state = 1;
}

void loop() {
  if(state == 1) {
    Seek(val2);
    if(isCapture() == 1) {  //remove the captured piece
      movePoint(Cr, Cc);
      oneMove();
      Seek(val1);
      pointNum = 1;
      while(pointNum <= 2) {
        capturePoint(pointNum);
        oneMove();
        if(pointNum == 2) Seek(val2); 
        pointNum++;
      }
      Serial.write(1);
    }
    if(isknight() == 1) {  //move the knight piece
      movePoint(M1r, M1c);
      oneMove();
      Seek(val1); 
      pointNum = 1;
      while(pointNum <= 3) {
        knightMovePoint(pointNum);
        oneMove();
        if(pointNum == 3) Seek(val2);
        pointNum++;
      }
      Serial.write(2);
    }
    if(isMove() == 1) {  //move regular piece
      movePoint(M1r, M1c);
      oneMove();
      Seek(val1);
      movePoint(M2r, M2c);
      oneMove();
      Seek(val2);
      Serial.write(2);
    }
    if(cstlKr != 255 && cstlKc != 255) {  //castling
      pointNum = 1;  //move the king piece in castling
      while(pointNum <= 2) {
        castleKPoint(pointNum);
        oneMove();
        if(pointNum == 1) Seek(val1);
        else if(pointNum == 2) Seek(val2);
        pointNum++;
      }
      Serial.write(1);
      movePoint(M1r, M1c);  //move the rook piece in castling
      oneMove();
      Seek(val1); 
      pointNum = 1;
      while(pointNum <= 3) {
        castlePoint(pointNum);
        oneMove();
        if(pointNum == 3) Seek(val2);
        pointNum++;
      }
      Serial.write(2);
    }
    if(isHoming() == 1) {
      pr = 0;
      pc = 0;
      oneMove();
    }
    state = 0;
  }
}
