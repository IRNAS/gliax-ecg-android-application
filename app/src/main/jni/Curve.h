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
 
#ifndef ANDROIDAPP_CURVE_H
#define ANDROIDAPP_CURVE_H

#include "DrawableObject.h"
#include <string>
#include <GLES2/gl2.h>

#include <vector>
#include "Vec2.h"
#include "../res/Common/DataFormat/CircularBuffer.h"

// variables for curve remaining available space on X axis
const int OK_REMAINS = -2;

class Curve: public DrawableObject{
    public:
        Curve();

        virtual void init(AAssetManager *assetManager);
        virtual void glInit();
        virtual void draw();
        virtual void contextResized(int w, int h);

        void setPosition(int x, int y);
        void setScale(float x, float y);

        static const float POINT_INVALID;

        void setLength(int lengthInPixels);
        int put(float *data, int n);
        //void put_rhythm(float *data, int n);
        //float getEndCoordinateX();

        const Vec2 <int> endpointCoordinates();

        void resetCurrWritePos();

private:
        std::string fragmentShader;
        std::string vertexShader;

        Vec2 <GLfloat> screenSize;
        Vec2 <GLfloat> scale;
        Vec2 <GLfloat> position;

        GLfloat color[3];

        GLfloat clearWidthInPoints;

        static std::vector<GLfloat> invalidBuffer;


        CircularBuffer<GLfloat, 12800, true> newPointBuffer;

        int shaderId;
        GLuint shader_a_Position;
        GLuint shader_screenSize;
        GLuint shader_position;
        GLuint shader_scale;
        GLuint shader_color;
        GLuint shader_a_Value;

        GLuint shader_endOffset;
        GLuint shader_pointCount;
        GLuint shader_clearWidth;

        GLuint valueBuffer;

        int requiredNumOfPoints;
        int currNumOfPoints;

        int currWritePos;
        int lengthInPixels;

        Vec2<float> endCoordinates;

        void clear();
        void resizeOnGPU();
        void moveNewDataToGPU();

        static GLuint getXCoordinates(int capacity);
        static GLfloat *xCoordinates;
        static int xCoordinatesLength;
        static GLuint xCoordinatesOnGPU;

        int reset;
};

#endif //ANDROIDAPP_CURVE_H
