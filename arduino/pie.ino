#include <PID_v1.h>

// pins
const byte pinLeftLaser = 2;
const byte pinRightLaser = 3;

const byte pinLeftPWM = 5;
const byte pinLeft2 = 6;
const byte pinLeft1 = 7;
const byte pinStandby = 8;
const byte pinRight1 = 9;
const byte pinRight2 = 10;
const byte pinRightPWM = 11;

// defaults
const double dDelay = 200;
const byte countStep = 20;
const int defaultPWM = 70;
const double defaultSpeed = 50;

// global variables
int lCounter = 0;
int rCounter = 0;
unsigned long lastLTime = 0;
unsigned long lastRTime = 0;
double lSpeed = 0;
double rSpeed = 0;
double lrPWMAdjSum = 0;
double speedPWMAdjSum = 0;
int leftPWM = 0;
int rightPWM = 0;
double lrSetpoint, speedSetpoint, speedDiff, speedAvg, lrPWMAdj, speedPWMAdj;
PID lrPID(&speedDiff, &lrPWMAdj, &lrSetpoint, 0.23, 0.00, 0.04, DIRECT);
PID speedPID(&speedAvg, &speedPWMAdj, &speedSetpoint, 0.05, 0, 0, DIRECT);

void lCount() {
  lCounter++;

  // compute speed
  if (lCounter % countStep == 0) {
    unsigned long now = millis();
    lSpeed = countStep * 1000 / double(now - lastLTime);
    /* Serial.println(now - lastLTime); */
    /* Serial.println(lSpeed); */
    lastLTime = now;
  }
}

void rCount() {
  rCounter++;

  // compute speed
  if (rCounter % countStep == 0) {
    unsigned long now = millis();
    rSpeed = countStep * 1000 / double(now - lastRTime);
    /* Serial.println(now - lastRTime); */
    /* Serial.println(rSpeed); */
    lastRTime = now;
  }
}

void computePIDInput() {
  speedDiff = lSpeed - rSpeed;
  speedAvg = (lSpeed + rSpeed) / 2;
}

void reset() {
  lCounter = 0;
  rCounter = 0;
  lastLTime = millis();
  lastRTime = millis();
  leftPWM = defaultPWM;
  rightPWM = defaultPWM;

  lrSetpoint = 0;
  lrPID.SetMode(MANUAL);
  lrPID.SetMode(AUTOMATIC);

  speedSetpoint = defaultSpeed;
  speedPID.SetMode(MANUAL);
  speedPID.SetMode(AUTOMATIC);
}

void updatePWM() {
  computePIDInput();
  Serial.print(speedDiff);
  Serial.print(" ");
  Serial.print(speedAvg);

  lrPID.Compute();
  Serial.print(" ");
  Serial.print(lrPWMAdj);
  lrPWMAdjSum += lrPWMAdj;

  speedPID.Compute();
  Serial.print(" ");
  Serial.print(speedPWMAdj);
  speedPWMAdjSum += speedPWMAdj;

  leftPWM = constrain(defaultPWM+int(lrPWMAdjSum)+int(speedPWMAdjSum), 0, 255);
  Serial.print(" ");
  Serial.print(leftPWM);

  rightPWM = constrain(defaultPWM-int(lrPWMAdjSum)+int(speedPWMAdjSum), 0, 255);
  Serial.print(" ");
  Serial.println(rightPWM);

  analogWrite(pinLeftPWM, leftPWM);
  analogWrite(pinRightPWM, rightPWM);

  delay(dDelay);
}

void forward() {
  digitalWrite(pinLeft1, HIGH);
  digitalWrite(pinLeft2, LOW);
  digitalWrite(pinRight1, HIGH);
  digitalWrite(pinRight2, LOW);

  updatePWM();
}

void backward() {
  digitalWrite(pinLeft1, LOW);
  digitalWrite(pinLeft2, HIGH);
  digitalWrite(pinRight1, LOW);
  digitalWrite(pinRight2, HIGH);

  updatePWM();
}

void turnLeft() {
  digitalWrite(pinLeft1, LOW);
  digitalWrite(pinLeft2, HIGH);
  digitalWrite(pinRight1, HIGH);
  digitalWrite(pinRight2, LOW);

  updatePWM();
}

void turnRight() {
  digitalWrite(pinLeft1, HIGH);
  digitalWrite(pinLeft2, LOW);
  digitalWrite(pinRight1, LOW);
  digitalWrite(pinRight2, HIGH);

  updatePWM();
}

void stop() {
  digitalWrite(pinLeft1, LOW);
  digitalWrite(pinLeft2, LOW);
  digitalWrite(pinRight1, LOW);
  digitalWrite(pinRight2, LOW);
}

void setup() {
  Serial.begin(9600);
  Serial.println("diff adj left right");
  
  // put your setup code here, to run once:
  pinMode(pinStandby, OUTPUT);
  pinMode(pinLeft1, OUTPUT);
  pinMode(pinLeft2, OUTPUT);
  pinMode(pinRight1, OUTPUT);
  pinMode(pinRight2, OUTPUT);
  pinMode(pinLeftPWM, OUTPUT);
  pinMode(pinRightPWM, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  
  pinMode(pinLeftLaser, INPUT);
  pinMode(pinRightLaser, INPUT);

  digitalWrite(pinStandby, HIGH);

  attachInterrupt(digitalPinToInterrupt(pinLeftLaser), lCount, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinRightLaser), rCount, CHANGE);
  
  analogWrite(pinLeftPWM, leftPWM);
  analogWrite(pinRightPWM, rightPWM);


  //turn the PID on
  lrPID.SetOutputLimits(-255, 255);
  lrPID.SetMode(AUTOMATIC);
  lrPID.SetSampleTime(dDelay);

  speedPID.SetOutputLimits(-255, 255);
  speedPID.SetMode(AUTOMATIC);
  speedPID.SetSampleTime(dDelay);

  stop();
  reset();
}

void loop() {
  int outA0 = digitalRead(A0);
  int outA1 = digitalRead(A1);
  int outA2 = digitalRead(A2);
  int outA3 = digitalRead(A3);

  // forward
  if (outA0 == HIGH && outA1 == LOW && outA2 == LOW && outA3 == LOW) {
    forward();
  } else if (outA0 == LOW && outA1 == HIGH && outA2 == LOW && outA3 == LOW) {
    backward();
  } else if (outA0 == LOW && outA1 == LOW && outA2 == HIGH && outA3 == LOW) {
    turnLeft();
  } else if (outA0 == LOW && outA1 == LOW && outA2 == LOW && outA3 == HIGH) {
    turnRight();
  } else {
    stop();
    reset();
  }
}
