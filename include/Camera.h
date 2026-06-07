#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <fstream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // transzformációkhoz
#include <glm/gtc/type_ptr.hpp>   

#include <vector>

class Camera
{

public:

private:
	float m_yaw = -90.f;
	float m_pitch = 0.0f;
	float m_roll = 0.0f;

	float m_fovy = 0.0f;
	float m_fovx = 45.0f;

	float sensitivity = 0.1f;

	float velocity = 2.5f;
	float speedyVelocity = 10.0f;

	float zNear = 0.1f;
	float zFar = 1000.0f;

	float aspect;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 m_worldUp;

	glm::vec3 u;
	glm::vec3 v;
	glm::vec3 w;



public:
	Camera();
	~Camera();

	void Init(const glm::vec3& position, const glm::vec3& at, const glm::vec3& worldUp,
			  int windowWidth,int windowHeight);

	void SetYaw(float yaw);
	float GetYaw() const { return m_yaw; }
	void SetPitch(float pitch);
	float GetPitch() const { return m_pitch; }
	void SetRoll(float roll);
	float GetRoll() const { return m_roll; }

	void SetFovx(float fovx);
	float GetFovx() const { return m_fovx; }

	void Resize(int windowWidth, int windowHeight);
	float GetAspect() const { return aspect; }

	float GetZNear()const { return zNear; }
	float GetZFar()const { return zFar; }

	glm::vec3 GetPosition();

	glm::vec3 GetFront()const { return w; }
	glm::vec3 GetUp()const { return v; }
	glm::vec3 GetRight()const { return u; }

	void Move(float deltatime, glm::vec3 directions,bool isSpeedy);
	void Rotate(float dx, float dy);

	glm::mat4 GetViewMatrix() const { return glm::lookAt(position, position + front, m_worldUp); }
	glm::mat4 GetPerspectiveMatrix() const 
	{ 
		return glm::perspective(2.0f*atanf( tanf( glm::radians( m_fovx/2.0f ) ) * 1.0f / aspect ), aspect, zNear, zFar); 
	}
	//..^^have to calculate fovy from fovx using math
private:
	void CalculateUVW();
	void CalculateFront();
};

#endif