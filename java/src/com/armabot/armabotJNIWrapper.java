package com.armabot;

@SuppressWarnings("MethodName")
public class armabotJNIWrapper {
  static boolean libraryLoaded = false;
  
  static {
    if (!libraryLoaded) {
      try {
        System.loadLibrary("sweepDriver");
      } catch (UnsatisfiedLinkError e) {
        e.printStackTrace();
        System.exit(1);
      }
      libraryLoaded = true;
    }
  }
}
