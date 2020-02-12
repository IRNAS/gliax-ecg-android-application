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

#include "Circle.h"
#include "Helper.h"
#include "EcgArea.h"
#include "ShaderBuilder.h"
#include <cassert>

const GLfloat Circle::vertexCoordinates[8]={
        -0.5f, -0.5f,
        0.5f, -0.5f,
        -0.5f,  0.5f,
        0.5f,  0.5f
};


Circle::Circle(){
    position[0]=0;
    position[1]=0;
    color[0]=1;
    color[1]=0;
    color[2]=0;
    size=10;
}

void Circle::init(AAssetManager *assetManager){
    vertexShader = helper::loadAsset(assetManager, "circle.vert");
    fragmentShader = helper::loadAsset(assetManager, "circle.frag");
}

void Circle::glInit(){
    shaderId = ShaderBuilder::instance().buildShader("Circle", vertexShader, fragmentShader);
    GLuint shaderProgram = ShaderBuilder::instance().getShader(shaderId);

    shader_a_Position=helper::getGlAttributeWithAssert(shaderProgram, "a_Position");
    shader_screenSize=helper::getGlUniformWithAssert(shaderProgram, "screenSize");
    shader_position=helper::getGlUniformWithAssert(shaderProgram, "position");
    shader_size=helper::getGlUniformWithAssert(shaderProgram, "size");
    shader_color=helper::getGlUniformWithAssert(shaderProgram, "color");

    glGenBuffers(1, &vertexBuffer);

    initGlBuffers();
}

void Circle::initGlBuffers(){
    glBindBuffer(GL_ARRAY_BUFFER , vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER , sizeof(vertexCoordinates), vertexCoordinates, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER , 0);
}

void Circle::draw(){
    if (EcgArea::instance().isRedrawNeeded()){
        initGlBuffers();
    }

    ShaderBuilder::instance().useProgram(shaderId);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(shader_a_Position, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(shader_a_Position);
    glUniform2f(shader_screenSize, screenSize[0],screenSize[1]);
    glUniform3f(shader_position, position[0], position[1], zCoordinate);
    glUniform1f(shader_size, (GLfloat)size);
    glUniform3f(shader_color, color[0], color[1], color[2]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER , 0);
}

void Circle::contextResized(int w, int h){
    screenSize[0]=w;
    screenSize[1]=h;
}

void Circle::setPosition(const Vec2<int> &pos){
    position[0]=pos.x;
    position[1]=pos.y;
}


void Circle::setSize(float d){
    size=d;
}

void Circle::setColor(float r, float g, float b){
    color[0]=r;
    color[1]=g;
    color[2]=b;
}