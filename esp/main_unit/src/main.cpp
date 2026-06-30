#include <EVShield.h>
#include <EVs_NXTTouch.h>
#include <Wire.h>

#include <rearm.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting EVShield setup...");

    Wire.begin();
    rearm::shieldInit();

    Serial.println("\nHoming motors...");
    rearm::stopAllMotors(SH_Next_Action_Brake);
    delay(2000);

    if (rearm::homeAllMotors()) {
        Serial.println("All motors homed.");
    } else {
        Serial.println("ERROR: Homing timeout. Abort.");
        while (1)
            ; // hang here
    }

    Serial.println("\nStarting main loop...");
}

void loop() {
    // TODO
}
