#include <Wire.h>
#include <EVShield.h>
#include <EVs_NXTTouch.h>

EVShield evshield;

// Touch sensors
// Motor A1 -> Touch sensor BAS1
// Motor A2 -> Touch sensor BAS2
// Motor B1 -> Touch sensor BBS1
// Motor B2 -> Touch sensor BBS2
// TODO adjust ports

EVs_NXTTouch touchA1;
EVs_NXTTouch touchA2;
EVs_NXTTouch touchB1;
EVs_NXTTouch touchB2;

const int HOMING_SPEED = 7; 
const unsigned long HOMING_TIMEOUT = 60000; // 15 seconds

// Direction used during homing.
const SH_Direction HOMING_DIR_A1 = SH_Direction_Forward;
const SH_Direction HOMING_DIR_A2 = SH_Direction_Forward;
const SH_Direction HOMING_DIR_B1 = SH_Direction_Reverse;
const SH_Direction HOMING_DIR_B2 = SH_Direction_Reverse;

// ------------------------------------------------------------
// Function declarations
// ------------------------------------------------------------

void stopAllMotors();
void homeAllMotors();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting EVShield setup...");

  Wire.begin();

  evshield.init(SH_HardwareI2C);

  // initialize touch sensors
  touchA1.init(&evshield, SH_BBS2);
  touchA2.init(&evshield, SH_BAS1);
  touchB1.init(&evshield, SH_BBS1);
  touchB2.init(&evshield, SH_BAS2);

  stopAllMotors();
  


  // main homing call
  homeAllMotors();

  Serial.println("Setup finished.");
}

void loop() {
  // TODO
}

// stop all motors
void stopAllMotors() {
  evshield.bank_a.motorStop(SH_Motor_Both, SH_Next_Action_Brake);
  evshield.bank_b.motorStop(SH_Motor_Both, SH_Next_Action_Brake);
}

// home all four motors
void homeAllMotors() {
  Serial.println("Starting homing...");

  bool homedA1 = touchA1.isPressed();
  bool homedA2 = touchA2.isPressed();
  bool homedB1 = touchB1.isPressed();
  bool homedB2 = touchB2.isPressed();

  unsigned long startTime = millis();

  // start motors that are not already pressing their home switch

  if (!homedA2) {
    evshield.bank_a.motorRunUnlimited(
        SH_Motor_2,
        HOMING_DIR_A2,
        HOMING_SPEED
        );
  }

  if (!homedB2) {
    evshield.bank_b.motorRunUnlimited(
        SH_Motor_2,
        HOMING_DIR_B2,
        HOMING_SPEED
        );
  }

  while (!(homedA2 && homedB2)) {
    if (!homedA2 && touchA2.isPressed()) {
      evshield.bank_a.motorStop(SH_Motor_2, SH_Next_Action_Brake);
      evshield.bank_a.motorResetEncoder(SH_Motor_2);

      homedA2 = true;
      Serial.println("Motor A2 homed.");
    }

    if (!homedB2 && touchB2.isPressed()) {
      evshield.bank_b.motorStop(SH_Motor_2, SH_Next_Action_Brake);
      evshield.bank_b.motorResetEncoder(SH_Motor_2);

      homedB2 = true;
      Serial.println("Motor B2 homed.");
    }

    // safety timeout
    if (millis() - startTime > HOMING_TIMEOUT) {
      stopAllMotors();
      Serial.println("ERROR: Homing timeout 2.");
      Serial.println("Check motor direction, wiring, and touch sensors.");
      return;
    }

    delay(10);
  }

  startTime = millis();

  if (!homedA1) {
    evshield.bank_a.motorRunUnlimited(
        SH_Motor_1,
        HOMING_DIR_A1,
        HOMING_SPEED
        );
  }

  if (!homedB1) {
    evshield.bank_b.motorRunUnlimited(
        SH_Motor_1,
        HOMING_DIR_B1,
        HOMING_SPEED
        );
  }

  // monitor sensors until all motors are homed
  while (!(homedA1 && homedB1)) {
    if (!homedA1 && touchA1.isPressed()) {
      evshield.bank_a.motorStop(SH_Motor_1, SH_Next_Action_Brake);
      evshield.bank_a.motorResetEncoder(SH_Motor_1);

      homedA1 = true;
      Serial.println("Motor A1 homed.");
    }

    if (!homedB1 && touchB1.isPressed()) {
      evshield.bank_b.motorStop(SH_Motor_1, SH_Next_Action_Brake);
      evshield.bank_b.motorResetEncoder(SH_Motor_1);

      homedB1 = true;
      Serial.println("Motor B1 homed.");
    }

    // safety timeout
    if (millis() - startTime > HOMING_TIMEOUT) {
      stopAllMotors();
      Serial.println("ERROR: Homing timeout 1.");
      Serial.println("Check motor direction, wiring, and touch sensors.");
      return;
    }

    delay(10);
  }

  stopAllMotors();

  Serial.println("All motors homed.");
}
