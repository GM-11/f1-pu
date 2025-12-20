#include "../include/constants.hpp"
#include <cmath>

double constants::torqueCurve(double rpm) {
  double peak_rpm = 15000.0;
  double peak_torque = 300.0;

  double x = rpm / peak_rpm;
  double shape = std::exp(-std::pow(x - 1.0, 2) * 3.0);

  return peak_torque * shape;
}
