/*
 * C++ Wrapper around the low-level primitives.
 * Automatically handles resource management.
 *
 * sweep::Sweep  - device to interact with
 * sweep::SweepSample - a single sample in a full scan
 */

#pragma once

#include <vector>

#include "SensorBase.h"

#include "sweep.h"

namespace sweep {

struct SweepSample {
  SweepSample(int angle_, int distance_, int signalStrength_)
      : angle(angle_), distance(distance_), signalStrength(signalStrength_) {}
  int angle;
  int distance;
  int signalStrength;
};

class Sweep : public frc::SensorBase {
 public:
  Sweep();
  Sweep(const char* port, int bitrate);
  Sweep(const Sweep&) = delete;
  Sweep& operator=(const Sweep&) = delete;
  ~Sweep();

  void StartScanning();
  void StopScanning();

  int GetMotorSpeed();
  void SetMotorSpeed(int speed);

  std::vector<SweepSample> GetScan();

  void Reset();

  void InitSendable(SendableBuilder& builder) override;

 private:
  sweep_device* m_device;
};

}  // namespace sweep
