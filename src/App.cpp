#include "App.h"
#include "Debug.h"

#include <CL/cl_gl.h>
#include <GL/wglew.h>

#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

App::App(int windowWidth, int windowHeight, const char* windowTitle)
{
	//std::cout << " App constructor" << std::endl;
	//Mandatorily intialization of glfw and glew

	GlfwInit(windowWidth, windowHeight, windowTitle);
	GlewInit(windowWidth, windowHeight);




	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 330");



	camera.Init(glm::vec3(0, 2, 3), glm::vec3(0,0,0), glm::vec3(0, 1, 0),windowWidth,windowHeight);


	//Get some models going
	int robotTextureIndex = virtualWorld.LoadTexture("res/textures/Robot_Texture.png");
	int robotMaterialIndex = virtualWorld.AddMaterial(robotTextureIndex, 1.0f, 0.1f, false,1.45f,0.7f);
	int robotMeshIndex = virtualWorld.LoadMesh("res/assets/head.obj", robotMaterialIndex);
	virtualWorld.AddObject(robotMeshIndex, glm::vec3(15,0,0),glm::vec3(0,0,0),glm::vec3(1,1,1));

	int suzanneTextureIndex = virtualWorld.LoadTexture("res/textures/wood.jpg");
	int suzanneMaterialIndex = virtualWorld.AddMaterial(suzanneTextureIndex, 1.0f, 0.1f,false, 1.45f, 0.7f);
	int suzanneMeshIndex = virtualWorld.LoadMesh("res/assets/Suzanne.obj", suzanneMaterialIndex);
	virtualWorld.AddObject(suzanneMeshIndex, glm::vec3(0, 0, 12), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

	int ballTextureIndex = virtualWorld.LoadTexture("res/textures/MarbleBall.png");
	int ballMaterialIndex = virtualWorld.AddMaterial(ballTextureIndex, 0.0f, 0.1f, true, 1.45f, 0.7f);
	int balltMeshIndex = virtualWorld.LoadMesh("res/assets/MarbleBall.obj", ballMaterialIndex);
	virtualWorld.AddObject(balltMeshIndex, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

	int houseTextureIndex = virtualWorld.LoadTexture("res/textures/old_house.png");
	int houseMaterialIndex = virtualWorld.AddMaterial(houseTextureIndex, 1.0f, 0.1f, false, 1.45f, 0.7f);
	int houseMeshIndex = virtualWorld.LoadMesh("res/assets/old_house.obj", houseMaterialIndex);
	virtualWorld.AddObject(houseMeshIndex, glm::vec3(-13, 0, 0), glm::vec3(-90.f, 0, 0), glm::vec3(2, 2,2));

	virtualWorld.Init(windowWidth, windowHeight,&camera);
	
	
}

void App::GlfwInit(int windowWidth, int windowHeight, const char* windowTitle)
{
	if (!glfwInit())
	{
		std::cout << "[GLFW Error]: Initialization of GLFW failed!" << std::endl;
		return;
	}
	else
	{
		std::cout << "Successfully initialized GLFW!" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	m_window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!m_window)
	{
		std::cout << "[GLFW Error]: Window creation failed!" << std::endl;
		glfwTerminate();
		return;
	}
	else
	{
		std::cout << "Window creation successful!" << std::endl;
	}

	glfwSetWindowSizeLimits(m_window, 1000, 800, GLFW_DONT_CARE, GLFW_DONT_CARE);

	//isWindowMinimized = false;


	glfwMakeContextCurrent(m_window);

	glfwSwapInterval(1);

	//Sets the current App context as the window user pointer
	glfwSetWindowUserPointer(m_window, this);

	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetWindowSizeCallback(m_window, App::WindowSizeCallback);
	glfwSetCursorPosCallback(m_window, App::CursorPosCallback);
	glfwSetKeyCallback(m_window, App::KeyCallback);
	glfwSetMouseButtonCallback(m_window, App::MouseButtonCallback);
	//Callbacks
	/*
	glfwSetScrollCallback(m_window, App::ScrollCallback);
	
	
	
	
	glfwSetWindowIconifyCallback(m_window, App::WindowIconifiedCallback);
	*/


	



}

void App::GlewInit(int windowWidth, int windowHeight)
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << ("[GlEW Error]: Initialization of GLEW failed!") << std::endl;
		return;
	}
	else
	{
		std::cout << ("Initialization of GLEW was successful!") << std::endl;
	}

	//Printing the current OpenGL version on the terminal
	std::cout << "Current OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	//glDisable(GL_CULL_FACE);
	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
}

