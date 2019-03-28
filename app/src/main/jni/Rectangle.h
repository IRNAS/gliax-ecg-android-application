//
// Created by vidra on 18. 03. 2019.
//

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
