#include <EVShield.h>
#include <EVs_NXTTouch.h>
#include <Wire.h>

#include <rearm.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting EVShield setup...");

    Wire.begin();
    shieldInit();

    if(!auxUnitCheck()) {
        Serial.println("Secondary unit is offline.\nWaiting for response...");
        while(!auxUnitCheck())
            delay(1000);
    }
    Serial.println("Connected to the secondary unit.");

    if(!visionUnitCheck()) {
        Serial.println("Vision unit is offline.\nWaiting for response...");
        while(!visionUnitCheck())
            delay(1000);
    }
    Serial.println("Connected to the vision unit.");

    // initialize touch sensors
    touchInit(TOUCH_BASE);
    touchInit(TOUCH_MAIN);
    touchInit(TOUCH_AUX);
    touchInit(TOUCH_CLAW);

    Serial.println("\nHoming motors...");
    stopAllMotors(SH_Next_Action_Brake);
    delay(2000);

    if (homeAllMotors()) {
        Serial.println("All motors homed.");
    } else {
        Serial.println("ERROR: Homing timeout. Abort.");
        while (1); // hang here
    }

    Serial.println("\nStarting main loop...");
}

void loop() {
    // TODO
}
