#include "Sweep.hpp"

namespace sweep {

Sweep::Sweep() {
  sweep_error_s error = nullptr;
  m_device = sweep_device_construct_simple("/dev/ttyUSB0", &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
    m_device = 0;
  }
}

Sweep::Sweep(const char *port, std::int32_t bitrate) {
  sweep_error_s error = nullptr;
  m_device = sweep_device_construct(port, bitrate, &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
    m_device = 0;
  }
}

Sweep::~Sweep() {
  sweep_device_destruct(m_device);
}

void Sweep::StartScanning() {
  if (StatusIsFatal()) return;
  sweep_error_s error = nullptr;
  sweep_device_start_scanning(m_device, &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
  }
}

void Sweep::StopScanning() {
  if (StatusIsFatal()) return;
  sweep_error_s error = nullptr;
  sweep_device_stop_scanning(m_device, &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
  }
}

int Sweep::GetMotorSpeed() {
  if (StatusIsFatal()) return 0;
  sweep_error_s error = nullptr;
  int rv = sweep_device_get_motor_speed(m_device, &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
  }
  return rv;
}

void Sweep::SetMotorSpeed(int speed) {
  if (StatusIsFatal()) return;
  sweep_error_s error = nullptr;
  sweep_device_set_motor_speed(m_device, speed, &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
  }
}

std::vector<SweepSample> Sweep::GetScan() {
  if (StatusIsFatal()) return std::vector<SweepSample>{};

  sweep_error_s error = nullptr;
  sweep_scan_s scan = sweep_device_get_scan(m_device, &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
    return std::vector<SweepSample>{};
  }

  int num_samples = sweep_scan_get_number_of_samples(scan);

  std::vector<SweepSample> result;
  result.reserve(num_samples);

  for (int n = 0; n < num_samples; ++n) {
    int angle = sweep_scan_get_angle(scan, n);
    int distance = sweep_scan_get_distance(scan, n);
    int signal = sweep_scan_get_signal_strength(scan, n);

    result.emplace_back(angle, distance, signal);
  }

  sweep_scan_destruct(scan);
  return result;
}

void Sweep::Reset() {
  if (StatusIsFatal()) return;
  sweep_error_s error = nullptr;
  sweep_device_reset(m_device, &error);
  if (error) {
    wpi_setErrorWithContext(-100, sweep_error_message(error));
    sweep_error_destruct(error);
  }
}

void Sweep::InitSendable(SendableBuilder& builder) {}

}
