#include "Object.h"
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
#include <glm/gtc/type_ptr.hpp>

using namespace std;

Object::Object(glm::vec3 translate, bool rotate, bool scale, bool shear,
    glm::vec3 ke, glm::vec3 kd, glm::vec3 ks, float s, shared_ptr<Shape> shape, float initScaleDown) {
    this->translate = translate;
    this->rotate = rotate;
    this->scale = scale;
    this->shear = shear;
    this->shape = shape;
    this->ke = ke;
	this->kd = kd;
	this->ks = ks;
    this->s = s;
    this->initScaleDown = initScaleDown;
    this->scaleMin = ((rand() + 2) % 100) / 300.0f;
    this->tOffset = rand() % 1000;
}

Object::~Object() {

}

void Object::render(shared_ptr<MatrixStack> MV, shared_ptr<Program> prog) {
    float scaleSpeed = 0.1f;
    float t = glfwGetTime() + tOffset;
    float timeScale = abs(sin(t * scaleSpeed));
    if (timeScale < scaleMin) {
        timeScale = scaleMin;
    }
    //translate.y = -(timeScale * shape->getMin() / 2);
    MV->pushMatrix();
    MV->translate(translate);
    if (rotate)
        MV->rotate(t, glm::vec3(0,1,0));
    MV->scale(initScaleDown);
    if (scale)
        MV->scale(timeScale);
    float scaleFac = -1 * (shape->getMin() - .005f);
    MV->translate(glm::vec3(0, scaleFac, 0));
    if (shear) {
        glm::mat4 S(1.0f);
        S[1][2] = 0.5f*cos(t);
        MV->multMatrix(S);
    }
    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
    glUniform3f(prog->getUniform("ke"), this->ke.x, this->ke.y, this->ke.z);
    glUniform3f(prog->getUniform("kd"), this->kd.x, this->kd.y, this->kd.z);    
    glUniform3f(prog->getUniform("ks"), this->ks.x, this->ks.y, this->ks.z);
    glUniform1f(prog->getUniform("s"), s);
    this->shape->draw(prog);
    MV->popMatrix();
}