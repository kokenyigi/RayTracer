#pragma once

#ifndef APP_H
#define APP_H

#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <CL/cl.h>

//#include <iostream>

#include "Utils.h"

#include "Shader.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Camera.h"
#include "Texture.h"

#include "VirtualWorld.h"


class App
{
private:
	//Window data
	GLFWwindow* m_window;
	int m_windowHeight;
	int m_windowWidth;

	bool isWindowMinimized;

	//Game Data

	float deltaTime = 0.0f;
	float lastFrameTime = 0.0f;
	float timeSinceStart = 0.0f;


	//ECS Ecs;



	
	//Camera m_camera;

	//Shaders



	//uniforms
	//glm::mat4 projectionTransform;
	//glm::mat4 viewTransform;



	//GUI
	//GUI Gui;




	//Helper variables
	
	bool is_free_cam = false;
	bool is_mouse_first_pos = true;
	float last_mouse_x = 0.0f;
	float last_mouse_y = 0.0f;

	//bool showGui = true;

	//int rollAmounts[7];
	//std::string rollAmountsStrings[7];

	//std::string rollSumAmountString;

	//opengl stuff
	Camera camera;
	VirtualWorld virtualWorld;
	

public:
	App(int windowWidth = 1000, int windowHeight = 800, const char* windowTitle = "_debugTitle");

	~App();
	void Run();

	static void WindowSizeCallback(GLFWwindow* window, int width, int height);
	static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	/*
	
	
	
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	
	static void WindowIconifiedCallback(GLFWwindow* window, int isIconified);
	*/

	/*
	template<int index, bool isIncrement>
	static void GuiIncrementDecrementCallback(void* context);

	static void GuiRollEnquedDiceCallback(void* context);
	*/

private:
	void GlfwInit(int windowWidth, int windowHeight, const char* windowTitle);
	void GlewInit(int windowWidth, int windowHeight);
	void Update();
	void Render();



};


#endif