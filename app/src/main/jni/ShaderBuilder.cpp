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
#include "ShaderBuilder.h"
#include "Helper.h"

ShaderBuilder::ShaderBuilder(){
    reset();
}

ShaderBuilder& ShaderBuilder::instance(){
    static ShaderBuilder i;
    return i;
}

int ShaderBuilder::buildShader(const std::string &name, const std::string &vert, const std::string &frag){
    NameMap::iterator it = nameMap.find(name);
    if (it!=nameMap.end())
        return it->second;

    shaders.push_back(helper::createGlProgram(vert, frag));
    int shaderIndex=shaders.size()-1;
    nameMap[name]=shaderIndex;

    return shaderIndex;
}

GLint ShaderBuilder::useProgram(int shaderId){
    GLint shader=shaders[shaderId];
    if (currShader!=shader) {
        glUseProgram(shader);
        currShader=shader;
    }
    return shader;
}

GLint ShaderBuilder::getShader(int shaderId){
    return shaders[shaderId];
}

void ShaderBuilder::reset(){
    nameMap.clear();
    shaders.clear();
    currShader=-1;
}
