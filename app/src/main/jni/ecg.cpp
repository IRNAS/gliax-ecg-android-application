/*
 * This file is part of gliax-ecg-android-application
 * Glia is a project with the goal of releasing high quality free/open medical hardware
 * to increase availability to those who need it.
 * For more information visit Glia Free Medical hardware webpage: https://glia.org/
 *
 * Made by Institute Irnas (https://www.irnas.eu/)
 * Copyright (C) 2019 Vid Rajtmajer
 *
 * Based on MobilECG, an open source clinical grade Holter ECG.
 * For more information visit http://mobilecg.hu
 * Authors: Robert Csordas, Peter Isza
 *
 * This project uses modified version of usb-serial-for-android driver library
 * to communicate with Irnas made ECG board.
 * Original source code: https://github.com/mik3y/usb-serial-for-android
 * Library made by mik3y and kai-morich, modified by Vid Rajtmajer
 * Licensed under LGPL Version 2.1
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <jni.h>
#include <GLES2/gl2.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <android/log.h>

#include <cstdint>
#include <cassert>
#include <string>

#include "EcgArea.h"
#include "Helper.h"
#include "log.h"
#include "PacketRouter.h"
#include "ShaderBuilder.h"
#include "../res/Common/DataFormat/PacketReader.cpp"

const int LOOPER_ID_USER = 3;
JavaVM* cachedJVM = 0;

class ecg {

 public:

    ecg() {}

    void init(AAssetManager *assetManager) {
        EcgArea::instance().init(assetManager);
    }

    void surfaceCreated() {
        LOGI("GL_VERSION: %s", glGetString(GL_VERSION));
        LOGI("GL_VENDOR: %s", glGetString(GL_VENDOR));
        LOGI("GL_RENDERER: %s", glGetString(GL_RENDERER));
        LOGI("GL_EXTENSIONS: %s", glGetString(GL_EXTENSIONS));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        ShaderBuilder::instance().reset();

        EcgArea::instance().glInit();
        EcgArea::instance().setZOrder(1);
    }

    void surfaceChanged(int w, int h) {
        glViewport(0, 0, w, h);
        EcgArea::instance().contextResized(w,h);
        EcgArea::instance().redraw();
    }

    void setDpcm(float x, float y){
        EcgArea::instance().setPixelDensity(Vec2<float>(x,y));
        EcgArea::instance().redraw();
    }

    void render() {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        EcgArea::instance().draw();
    }

    void pause() {
        EcgArea::instance().resetContent();
    }

    void resume() {
        EcgArea::instance().resetContent();
        EcgArea::instance().redraw();
    }

    void onEcgConnected(){
        EcgArea::instance().deviceConnected();
    }

    void onEcgDisconnected(){
        EcgArea::instance().deviceDisconnected();
    }

    void changeLayout() {
        EcgArea::instance().changeLayout();
    }

    int getCurrentLayout() {
        return EcgArea::instance().getCurrentLayout();
    }

    int getRhyStatus() {
        return EcgArea::instance().getRhyScreenFull();
    }
};

ecg gEcg;

extern "C" {
    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_init(JNIEnv *env, jclass type, jobject assetManager) {
        (void)type;
        AAssetManager *nativeAssetManager = AAssetManager_fromJava(env, assetManager);
        gEcg.init(nativeAssetManager);
        //LOGD("ecgJNI init call");
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_surfaceCreated(JNIEnv *env, jclass type) {
        (void)env;
        (void)type;
        gEcg.surfaceCreated();
        env->GetJavaVM(&cachedJVM);
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_surfaceChanged(JNIEnv *env, jclass type, jint width,
                                                     jint height) {
        (void)env;
        (void)type;
        gEcg.surfaceChanged(width, height);
    }


    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_setDotPerCM(JNIEnv *env, jclass type, jfloat xdpcm,
                                                           jfloat ydpcm) {
        (void)env;
        (void)type;
        gEcg.setDpcm(xdpcm, ydpcm);
    }


    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_drawFrame(JNIEnv *env, jclass type) {
        (void)env;
        (void)type;
        gEcg.render();
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_pause(JNIEnv *env, jclass type) {
        (void)env;
        (void)type;
        gEcg.pause();
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_resume(JNIEnv *env, jclass type) {
        (void)env;
        (void)type;
        gEcg.resume();
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_processEcgData(JNIEnv *env, jclass type, jbyteArray jdata, jint size) {
        (void)type;

        jbyte* data = env->GetByteArrayElements(jdata, 0);
        char* chars = (char*) data;

        static PacketReader packetReader;
        for(int i=0; i < size; ++i) {
            packetReader.addByte(chars[i]);
            if(packetReader.isPacketReady()) {
                PacketRouter::instance().packetReceived(packetReader.getPacketHeader(), packetReader.getPacketData());
            }
        }
        env->ReleaseByteArrayElements(jdata, data, 0);
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_onDeviceConnected(JNIEnv *env, jclass type) {
        (void)env;
        (void)type;
        gEcg.onEcgConnected();
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_onDeviceDisconnected(JNIEnv *env, jclass type) {
        (void)env;
        (void)type;
        gEcg.onEcgDisconnected();
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_initNDK(JNIEnv *env, jclass type, jstring str) {
        (void)type;
        EcgArea::instance().internalStoragePath = env->GetStringUTFChars(str, 0);
        struct stat sb;
        int32_t res = stat(EcgArea::instance().internalStoragePath, &sb);
        if (0 == res && sb.st_mode && S_IFDIR){
            //LOGD("'%s' dir already in app's internal data storage.",EcgArea::instance().internalStoragePath);
        }
        else if (ENOENT == errno){
            res = mkdir(EcgArea::instance().internalStoragePath, 0770);
        }
    }

    JNIEXPORT void JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_ChangeSpeed(JNIEnv *env, jclass type, jfloat x_speed) {
        (void)env;
        (void)type;
        //LOGD("HEH %f", x_speed);
        EcgArea::instance().setSpeed(x_speed);
    }

    JNIEXPORT jint JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_changeLayout(JNIEnv *env, jclass type) {
        (void)env;
        (void)type;
        gEcg.changeLayout();
        //gEcg.resume();
        return gEcg.getCurrentLayout();
    }

    JNIEXPORT jint JNICALL
    Java_com_mobilecg_androidapp_EcgJNI_getRhyFull(JNIEnv* env, jclass type) {
        (void)env;
        (void)type;
        return gEcg.getRhyStatus();
    }
}
