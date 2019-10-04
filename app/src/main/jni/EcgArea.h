/*
 * This file is part of MobilECG, an open source clinical grade Holter
 * ECG. For more information visit http://mobilecg.hu
 *
 * Copyright (C) 2016  Robert Csordas, Peter Isza
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
 
#ifndef ANDROIDAPP_ECGAREA_H
#define ANDROIDAPP_ECGAREA_H


#include "DrawableGroup.h"
#include "GridDrawer.h"
#include "TextDrawer.h"
#include "Rect.h"
#include "Vec2.h"
#include "Curve.h"
#include "Circle.h"
#include "Rectangle.h"

class EcgArea: public DrawableGroup{
    private:
        static const int ECG_CURVE_COUNT = 12;

        const char *labelText[ECG_CURVE_COUNT] = {"I", "II", "III", "aVR", "aVL", "aVF", "V1", "V2", "V3", "V4", "V5", "V6"};
        const char *rhythm_text = "RHYTHM";

        const int NORMAL_LAYOUT = 0;
        const int RHYTHM_LAYOUT = 1;
        int selected_layout;

        EcgArea();

        Rect activeArea;
        Vec2<float> pixelDensity;
        int calculateUnalignedArea(int size, float dpcm);
        GridDrawer grid;
        TextDrawer labels[ECG_CURVE_COUNT];
        TextDrawer rhythm_label;
        TextDrawer devLabel;
        TextDrawer speed_warning_label;
        TextDrawer disconnectedLabel;
        TextDrawer hr_label;
        TextDrawer bpm_label;
        TextDrawer bpm_num;
        int bpm_num_width;

        Curve ecgCurves[ECG_CURVE_COUNT];
        Curve rhythm;
        //It is somewhat ugly to use endpoint circles separated from their
        //curves, but it avoids unneccessary shader switches.
        Circle endpointCircles[ECG_CURVE_COUNT];
        Circle rhythm_circle;

        float lastSampleFrequency;

        float padInCm;
        bool redrawNeeded;

        void constructLayout();
        void constructLayoutNormal();
        void constructLayoutRhythm();
        void rescale();

        float ecgCmPerMv;
        float ecgCmPerSec;

        Vec2<int> screenSize;

        //int availableHeight;
        //int curvePositions[12];
        //int timers[12];

        static const int ECG_COLUMN_COUNT = 4;
        int cur_column;
        int rhy_remains; // OPTION 2
        int layout_remains;

        bool deviceNotConnected;
        int x_speed_color;
        int last_color_change;

        void setContentVisible(bool visible);

        int rhy_screen_full;
public:
        static EcgArea &instance();
        virtual void contextResized(int w, int h);

        const Rect &getActiveArea();
        const Vec2<float> &getPixelDensity();

        const char * internalStoragePath;

        void redraw();
        bool isRedrawNeeded();
        virtual void draw();
        void setPixelDensity(const Vec2<float> &pixelDensity);

        void putData(GLfloat *data, int nChannels, int nPoints, int stride, int bpm, int cur_time);
        virtual void init(AAssetManager *assetManager, int mains_freq);

        void deviceConnected();
        void deviceDisconnected();

        void resetContent();
        void changeLayout();

        void setSpeed(float speed);
        int getRhyScreenFull();
};

#endif //ANDROIDAPP_ECGAREA_H
