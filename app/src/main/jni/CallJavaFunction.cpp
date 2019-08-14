//
// Created by vidra on 14. 08. 2019.
//

#include "CallJavaFunction.h"

CallJavaFunction::CallJavaFunction() {}

CallJavaFunction::CallJavaFunction(JNIEnv *env, jclass clazz, jmethodID mid) {
    rhy_env = env;
    rhy_clazz = clazz;
    rhy_mid = mid;
}

void CallJavaFunction::EndOfLayout() {
    //jclass clazz = rhy_env->FindClass("com/mobilecg/androidapp/EcgActivity");
    //jmethodID mid = rhy_env->GetStaticMethodID(clazz, "RhyLayoutFull", "()V");
    rhy_env->CallStaticObjectMethod(rhy_clazz, rhy_mid);
}
