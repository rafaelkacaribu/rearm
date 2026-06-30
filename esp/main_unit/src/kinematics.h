#pragma once

typedef struct {
    int base;
    int main;
    int aux;
    int claw;
    int grab;
    bool valid;
} MotorAngles;

/**
 * @brief Calculates absolute rotation angles for each motor.
 *
 * @param x Target x-coordinate in mm.
 * @param y Target y-coordinate in mm.
 * @param z Target z-coordinate in mm.
 * @param a Target rotation angle of the claw in degrees.
 * @param w Opening width of the claw in mm.
 *
 * @return MotorAngles containing the final target rotation for each motor.
 */
MotorAngles calculate_motor_angles(int x, int y, int z, int a, int w);
