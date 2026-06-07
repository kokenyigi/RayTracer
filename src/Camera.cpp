#include "Camera.h"
#include <iostream>

Camera::Camera()
{
	//std::cout << "Camera" << std::endl;
	

	
}

void Camera::Init(const glm::vec3& position, const glm::vec3& at, const glm::vec3& worldUp,
				  int windowWidth, int windowHeight)
{
	this->position = position;
	front = glm::normalize(at - position);
	m_worldUp = worldUp;

	aspect = (float)windowWidth / (float)windowHeight;

	CalculateFront();
}

void Camera::Move(float deltatime, glm::vec3 directions,bool isSpeedy)
{
	float deltaspeed = velocity * deltatime;
	if (isSpeedy)
	{
		deltaspeed = speedyVelocity * deltatime;
	}

	

	position += u * deltaspeed * directions.x;
	position += v * deltaspeed * directions.y;
	position += w * deltaspeed * directions.z;

	//No extra calculation needed
}

void Camera::Rotate(float dx, float dy)
{
	SetYaw(m_yaw + dx * sensitivity);
	SetPitch(m_pitch + dy * sensitivity);

	CalculateFront();//Caluclates uvw too
}

Camera::~Camera()
{
	//implement
}

void Camera::SetFovx(float fovx)
{
	if (fovx < 1.0f)
	{
		m_fovx = 1.0f;
	}
	else if (fovx > 45.0f)
	{
		m_fovx = 45.0f;
	}
	else
	{
		m_fovx = fovx;
	}
}

void Camera::SetYaw(float yaw)
{
	m_yaw = yaw;
}

glm::vec3 Camera::GetPosition()
{
	return this->position;
}

void Camera::SetPitch(float pitch)
{
	if (pitch >= 89.5f)
	{
		m_pitch = 89.5f;
	}
	else if (pitch <= -89.5f)
	{
		m_pitch = -89.5f;
	}
	else
	{
		m_pitch = pitch;
	}
}

void Camera::SetRoll(float roll)
{
	m_roll = roll;
}

void Camera::Resize(int windowWidth, int windowHeight)
{
	aspect = (float)windowWidth / (float)windowHeight;
}

void Camera::CalculateUVW()
{
	w = front;
	u = glm::normalize(glm::cross(m_worldUp,w));
	v = glm::cross(w, u);
}

void Camera::CalculateFront()
{
	glm::vec3 new_front;
	new_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	new_front.y = sin(glm::radians(m_pitch));
	new_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	front = glm::normalize(new_front);

	CalculateUVW();
}