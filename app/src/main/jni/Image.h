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
 
#ifndef ANDROIDAPP_IMAGE_H
#define ANDROIDAPP_IMAGE_H

#include <cstdint>
#include <vector>
#include "Rect.h"

class Image {
    public:
        Image() {}

        struct Pixel{
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;

            Pixel();
            Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
        } __attribute__((packed));

        static const Pixel BLACK;
        static const Pixel WHITE;
        static const Pixel RED;
        static const Pixel GREY;
        static const Pixel TRANSPARENT;

        void resize(int w, int h);
        void *getData();

        void fill(const Pixel &color);

        Pixel &operator()(int x, int y);
        void hLine(int y, int xStart, int xEnd, const Pixel &color, int width=1);
        void vLine(int x, int yStart, int yEnd, const Pixel &color, int width=1);

        void fill(int xStart, int yStart, int width, int height, const Pixel &color);
        void fill(const Rect &rect, const Pixel &color);

        int width();
        int height();

        void setBitmap(const char* bitmap);
    private:
        int w;
        int h;
        std::vector <Pixel> data;
};


#endif //ANDROIDAPP_IMAGE_H
