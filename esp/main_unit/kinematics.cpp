#include "kinematics.h"
#include <cmath>

#define PI 3.141592

#define BASE_ROT_RATIO  11.2
#define MAIN_ARM_RATIO  24.0
#define AUX_ARM_RATIO   5.0
#define CLAW_ROT_RATIO  11.67
#define CLAW_GRAB_RATIO 40

#define ZONE_W      392.0 
#define ZONE_H      208.0
#define ZONE_GAP    93.0

#define BASE_ELEVATION  119.0
#define MAIN_ARM_LEN    136.0
#define MAIN_ARM_ROT    45.0 // TODO: adjust
#define AUX_ARM_LEN     80.0
#define CLAW_EXTENSION  40.0
#define CLAW_HEIGHT     112.0
#define CLAW_LENGTH     48.0
#define CLAW_BASE_DIST  32.0
#define CLAW_ROT_INIT   45.0

namespace {

    inline double to_deg(double rad) {
        return rad * 180 / PI;
    }

    inline double dist_x(double x) {
        return x - ZONE_W / 2;
    }

    inline double dist_y(double y) {
        return ZONE_H + ZONE_GAP - y;
    }

    double base_rotation(double x, double y) {
        double dx = dist_x(x);
        double dy = dist_y(y);

        return 90 + to_deg(atan(dx/dy));
    }

    double dist_obj(double x, double y) {
      return sqrt(dist_x(x)*dist_x(x) + dist_y(y) * dist_y(y));
    }

    double main_arm_rotation(double x, double y, double z) {
        double dist =  dist_obj(x, y);
        double diagonal = sqrt(
                pow(dist - CLAW_EXTENSION, 2) + pow(z + CLAW_HEIGHT - BASE_ELEVATION, 2)
                );

        double angle1 = acos((pow(MAIN_ARM_LEN, 2) + pow(diagonal, 2) - pow(AUX_ARM_LEN, 2)) / (2 * MAIN_ARM_LEN * diagonal));
        double angle2 = asin((z + CLAW_HEIGHT - BASE_ELEVATION) / diagonal);

        return 90  - to_deg(angle1 + angle2) - MAIN_ARM_ROT;
    }

    double aux_arm_rotation(double x, double y, double z) {
        double dist = dist_obj(x, y);
        double diagonal = sqrt(
                pow(dist - CLAW_EXTENSION, 2) + pow(z + CLAW_HEIGHT - BASE_ELEVATION, 2)
                );
        double angle = acos((pow(MAIN_ARM_LEN,2) + pow(AUX_ARM_LEN, 2) - pow(diagonal, 2)) / (2 * MAIN_ARM_LEN * AUX_ARM_LEN));

        return 180 - to_deg(angle);
    }

    double claw_grab_rotation(double w) {
        return 90 - (CLAW_ROT_INIT + to_deg(asin(((w - CLAW_BASE_DIST) / 2) / CLAW_LENGTH)));
    }

}

JointAngles calculate_joint_angles(int x, int y, int z, int a, int w) {
    JointAngles angles = {};

    angles.base = (int)(base_rotation(x, y) * BASE_ROT_RATIO / BASE_ROT_RATIO);
    angles.main = (int)(main_arm_rotation(x, y, z) * MAIN_ARM_RATIO / MAIN_ARM_RATIO);
    angles.aux = (int)(aux_arm_rotation(x, y, z) * AUX_ARM_RATIO / AUX_ARM_RATIO);
    angles.claw = (int)((a % 180) * CLAW_ROT_RATIO / CLAW_ROT_RATIO);
    angles.grab = (int)(claw_grab_rotation(w) * CLAW_GRAB_RATIO / CLAW_GRAB_RATIO);

    return angles;
}
