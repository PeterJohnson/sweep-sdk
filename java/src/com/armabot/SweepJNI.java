package com.armabot;

@SuppressWarnings("MethodName")
public class SweepJNI extends armabotJNIWrapper {  
  public static native int getVersion();
  public static native boolean isAbiCompatible();

  public static native long constructSimple();
  public static native long construct(String port, int bitrate);
  public static native void destruct(long device);

  public static native void startScanning(long device);
  public static native void stopScanning(long device);

  public static native SweepSample[] getScan(long device);

  public static native int getMotorSpeed(long device);
  public static native void setMotorSpeed(long device, int hz);

  public static native void reset(long device);
}
