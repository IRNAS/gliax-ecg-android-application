//
// Created by vidra on 18. 03. 2019.
//

#include "Rectangle.h"
#include "EcgArea.h"
#include "ShaderBuilder.h"
#include "Helper.h"
#include <cassert>
#include "log.h"

const GLfloat Rectangle::vertexCoordinates[8] = {
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        8.0f, -1.0f,
        8.0f, 1.0f,
};

Rectangle::Rectangle() {
    position[0] = 0;
    position[1] = 0;
    color[0] = 0;
    color[1] = 0;
    color[2] = 1;
    size = 10;
}

void Rectangle::init(AAssetManager *assetManager) {
    vertexShader = helper::loadAsset(assetManager, "rectangle.vert");
    fragmentShader = helper::loadAsset(assetManager, "rectangle.frag");
}

void Rectangle::glInit(){
    shaderId = ShaderBuilder::instance().buildShader("Rectangle", vertexShader, fragmentShader);
    GLuint shaderProgram = ShaderBuilder::instance().getShader(shaderId);

    shader_a_Position=helper::getGlAttributeWithAssert(shaderProgram, "a_Position");
    shader_screenSize=helper::getGlUniformWithAssert(shaderProgram, "screenSize");
    shader_position=helper::getGlUniformWithAssert(shaderProgram, "position");
    shader_size=helper::getGlUniformWithAssert(shaderProgram, "size");
    shader_color=helper::getGlUniformWithAssert(shaderProgram, "color");

    glGenBuffers(1, &vertexBuffer);

    initGlBuffers();
}

void Rectangle::initGlBuffers(){
    glBindBuffer(GL_ARRAY_BUFFER , vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER , sizeof(vertexCoordinates), vertexCoordinates, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER , 0);
}

void Rectangle::draw(){
    if (EcgArea::instance().isRedrawNeeded()){
        initGlBuffers();
    }

    ShaderBuilder::instance().useProgram(shaderId);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(shader_a_Position, 2, GL_FLOAT, GL_FALSE, 0, 0);  // original
    //glVertexAttribPointer(shader_a_Position, 3, GL_FLOAT, GL_FALSE, 4*sizeof(vertexCoordinates), 0);

    glEnableVertexAttribArray(shader_a_Position);
    glUniform2f(shader_screenSize, screenSize[0],screenSize[1]);
    glUniform3f(shader_position, position[0], position[1], zCoordinate);
    glUniform1f(shader_size, (GLfloat)size);
    glUniform3f(shader_color, color[0], color[1], color[2]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER , 0);
}

void Rectangle::contextResized(int w, int h){
    screenSize[0]=w;
    screenSize[1]=h;
}

void Rectangle::setPosition(int x, int y){
    position[0] = x;
    position[1] = y;
}

void Rectangle::setSize(float d){
    size=d;
}

void Rectangle::setColor(float r, float g, float b){
    color[0]=r;
    color[1]=g;
    color[2]=b;
}

int* Rectangle::getSize() {
    LOGI("Position: X: %f Y: %f", position[0], position[1]);
    int * size_coords;
    size_coords[0] = size * 9;  // 9 -> vertex coords x distance (-1 to 8)
    size_coords[1] = size * 2;  // 2 -> vertex coords y distance (-1 to +1)
    LOGI("Size: X: %d Y: %d", size_coords[0], size_coords[1]);
    return size_coords;
}

