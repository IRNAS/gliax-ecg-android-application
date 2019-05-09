package com.mobilecg.androidapp;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.pdf.PdfDocument;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.print.PrintAttributes;
import android.print.pdf.PrintedPdfDocument;
import android.util.DisplayMetrics;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FilePermission;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;
import java.util.Calendar;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLRenderer implements GLSurfaceView.Renderer {
    private DisplayMetrics displayMetrics;
    private int glHeight;
    private int glWidth;

    private boolean screenshot = false;
    private boolean screenshotResult = false;
    private String savePath = null;
    private Patient thisPatient = null;

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
        glHeight = height;
        glWidth = width;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        EcgJNI.drawFrame();
        // make screenshot of open gl screen area
        if (screenshot) {
            Log.d("HEH", "taking screenshot...");
            int width = glWidth;
            int height = glHeight;
            int screenshotSize = width * height;
            ByteBuffer byteBuffer = ByteBuffer.allocateDirect(screenshotSize * 4);
            byteBuffer.order(ByteOrder.nativeOrder());
            gl.glReadPixels(0,0, width, height, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, byteBuffer);
            int pixelsBuffer[] = new int[screenshotSize];
            byteBuffer.asIntBuffer().get(pixelsBuffer);

            Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
            bitmap.setPixels(pixelsBuffer, screenshotSize - width, -width, 0, 0, width, height);

            short sBuffer[] = new short[screenshotSize];
            ShortBuffer shortBuffer = ShortBuffer.wrap(sBuffer);
            bitmap.copyPixelsToBuffer(shortBuffer);

            for (int i = 0; i < screenshotSize; i++) {
                short v = sBuffer[i];
                sBuffer[i] = (short) (((v&0x1f) << 11) | (v&0x7e0) | ((v&0xf800) >> 11));
            }
            shortBuffer.rewind();
            bitmap.copyPixelsFromBuffer(shortBuffer);

            saveScreenshot(bitmap);
            screenshot = false;
        }
    }

    public void takeScreenshot(String saveLocation, Patient patient) {
        screenshot = true;
        savePath = saveLocation;
        thisPatient = patient;
    }

    private void saveScreenshot(Bitmap pic) {
        // save screenshot to pdf file with added text info
        try {
            String measurementTimestamp = java.text.DateFormat.getDateTimeInstance().format(Calendar.getInstance().getTime());
            String id = thisPatient.getMeasurementId();
            if (id == "") {
                id = "000";
            }
            String filename = id + "_" + measurementTimestamp.replace(" ", "-") + ".pdf";
            int textSize = 50;

            PdfDocument document = new PdfDocument();
            PdfDocument.PageInfo pageInfo = new PdfDocument.PageInfo.Builder(pic.getWidth(), pic.getHeight() + textSize + 10, 1).create();
            PdfDocument.Page page = document.startPage(pageInfo);

            Canvas canvas = page.getCanvas();
            Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
            //paint.setColor(Color.BLACK);
            paint.setTextSize(textSize);
            // save id and timestamp
            String title = String.format("ID: %s \t Date: %s", id, measurementTimestamp);
            canvas.drawText(title, 10, textSize + 1, paint);
            // save patient info to file if exists
            if (thisPatient.getName() == "" && thisPatient.getSurname() == "" && thisPatient.getBirth() == "") {
                String noPatient = String.format("No patient data");
                canvas.drawText(noPatient, pic.getWidth() - noPatient.length()*23, textSize + 1, paint);
            }
            else {
                String patientInfo = String.format("Patient: %s %s, %s", thisPatient.getName(), thisPatient.getSurname(), thisPatient.getBirth());
                canvas.drawText(patientInfo, pic.getWidth() - patientInfo.length()*23, textSize + 1, paint);
                // TODO handle too long names
            }
            // save screenshot to file
            canvas.drawBitmap(pic,0,textSize + 10,null);
            document.finishPage(page);

            String dir = Environment.getExternalStorageDirectory() + File.separator + savePath + File.separator;
            File file = new File(dir + filename);
            document.writeTo(new FileOutputStream(file));
            document.close();
            screenshotResult = true;

            /*  // DEBUG - to save picture directly
            File file = new File(dir + filename);
            FileOutputStream outputStream = new FileOutputStream(file);
            pic.compress(Bitmap.CompressFormat.PNG, 85, outputStream);
            outputStream.flush();
            outputStream.close();
            */
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.i("HEH", "Save pdf file error!");
        }
    }

    public boolean getScreenshotResult() {
        boolean result = screenshotResult;
        screenshotResult = false;
        return result;
    }
}
