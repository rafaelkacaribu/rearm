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

EVs_NXTTouch touchBase;
EVs_NXTTouch touchMain;
EVs_NXTTouch touchClaw;
EVs_NXTTouch touchAux;

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
  touchBase.init(&evshield, SH_BBS2);
  touchMain.init(&evshield, SH_BAS1);
  touchAux.init(&evshield, SH_BAS2);
  touchClaw.init(&evshield, SH_BBS1);

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

  bool baseHomed = touchBase.isPressed();
  bool mainHomed = touchMain.isPressed();
  bool auxHomed = touchAux.isPressed();
  bool clawHomed = touchClaw.isPressed();

  unsigned long startTime = millis();

  // start motors that are not already pressing their home switch

  if (!mainHomed) {
    evshield.bank_a.motorRunUnlimited(
        SH_Motor_2,
        HOMING_DIR_A2,
        HOMING_SPEED
        );
  }

  if (!auxHomed) {
    evshield.bank_b.motorRunUnlimited(
        SH_Motor_2,
        HOMING_DIR_B2,
        HOMING_SPEED
        );
  }

  while (!(mainHomed && auxHomed)) {
    if (!mainHomed && touchMain.isPressed()) {
      evshield.bank_a.motorStop(SH_Motor_2, SH_Next_Action_Brake);
      evshield.bank_a.motorResetEncoder(SH_Motor_2);

      mainHomed = true;
      Serial.println("Main arm motor homed.");
    }

    if (!auxHomed && touchAux.isPressed()) {
      evshield.bank_b.motorStop(SH_Motor_2, SH_Next_Action_Brake);
      evshield.bank_b.motorResetEncoder(SH_Motor_2);

      auxHomed = true;
      Serial.println("Aux arm motor B2 homed.");
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

  if (!baseHomed) {
    evshield.bank_a.motorRunUnlimited(
        SH_Motor_1,
        HOMING_DIR_A1,
        6
        );
  }

  if (!clawHomed) {
    evshield.bank_b.motorRunUnlimited(
        SH_Motor_1,
        HOMING_DIR_B1,
        HOMING_SPEED
        );
  }

  // monitor sensors until all motors are homed
  while (!(baseHomed && clawHomed)) {
    if (!baseHomed && touchBase.isPressed()) {
      evshield.bank_a.motorStop(SH_Motor_1, SH_Next_Action_Brake);
      evshield.bank_a.motorResetEncoder(SH_Motor_1);

      baseHomed = true;
      Serial.println("Base motor homed.");
    }

    if (!clawHomed && touchClaw.isPressed()) {
      evshield.bank_b.motorStop(SH_Motor_1, SH_Next_Action_Float);
      evshield.bank_b.motorResetEncoder(SH_Motor_1);

      clawHomed = true;
      Serial.println("Claw motor homed.");
    }

    // safety timeout
    if (millis() - startTime > HOMING_TIMEOUT) {
      stopAllMotors();
      Serial.println("ERROR: Homing timeout 1.");
      Serial.println("Check motor direction, wiring, and touch sensors.");
      return;
    }

  }

  stopAllMotors();

  Serial.println("All motors homed.");
}
