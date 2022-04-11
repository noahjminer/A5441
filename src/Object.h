#ifndef OBJECT_H
#define OBJECT_H

#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "GLSL.h"

#include <iostream> 
#include <vector>
#include <cmath>
#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

using namespace std;

class Object {
    public: 
        Object(glm::vec3 translate, bool rotate, bool scale, bool shear, glm::vec3 ke, glm::vec3 kd, glm::vec3 ks, float s, shared_ptr<Shape> shape, float initScaleDown); 
        ~Object();
        void render(shared_ptr<MatrixStack> MV, shared_ptr<Program> prog);
    private: 
        glm::vec3 translate;
        bool rotate;
        bool scale; 
        bool shear;
        glm::vec3 ke;
        glm::vec3 ks;
        glm::vec3 kd;
        float scaleMin;
        float s;
        float initScaleDown;
        float tOffset;
        shared_ptr<Shape> shape;
};

#endif