#include "com_armabot_SweepJNI.h"
#include "armabotJNIUtilities.h"

#include "support/jni_util.h"

#include "sweep.h"

using namespace wpi::java;

extern "C" {

/*
 * Class:     com_armabot_SweepJNI
 * Method:    getVersion
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_armabot_SweepJNI_getVersion
  (JNIEnv *, jclass)
{
  return sweep_get_version();
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    isAbiCompatible
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_armabot_SweepJNI_isAbiCompatible
  (JNIEnv *, jclass)
{
  return sweep_is_abi_compatible() ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    constructSimple
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_armabot_SweepJNI_constructSimple
  (JNIEnv *env, jclass)
{
  sweep_error_s error = nullptr;
  sweep_device_s rv = sweep_device_construct_simple("/dev/ttyUSB0", &error);
  if (!CheckStatus(env, error)) return 0;
  return reinterpret_cast<jlong>(rv);
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    construct
 * Signature: (Ljava/lang/String;I)J
 */
JNIEXPORT jlong JNICALL Java_com_armabot_SweepJNI_construct
  (JNIEnv *env, jclass, jstring port, jint bitrate)
{
  sweep_error_s error = nullptr;
  sweep_device_s rv =
      sweep_device_construct(JStringRef{env, port}.c_str(), bitrate, &error);
  if (!CheckStatus(env, error)) return 0;
  return reinterpret_cast<jlong>(rv);
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    destruct
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_armabot_SweepJNI_destruct
  (JNIEnv *, jclass, jlong device)
{
  sweep_device_destruct(reinterpret_cast<sweep_device_s>(device));
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    startScanning
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_armabot_SweepJNI_startScanning
  (JNIEnv *env, jclass, jlong device)
{
  sweep_error_s error = nullptr;
  sweep_device_start_scanning(reinterpret_cast<sweep_device_s>(device), &error);
  CheckStatus(env, error);
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    stopScanning
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_armabot_SweepJNI_stopScanning
  (JNIEnv *env, jclass, jlong device)
{
  sweep_error_s error = nullptr;
  sweep_device_stop_scanning(reinterpret_cast<sweep_device_s>(device), &error);
  CheckStatus(env, error);
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    getScan
 * Signature: (J)[Lcom/armabot/SweepSample;
 */
JNIEXPORT jobjectArray JNICALL Java_com_armabot_SweepJNI_getScan
  (JNIEnv *env, jclass, jlong device)
{
  static JClass elemCls{env, "com/armabot/SweepSample"};
  if (!elemCls) return nullptr;
  static jmethodID constructorId = nullptr;
  if (!constructorId)
    constructorId = env->GetMethodID(elemCls, "<init>", "(III)V");

  sweep_error_s error = nullptr;
  sweep_scan_s scan =
      sweep_device_get_scan(reinterpret_cast<sweep_device_s>(device), &error);
  if (!CheckStatus(env, error)) return nullptr;

  int num_samples = sweep_scan_get_number_of_samples(scan);

  jobjectArray jarr = env->NewObjectArray(num_samples, elemCls, nullptr);
  if (!jarr) {
    sweep_scan_destruct(scan);
    return nullptr;
  }
  for (int i = 0; i < num_samples; ++i) {
    jint angle = sweep_scan_get_angle(scan, i);
    jint distance = sweep_scan_get_distance(scan, i);
    jint signal = sweep_scan_get_signal_strength(scan, i);

    JLocal<jobject> elem{
        env, env->NewObject(elemCls, constructorId, angle, distance, signal)};
    env->SetObjectArrayElement(jarr, i, elem.obj());
  }

  sweep_scan_destruct(scan);
  return jarr;
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    getMotorSpeed
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_armabot_SweepJNI_getMotorSpeed
  (JNIEnv *env, jclass, jlong device)
{
  sweep_error_s error = nullptr;
  int rv = sweep_device_get_motor_speed(
      reinterpret_cast<sweep_device_s>(device), &error);
  if (!CheckStatus(env, error)) return 0;
  return rv;
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    setMotorSpeed
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_com_armabot_SweepJNI_setMotorSpeed
  (JNIEnv *env, jclass, jlong device, jint speed)
{
  sweep_error_s error = nullptr;
  sweep_device_set_motor_speed(reinterpret_cast<sweep_device_s>(device), speed,
                               &error);
  CheckStatus(env, error);
}

/*
 * Class:     com_armabot_SweepJNI
 * Method:    reset
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_armabot_SweepJNI_reset
  (JNIEnv *env, jclass, jlong device)
{
  sweep_error_s error = nullptr;
  sweep_device_reset(reinterpret_cast<sweep_device_s>(device), &error);
  CheckStatus(env, error);
}

}
