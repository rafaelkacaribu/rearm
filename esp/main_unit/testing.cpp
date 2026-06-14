#include <iostream>
#include "kinematics.h"

using namespace std;

int main() {
    double current_base_rot = 0;
    double current_main_rot = 0;
    double current_aux_rot = 0;
    double current_claw_rot = 0;

    double x = 155;
    double y = 80;
    double z = 28;
    double w = 30;
    double a = 42;

    JointAngles angles = calculate_joint_angles(x, y, z, a, w);

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
    cout << "main: " << (int)current_main_rot << "°" << endl;
    cout << "aux: " << (int)current_aux_rot << "°" << endl;
    cout << "claw: " << (int)current_claw_rot << "°" << endl;

    cout <<  "----------------------------------" << endl;
    cout << "Output (absolute):" << endl;
    cout << "Base: " << angles.base << "°" << endl;
    cout << "Main: " << angles.main << "°" << endl;
    cout << "Aux: " << angles.aux << "°" << endl;
    cout << "Claw: " << angles.claw << "°" << endl;
    cout << "Grab: " << angles.grab << "°" << endl;
    cout << "----------------------------------" << endl;
    
    return 0;
}

