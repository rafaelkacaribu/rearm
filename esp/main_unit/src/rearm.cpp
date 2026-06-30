#include <EVShield.h>
#include <Wire.h>
#include <rearm.h>

#define VISION_ADDR 0x08
#define AUX_ADDR 0x09

TwoWire I2CExternal = TwoWire(1);

void shieldInit() { shield.init(SH_HardwareI2C); }

void touchInit(Touch sensor) {
    switch (sensor) {
    case TOUCH_BASE:
        touchBase.init(&shield, SH_BBS2);
        break;
    case TOUCH_MAIN:
        touchMain.init(&shield, SH_BAS1);
        break;
    case TOUCH_AUX:
        touchAux.init(&shield, SH_BAS2);
        break;
    case TOUCH_CLAW:
        touchClaw.init(&shield, SH_BBS1);
        break;
    }
}

bool auxUnitCheck() {
    I2CExternal.beginTransmission(AUX_ADDR);
    return I2CExternal.endTransmission() == 0;
}

bool visionUnitCheck() {
    I2CExternal.beginTransmission(VISION_ADDR);
    return I2CExternal.endTransmission() == 0;
}

bool auxUnitMoveBy(int deg) {

    int16_t command = (int16_t)deg;

    I2CExternal.beginTransmission(AUX_ADDR);

    I2CExternal.write((uint8_t)(command >> 8));
    I2CExternal.write((uint8_t)(command & 0xFF));

    return I2CExternal.endTransmission() == 0;
}

ObjectData visionUnitRequestData() {
    ObjectData data = {0, 0, 0, 0, 0, false};

    const uint8_t bytesNeeded = 5 * 2; // 5 int16_t values

    uint8_t received = I2CExternal.requestFrom(VISION_ADDR, bytesNeeded);

    if (received != bytesNeeded) {
        return data;
    }

    int16_t values[5];

    for (int i = 0; i < 5; i++) {
        uint8_t highByte = I2CExternal.read();
        uint8_t lowByte = I2CExternal.read();

        values[i] = (int16_t)((highByte << 8) | lowByte);
    }

    data.x = values[0];
    data.y = values[1];
    data.w = values[2];
    data.h = values[3];
    data.r = values[4];
    data.valid = true;

    return data;
}

bool sendAuxCommand(int value) {
    int16_t command = (int16_t)value;

    I2CExternal.beginTransmission(AUX_ADDR);

    I2CExternal.write((uint8_t)(command >> 8));
    I2CExternal.write((uint8_t)(command & 0xFF));

    return I2CExternal.endTransmission() == 0;
}

void motorStop(Motor motor, SH_Next_Action action) {
    switch (motor) {
    case MOTOR_BASE:
        shield.bank_a.motorStop(SH_Motor_1, action);
        break;
    case MOTOR_MAIN:
        shield.bank_a.motorStop(SH_Motor_2, action);
        break;
    case MOTOR_AUX:
        shield.bank_b.motorStop(SH_Motor_2, action);
        break;
    case MOTOR_CLAW:
        shield.bank_b.motorStop(SH_Motor_1, action);
        break;
    }
}

void motorResetEncoder(Motor motor) {
    switch (motor) {
    case MOTOR_BASE:
        shield.bank_a.motorResetEncoder(SH_Motor_1);
        break;
    case MOTOR_MAIN:
        shield.bank_a.motorResetEncoder(SH_Motor_2);
        break;
    case MOTOR_AUX:
        shield.bank_b.motorResetEncoder(SH_Motor_2);
        break;
    case MOTOR_CLAW:
        shield.bank_b.motorResetEncoder(SH_Motor_1);
        break;
    }
}

void stopAllMotors(SH_Next_Action action) {
    shield.bank_a.motorStop(SH_Motor_1, action);
    shield.bank_a.motorStop(SH_Motor_2, action);
    shield.bank_b.motorStop(SH_Motor_2, action);
    shield.bank_b.motorStop(SH_Motor_1, action);
}

bool homeAllMotors() {
    bool baseHomed = touchBase.isPressed();
    bool mainHomed = touchMain.isPressed();
    bool auxHomed = touchAux.isPressed();
    bool clawHomed = touchClaw.isPressed();
    unsigned long startTime;

    // start motors that are not already pressing their home switch
    if (!mainHomed)
        shield.bank_a.motorRunUnlimited(SH_Motor_2, HOMING_DIR_MAIN, SPEED_MAIN);

    if (!auxHomed)
        shield.bank_b.motorRunUnlimited(SH_Motor_2, HOMING_DIR_AUX, SPEED_AUX);

    // home main and aux motors first
    startTime = millis();
    while (!(mainHomed && auxHomed)) {
        if (!mainHomed && touchMain.isPressed()) {
            motorStop(MOTOR_MAIN, SH_Next_Action_Brake);
            motorResetEncoder(MOTOR_MAIN);
            mainHomed = true;
        }

        if (!auxHomed && touchAux.isPressed()) {
            motorStop(MOTOR_AUX, SH_Next_Action_Brake);
            motorResetEncoder(MOTOR_AUX);
            auxHomed = true;
        }

        // safety timeout
        if (millis() - startTime > HOMING_TIMEOUT) {
            stopAllMotors(SH_Next_Action_Brake);
            return false;
        }

        delay(10);
    }

    // home base and claw motors

    if (!baseHomed)
        shield.bank_a.motorRunUnlimited(SH_Motor_1, HOMING_DIR_BASE, SPEED_BASE);

    if (!clawHomed)
        shield.bank_b.motorRunUnlimited(SH_Motor_1, HOMING_DIR_CLAW, SPEED_CLAW);

    startTime = millis();
    while (!(baseHomed && clawHomed)) {
        if (!baseHomed && touchBase.isPressed()) {
            motorStop(MOTOR_BASE, SH_Next_Action_Brake);
            motorResetEncoder(MOTOR_BASE);
            baseHomed = true;
        }

        if (!clawHomed && touchClaw.isPressed()) {
            motorStop(MOTOR_CLAW, SH_Next_Action_Brake);
            motorResetEncoder(MOTOR_CLAW);
            clawHomed = true;
        }

        // safety timeout
        if (millis() - startTime > HOMING_TIMEOUT) {
            stopAllMotors(SH_Next_Action_Brake);
            return false;
        }
    }

    stopAllMotors(SH_Next_Action_Brake);
    return true;
}