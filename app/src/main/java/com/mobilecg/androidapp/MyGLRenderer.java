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
import java.lang.reflect.Array;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;
import java.util.ArrayList;
import java.util.Calendar;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.opengles.GL10;

public class MyGLRenderer implements GLSurfaceView.Renderer {
    private DisplayMetrics displayMetrics;
    private int glHeight;
    private int glWidth;

    private boolean screenshot = false;
    private int screenshotType = States.SHOT_ONE;
    private String savePath;
    private Patient thisPatient;
    private String ecgType;

    private boolean screenshotResult = false;
    private Bitmap preparedScreenshot;
    private ArrayList<Bitmap> screenshotArray;

    private int textSize = 50;
    private int textOffsetX = 10;
    private int textOffsetY = 15;

    MyGLRenderer(DisplayMetrics display) {
        displayMetrics = display;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        gl.glEnable(gl.GL_LINE_SMOOTH);
        gl.glHint(gl.GL_LINE_SMOOTH_HINT, gl.GL_NICEST);
        EcgJNI.surfaceCreated();
        screenshotArray = new ArrayList<>();
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
        //GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);   // TODO check if it fixes screen blinking

        EcgJNI.drawFrame();
        if (screenshot) {
            Bitmap bitmap = makeScreenshot(gl);
            if (screenshotType == States.SHOT_PREPARE) {
                preparedScreenshot = bitmap;
            }
            else if (screenshotType == States.SHOT_MANY) {
                screenshotArray.add(bitmap);
            }
            else {  // screenshotType == States.SHOT_ONE
                saveScreenshot(bitmap);
                preparedScreenshot = null;
            }
            screenshot = false;
        }
    }

    public void takeScreenshot(String saveLocation, Patient patient, int shot_type, String ecg_type) {
        screenshotResult = false;   // reset completion flag
        screenshot = true;
        screenshotType = shot_type;
        savePath = saveLocation;
        thisPatient = patient;
        ecgType = ecg_type;
    }

    private Bitmap makeScreenshot(GL10 gl) {
        //Log.d("HEH", "taking screenshot...");
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
        return bitmap;
    }

    private void saveScreenshot(Bitmap pic) {
        // save screenshot to pdf file with added text info
        try {
            String measurementTimestamp = java.text.DateFormat.getDateTimeInstance().format(Calendar.getInstance().getTime());
            String id = thisPatient.getMeasurementId();
            String filename = id + "_" + ecgType + "_" + measurementTimestamp.replace(" ", "-") + ".pdf";

            PdfDocument document = new PdfDocument();
            // prepare id and timestamp
            String title = String.format("ID: %s \t Date: %s", id, measurementTimestamp);
            // prepare patient info to file if exists
            String patientInfo = String.format("No patient data");
            if (!thisPatient.getName().isEmpty() || !thisPatient.getSurname().isEmpty() || !thisPatient.getBirth().isEmpty()) {
                patientInfo = String.format("Patient: %s %s, %s", thisPatient.getName(), thisPatient.getSurname(), thisPatient.getBirth());
            }
            // prepare page number
            String pageNum = String.format("Page: %s / %s", 1, 1);

            PdfDocument.PageInfo pageInfo = new PdfDocument.PageInfo.Builder(pic.getWidth(), pic.getHeight() + textSize*2 + textOffsetY*2, 1).create();
            PdfDocument.Page page = document.startPage(pageInfo);
            Canvas canvas = page.getCanvas();
            Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
            //paint.setColor(Color.BLACK);
            paint.setTextSize(textSize);
            // draw title
            canvas.drawText(title, textOffsetX, textSize + 1, paint);
            // draw page number
            canvas.drawText(pageNum, pic.getWidth() - pageNum.length()*23, textSize + 1, paint);
            // draw screenshot
            canvas.drawBitmap(pic, 0, textSize + textOffsetY,null);
            // draw patient info
            canvas.drawText(patientInfo, textOffsetX, pic.getHeight() + textSize*2 + textOffsetY, paint);
            document.finishPage(page);

            String dir = Environment.getExternalStorageDirectory() + File.separator + savePath + File.separator;
            File file = new File(dir + filename);
            document.writeTo(new FileOutputStream(file));
            document.close();
            screenshotResult = true;

            /*  // DEBUG - to save picture directly
            String dir = Environment.getExternalStorageDirectory() + File.separator + savePath + File.separator;
            File file = new File(dir + "test.png");
            FileOutputStream outputStream = new FileOutputStream(file);
            pic.compress(Bitmap.CompressFormat.PNG, 85, outputStream);
            outputStream.flush();
            outputStream.close();
            */
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.i("HEH", "Save pdf file error!");
            screenshotResult = false;
        }
    }

    public void savePreparedScreenshot() {
        if (preparedScreenshot != null) {
            saveScreenshot(preparedScreenshot);
        }
        else {
            Log.i("HEH", "preparedScreenshot is null!");
        }
    }

    public void saveManyScreenshots() {
        try {
            if (screenshotArray.isEmpty()) {    // if bitmap array is empty: take new single screenshot
                takeScreenshot(savePath, thisPatient, States.SHOT_ONE, ecgType);
                return;
            }

            String measurementTimestamp = java.text.DateFormat.getDateTimeInstance().format(Calendar.getInstance().getTime());
            String id = thisPatient.getMeasurementId();
            String filename = id + "_" + ecgType + "_" + measurementTimestamp.replace(" ", "-") + ".pdf";

            PdfDocument document = new PdfDocument();
            // prepare id and timestamp
            String title = String.format("ID: %s \t Date: %s", id, measurementTimestamp);
            // prepare patient info to file if exists
            String patientInfo = String.format("No patient data");
            if (!thisPatient.getName().isEmpty() || !thisPatient.getSurname().isEmpty() || !thisPatient.getBirth().isEmpty()) {
                patientInfo = String.format("Patient: %s %s, %s", thisPatient.getName(), thisPatient.getSurname(), thisPatient.getBirth());
            }
            int pageCounter = 0;
            int pagesCount = screenshotArray.size();

            for (Bitmap pic : screenshotArray) {
                pageCounter++;
                PdfDocument.PageInfo pageInfo = new PdfDocument.PageInfo.Builder(pic.getWidth(), pic.getHeight() + textSize*2 + textOffsetY*2, pageCounter+1).create();
                PdfDocument.Page page = document.startPage(pageInfo);
                Canvas canvas = page.getCanvas();
                Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
                //paint.setColor(Color.BLACK);
                paint.setTextSize(textSize);
                // draw title
                canvas.drawText(title, textOffsetX, textSize + 1, paint);
                // draw page number
                String pageNum = String.format("Page: %s / %s", pageCounter, pagesCount);
                canvas.drawText(pageNum, pic.getWidth() - pageNum.length()*23, textSize + 1, paint);
                // draw screenshot
                canvas.drawBitmap(pic, 0, textSize + textOffsetY,null);
                // draw patient info
                canvas.drawText(patientInfo, textOffsetX, pic.getHeight() + textSize*2 + textOffsetY, paint);
                document.finishPage(page);
            }

            String dir = Environment.getExternalStorageDirectory() + File.separator + savePath + File.separator;
            File file = new File(dir + filename);
            document.writeTo(new FileOutputStream(file));
            document.close();
            screenshotResult = true;
        }
        catch (Exception e) {
            e.printStackTrace();
            Log.i("HEH", "saveManyScreenshots error!");
            screenshotResult = false;
        }
    }

    public boolean getScreenshotResult() {
        return screenshotResult;
    }

    public void deleteManyScreenshots() {
        screenshotArray = new ArrayList<>();
    }
}
