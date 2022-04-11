#pragma  once
#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class MatrixStack;

class Camera
{
public:
	enum {
		ROTATE = 0,
		TRANSLATE,
		SCALE
	};
	
	Camera();
	virtual ~Camera();
	void setInitDistance(float z) { translations.z = -std::abs(z); }
	void setAspect(float a) { aspect = a; };
	void setRotationFactor(float f) { rfactor = f; };
	void setTranslationFactor(float f) { tfactor = f; };
	void setScaleFactor(float f) { sfactor = f; };
	void mouseClicked(float x, float y, bool shift, bool ctrl, bool alt);
	void move(glm::vec3 trans);
	void mouseMoved(float x, float y);
	void applyProjectionMatrix(std::shared_ptr<MatrixStack> P, bool hud) const;
	void applyOrthoMatrix(std::shared_ptr<MatrixStack> P, int width, int height) const;
	void applyViewMatrix(std::shared_ptr<MatrixStack> MV);
	void updatePos(glm::vec3 pos);
	void zoom(bool in);
	glm::vec3 getForward();
	glm::vec3 getPos();
	glm::mat4 getLookAt();
	float getFovy();
	
private:
	float aspect;
	float fovy;
	float znear;
	float zfar;
	glm::vec2 rotations;
	glm::vec3 movementvector;
	glm::vec3 translations;
	glm::vec2 mousePrev;
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	glm::mat4 lookpoint;
	int state;
	float rfactor;
	float tfactor;
	float sfactor;
	float speed;
};

#endif
