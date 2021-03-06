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

#ifndef ANDROIDAPP__RECTANGLE_H
#define ANDROIDAPP__RECTANGLE_H

#include <android/asset_manager_jni.h>
#include <string>
#include "DrawableObject.h"
#include "Vec2.h"
#include <GLES2/gl2.h>

class Rectangle: public DrawableObject {
    public:
        Rectangle();
        virtual void init(AAssetManager *assetManager);
        virtual void glInit();
        virtual void draw();
        virtual void contextResized(int w, int h);

        void setPosition(int x, int y);
        void setSize(float d);

        void setColor(float r, float g, float b);

        int* getSize();

    private:
        std::string vertexShader;
        std::string fragmentShader;

        void initGlBuffers();

        static const GLfloat vertexCoordinates[8];
        int shaderId;

        GLuint shader_a_Position;
        GLuint shader_screenSize;
        GLuint shader_position;
        GLuint shader_size;
        GLuint shader_color;
        GLuint vertexBuffer;

        GLfloat screenSize[2];
        GLfloat position[2];
        GLfloat color[3];

        float size;
};

#endif //ANDROIDAPP__RECTANGLE_H
