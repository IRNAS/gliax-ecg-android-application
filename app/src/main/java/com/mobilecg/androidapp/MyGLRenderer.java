package com.mobilecg.androidapp;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.DisplayMetrics;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLRenderer implements GLSurfaceView.Renderer {
    private DisplayMetrics displayMetrics;

    MyGLRenderer(DisplayMetrics display) {
        displayMetrics = display;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        gl.glEnable(gl.GL_LINE_SMOOTH);
        gl.glHint(gl.GL_LINE_SMOOTH_HINT, gl.GL_NICEST);
        EcgJNI.surfaceCreated();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        gl.glEnable(gl.GL_LINE_SMOOTH);
        gl.glHint(gl.GL_LINE_SMOOTH_HINT, gl.GL_NICEST);
        EcgJNI.setDotPerCM(displayMetrics.xdpi / 2.54f, displayMetrics.ydpi / 2.54f);
        EcgJNI.surfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        EcgJNI.drawFrame();
    }
}
