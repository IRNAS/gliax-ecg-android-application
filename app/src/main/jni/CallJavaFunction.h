//
// Created by vidra on 14. 08. 2019.
//

#ifndef ANDROIDAPP_CALLJAVAFUNCTION_H
#define ANDROIDAPP_CALLJAVAFUNCTION_H

#include <jni.h>

// variables and methods for native code calling Java functions
class CallJavaFunction {
    private:
        JNIEnv* rhy_env;
        jclass rhy_clazz;
        jmethodID rhy_mid;

    public:
        CallJavaFunction();
        CallJavaFunction(JNIEnv* env, jclass clazz, jmethodID mid);

        void EndOfLayout();
};
#endif //ANDROIDAPP_CALLJAVAFUNCTION_H
