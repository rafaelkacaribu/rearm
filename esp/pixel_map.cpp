#include <iostream>
#include <cmath>

using namespace std;

#define WPX   4562.0
#define HPX   2392.0
#define VIEW  75.0
#define HMM   275.0

#define PI 3.141592

double to_rad(double rad) {
    return rad / 180 * PI;
}

double px_to_mm(double px) {
    double px_per_mm = (sqrt(WPX*WPX + HPX*HPX)) / (2 * HMM * tan(to_rad(VIEW/2)));
    return px / px_per_mm;
}

int main() {
    double px;
    cout << "px: ";
    cin >> px;
    cout << px_to_mm(px) << endl;

    return 0;
}