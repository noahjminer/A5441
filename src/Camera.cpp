#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "MatrixStack.h"

using namespace std;

Camera::Camera() :
	aspect(1.0f),
	fovy((float)(45.0*M_PI/180.0)),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(5.0f, 1.0f, -5.0f),
	movementvector(0.0f, 0.0f, 0.0f),
	position(5.0f, 1.0f, -5.0f),
	forward(0.0f, 0.0f, 0.0f),
	up(0.0f, 1.0f, 0.0f),
	lookpoint(1.0f),
	rfactor(0.01f),
	tfactor(0.001f),
	sfactor(0.005f),
	speed(0.01f)
{
}

Camera::~Camera()
{
}

glm::vec3 Camera::getForward() {
	return forward;
}

glm::vec3 Camera::getPos() {
	return position;
}

float Camera::getFovy() {
	return fovy;
}

void Camera::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt)
{
	mousePrev.x = x;
	mousePrev.y = y;
	if(shift) {
		state = Camera::TRANSLATE;
	} else if(ctrl) {
		state = Camera::SCALE;
	} else {
		state = Camera::ROTATE;
	}
}

void Camera::move(glm::vec3 trans) {
	movementvector = trans * speed;
}

void Camera::zoom(bool in) {
	if (in) {
		fovy -= .01f;
	} else {
		fovy += .01f;
	}

	if (fovy < .07f) {
		fovy = .07f;
	} else if (fovy >= 1.98f) {
		fovy = 1.98f;
	}
} 

void Camera::mouseMoved(float x, float y)
{
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;
	rotations += rfactor * dv;
	mousePrev = mouseCurr;
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P, bool hud) const
{
	// Modify provided MatrixStack
	if (!hud) {
		P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
	} else {
		P->multMatrix(glm::perspective((float)(45.0*M_PI/180.0), aspect, znear, zfar));
	}
}

void Camera::applyOrthoMatrix(std::shared_ptr<MatrixStack> P, int width, int height) const {
	P->multMatrix(glm::ortho(float(-width/height), float(width/height), -1.0f, 1.0f, 0.0f, 1000.0f));
}

void Camera::updatePos(glm::vec3 pos) {
	position.x += pos.x;
	position.y += pos.y;
	position.z += pos.z;
}

glm::mat4 Camera::getLookAt() {
	return lookpoint;
} 

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV)
{
	forward.x = sinf(rotations.x);
	forward.z = cosf(rotations.x);
	forward.y = sinf(rotations.y);
	
	if (forward.y > 1.04) {
		forward.y = 1.04;
	} else if (forward.y < -1.04) {
		forward.y = -1.04;
	}

	lookpoint = glm::lookAt(position, 
		glm::vec3(position.x + forward.x, position.y + forward.y, position.z + forward.z), glm::vec3(0, 1, 0));

	MV->multMatrix(lookpoint);
	glm::vec3 forward_vec(movementvector.z * forward.x, 0, movementvector.z * forward.z);
	glm::vec3 right_vec = movementvector.x * glm::cross(forward,up);
	right_vec.y = 0;
	MV->translate(forward_vec);
	MV->translate(right_vec);
	updatePos(forward_vec);
	updatePos(right_vec);
	
}
