package com.armabot;

import edu.wpi.first.wpilibj.SensorBase;
import edu.wpi.first.wpilibj.smartdashboard.SendableBuilder;

public class Sweep extends SensorBase {
  public Sweep() {
    m_device = SweepJNI.constructSimple();
  }

  public Sweep(String port, int bitrate) {
    m_device = SweepJNI.construct(port, bitrate);
  }

  public void free() {
    if (m_device != 0) {
      SweepJNI.destruct(m_device);
    }
    m_device = 0;
  }

  @Override
  @SuppressWarnings("NoFinalizer")
  protected void finalize() {
    free();
  }

  void startScanning() {
    SweepJNI.startScanning(m_device);
  }

  void stopScanning() {
    SweepJNI.stopScanning(m_device);
  }

  int getMotorSpeed() {
    return SweepJNI.getMotorSpeed(m_device);
  }

  void setMotorSpeed(int speed) {
    SweepJNI.setMotorSpeed(m_device, speed);
  }

  SweepSample[] getScan() {
    return SweepJNI.getScan(m_device);
  }

  void reset() {
    SweepJNI.reset(m_device);
  }

  @Override
  public void initSendable(SendableBuilder builder) {

  }

  private long m_device;
}
