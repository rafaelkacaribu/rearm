#include <iostream>
#include <cmath>
#include <ostream>

#define PI 3.141592

#define BASE_ROT_RATIO  11.2
#define MAIN_ARM_RATIO  24.0
#define AUX_ARM_RATIO   5.0
#define CLAW_ROT_RATIO  11.67
#define CLAW_GRAB_RATIO 40

#define ZONE_W      552.0 // 392
#define ZONE_H      208.0
#define ZONE_GAP    93.0

#define BASE_ELEVATION  119.0
#define MAIN_ARM_LEN    136.0
#define AUX_ARM_LEN     80.0
#define CLAW_EXTENSION  40.0
#define CLAW_HEIGHT     112.0
#define CLAW_LENGTH     48.0
#define CLAW_BASE_DIST  32.0
#define CLAW_ROT_INIT   44.97

using namespace std;

double to_deg(double rad) {
    return rad * 180 / PI;
}

double dist_x(double x) {
    return x - ZONE_W / 2;
}

double dist_y(double y) {
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

double main_arm_rotation(double dist, double z) {
    double diagonal = sqrt(
            pow(dist - CLAW_EXTENSION, 2) + pow(z + CLAW_HEIGHT - BASE_ELEVATION, 2)
    );

    double angle1 = acos((pow(MAIN_ARM_LEN, 2) + pow(diagonal, 2) - pow(AUX_ARM_LEN, 2)) / (2 * MAIN_ARM_LEN * diagonal));
    double angle2 = asin((z + CLAW_HEIGHT - BASE_ELEVATION) / diagonal);

    return 90  - to_deg(angle1 + angle2);
}

double aux_arm_rotation(double dist, double z) {
    double diagonal = sqrt(
            pow(dist - CLAW_EXTENSION, 2) + pow(z + CLAW_HEIGHT - BASE_ELEVATION, 2)
        );
    double angle = acos((pow(MAIN_ARM_LEN,2) + pow(AUX_ARM_LEN, 2) - pow(diagonal, 2)) / (2 * MAIN_ARM_LEN * AUX_ARM_LEN));

    return 180 - to_deg(angle);
}

double claw_grab_rotation(double w) {
    return 90 - (CLAW_ROT_INIT + to_deg(asin(((w - CLAW_BASE_DIST) / 2) / CLAW_LENGTH)));
}

int main() {
    double current_base_rot = 0;
    double current_main_rot = 0;
    double current_aux_rot = 0;
    double current_claw_rot = 0;

    double x = 155;
    double y = 80;
    double z = 28;
    double w = 30;
    double a = 0;

    double dist = dist_obj(x, y);

    double base_rot = base_rotation(x, y) - current_base_rot;
    double base_rot_final = base_rot * BASE_ROT_RATIO;

    double main_arm_rot = main_arm_rotation(dist, z) - current_main_rot;
    double main_arm_rot_final = main_arm_rot * MAIN_ARM_RATIO;

    double aux_arm_rot = aux_arm_rotation(dist, z) - current_aux_rot;
    double aux_arm_rot_final = aux_arm_rot * AUX_ARM_RATIO;

    double claw_grab_rot = claw_grab_rotation(w);
    double claw_grab_rot_final = claw_grab_rot * CLAW_ROT_RATIO;

    int claw_rot = (int)a % 180 - current_claw_rot;

    cout << "----------------------------------" << endl;
    cout << "Input (absolute):" << endl;
    cout << "x: " << x << "mm" << endl;
    cout << "y: " << y << "mm" << endl;
    cout << "z: " << z << "mm" << endl;
    cout << "w: " << w << "mm" << endl;
    cout << "a: " << a << "°" << endl;
    
    cout << "-----------------------------------" << endl;
    cout << "Current state:" << endl;
    cout << "base: " << (int)current_base_rot << "°" <<  endl;
    cout << "main arm: " << (int)current_main_rot << "°" << endl;
    cout << "aux arm: " << (int)current_aux_rot << "°" << endl;
    cout << "claw: " << (int)current_claw_rot << "°" << endl;

    cout <<  "----------------------------------" << endl;
    cout << "Output (relative):" << endl;
    cout << "object dist: " << dist_obj(x, y) << "mm" << endl;
    cout << "base (final): " << (int)base_rot << "° (" << (int)base_rot_final << "°)" << endl;
    cout << "main arm (final): " << (int)main_arm_rot << "° (" << (int)main_arm_rot_final << "°)" << endl;
    cout << "aux arm (final): " << (int)aux_arm_rot << "° (" << (int)aux_arm_rot_final << "°)" << endl;
    cout << "claw grab (final): " << (int)claw_grab_rot << "° (" << (int)claw_grab_rot_final << "°)" << endl;
    cout << "claw rotation: " << (int)claw_rot << "°" << endl;

    cout << "----------------------------------" << endl;

    return 0;
}

