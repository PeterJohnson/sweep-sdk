#pragma once

#include <jni.h>
#include <stdint.h>

#include "sweep.h"

namespace armabot {

void ReportError(JNIEnv *env, sweep_error_s error, bool do_throw);

}

inline bool CheckStatus(JNIEnv *env, sweep_error_s error,
                        bool do_throw = true) {
  if (error) {
    armabot::ReportError(env, error, do_throw);
    sweep_error_destruct(error);
    return false;
  }
  return true;
}