App::~App()
{
	glfwTerminate();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void App::Run()
{
	while (!glfwWindowShouldClose(m_window))
	{

		Update();
		Render();

	}
}

void App::Update()
{
	float currentFrameTime = glfwGetTime();
	deltaTime = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime;

	timeSinceStart += deltaTime;

	
	
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		direction.z += 1.0f;
	}
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		direction.z -= 1.0f;
	}
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		direction.x += 1.0f;
	}
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		direction.x -= 1.0f;
	}
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		direction.y += 1.0f;
	}
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		direction.y -= 1.0f;
	}
	bool isSpeedy = false;

	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		isSpeedy = true;
	}
	

	camera.Move(deltaTime, direction, isSpeedy);

	virtualWorld.Update();
	


	//viewTransform = m_camera.GetViewMatrix();

	//int rollSumNumber;

	//Ecs.Update(deltatime, rollSumNumber);

	//rollSumAmountString = IntegerToString(rollSumNumber);

	//std::cout << rollSumAmountString << std::endl;


	//Gui.Update(deltatime);
	glfwPollEvents();
}

void App::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	virtualWorld.Render(timeSinceStart);

	
	//opengl boiler plate


	//toggling the main base shader


	//Setting up universal shader unifroms

	//std::cout << isWindowMinimized << std::endl;
	/*
	if (!isWindowMinimized)
	{
		projectionTransform = m_camera.GetPerspectiveMatrix((float)m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);


		Ecs.Render(viewTransform, projectionTransform, m_camera.GetPosition(), (float)m_windowWidth, (float)m_windowHeight);

		if (showGui)
		{
			Gui.Render();
		}



		//final backbuffer swap
		
	}
	*/
	auto start = std::chrono::steady_clock::now();

	

	

	glfwSwapBuffers(m_window);

	auto end = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	//std::cout<<"App:" << diff.count() << std::endl;


	
}

void App::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	App* app = (App*)glfwGetWindowUserPointer(window);
	app->m_windowWidth = width;
	app->m_windowHeight = height;

	if (width != 0 && height != 0)
	{
		glViewport(0, 0, width, height);
		app->virtualWorld.Resize(width, height);
		app->camera.Resize(width, height);
	}
}

void App::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	App* app = (App*)glfwGetWindowUserPointer(window);


	if (app->is_free_cam)
	{
		if (app->is_mouse_first_pos)
		{
			app->last_mouse_x = xpos;
			app->last_mouse_y = ypos;
			app->is_mouse_first_pos = false;
			//std::cout << "First mouse pos: (x: " << xpos << " y: " << ypos << " )\n";
		}
		else
		{
			float dx = xpos - app->last_mouse_x;
			float dy = app->last_mouse_y - ypos;

			//std::cout << "Delta mouse pos: (x: " << dx << " y: " << dy << " )\n";

			app->last_mouse_x = xpos;
			app->last_mouse_y = ypos;

			app->camera.Rotate(dx, dy);

		}
	}
	

	


}

void App::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	App* app = (App*)glfwGetWindowUserPointer(window);

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		if (app->is_free_cam == false)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			app->is_free_cam = true;
			app->is_mouse_first_pos = true;

			double x, y;
			glfwGetCursorPos(window, &x, &y);

			app->last_mouse_x = x;
			app->last_mouse_y = y;

			
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			app->is_free_cam = false;
		}


	}

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		app->virtualWorld.SwitchRenderMode();


	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		app->virtualWorld.debugDepth += 1;
	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		app->virtualWorld.debugDepth -= 1;
	}
}

void App::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	App* app = (App*)glfwGetWindowUserPointer(window);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		//app->Ecs.AddEntity(ICOSAHEDRON);

		//app->Ecs.Update(0.05f);
		double x; //= //(int)(app->last_mouse_x + app->m_windowWidth);
		double y;// = (int)(app->last_mouse_y + app->m_windowHeight);
		glfwGetCursorPos(window, &x, &y);
		//std::cout << "Picking at: (x :"<< x<< " y: " << y<< ")\n";
		app->virtualWorld.Pick((int)x, (int)y);

	}
}