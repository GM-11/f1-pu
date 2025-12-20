#include "../include/ice_engine.hpp"
#include <bits/stdc++.h>
#include <iostream>
int main() {
  ICEEngine engine;

  double dt = 0.0001; // 1 ms timestep

  double throttle_init = 0;
  engine.setThrottle(throttle_init); // full throttle

  std::ofstream log("data/engine_log.csv");
  log << "time,rpm,torque,omega,power\n";

  int iterations = 100000;

  for (int i = 0; i < iterations; i++) {
    engine.update(dt);

    if (i % 10 == 0) {
      std::cout << "RPM = " << engine.getRPM()
                << " revs/min \tTorque = " << engine.getTorqueOutput()
                << " Nm \tAngular Velocity = " << engine.getAngularVelocity()
                << " rad/s \tNet Power = " << engine.getNetPower() << " W\n";
    }

    log << i * dt << "," << engine.getRPM() << "," << engine.getTorqueOutput()
        << "," << engine.getAngularVelocity() << "," << engine.getNetPower()
        << "\n";

    if (throttle_init < 1.0) {
      throttle_init += 0.001; // Rate: 50% per second
      engine.setThrottle(throttle_init);
    }
  }
}
