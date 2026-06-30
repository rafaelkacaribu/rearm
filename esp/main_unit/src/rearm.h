

#include <EVShield.h>
#include <EVs_NXTTouch.h>

#define SPEED_BASE 6
#define SPEED_MAIN 7
#define SPEED_AUX 7
#define SPEED_CLAW 7

#define HOMING_TIMEOUT 30000 // 30 seconds

namespace rearm {

typedef enum {
    MOTOR_BASE,
    MOTOR_MAIN,
    MOTOR_AUX,
    MOTOR_CLAW,
} Motor;

typedef enum {
    TOUCH_BASE,
    TOUCH_MAIN,
    TOUCH_AUX,
    TOUCH_CLAW,
} Touch;

struct ObjectData {
    int x;
    int y;
    int w;
    int h;
    int r;
    bool valid;
};

extern EVShield shield;

extern EVs_NXTTouch touchBase;
extern EVs_NXTTouch touchMain;
extern EVs_NXTTouch touchClaw;
extern EVs_NXTTouch touchAux;

// Directions used during homing.

const SH_Direction HOMING_DIR_BASE = SH_Direction_Forward;
const SH_Direction HOMING_DIR_MAIN = SH_Direction_Forward;
const SH_Direction HOMING_DIR_CLAW = SH_Direction_Reverse;
const SH_Direction HOMING_DIR_AUX = SH_Direction_Reverse;

/**
 * @brief Initializes main EVShield
 */
void shieldInit();

/**
 * @brief Initializes a touch sensor
 */
void touchInit(Touch sensor);

/**
 * @brief Checks if the secondary EVShield is reachable
 */
bool auxUnitCheck();

/**
 * @brief Checks if the secondary EVShield is reachable
 */
bool visionUnitCheck();

/**
 * @brief Sends a rotate command to the secondary EVShield via I2C
 * @param deg Relative angle in degrees
 * @return true if successfuly sent
 */
bool auxUnitMoveBy(int deg);

/**
 * @brief Initiates object detection via I2C
 * @return Object data structure
 */
ObjectData visionUnitRequestData();

/**
 * @brief Stops given motor
 */
void motorStop(Motor motor, SH_Next_Action action);

/**
 * @brief Resets the internal position to 0
 */
void motorResetEncoder(Motor motor);

/**
 * @brief Stops all four motors
 */
void stopAllMotors(SH_Next_Action action);

/**
 * @brief Executes a homing sequence for all motors
 */
bool homeAllMotors();

} // namespace rearm
