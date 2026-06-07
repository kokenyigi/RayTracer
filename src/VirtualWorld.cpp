#include "VirtualWorld.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <sstream>
#include <unordered_map>
#include <chrono>

#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


VirtualWorld::VirtualWorld()
{
	isRayTracingToggled = true;


	entityCapacity = 20;
	entityCount = 0;

	transforms = new Transform[entityCapacity];
	worldTransforms = new WorldTransform[entityCapacity];
	renders = new RenderData[entityCapacity];

	rasterScene.objects.resize(entityCapacity);
	scene.objectDatas.resize(entityCapacity);
}

VirtualWorld::~VirtualWorld()
{
	//Should delete shi
}

int VirtualWorld::LoadTexture(const std::string& filePath)
{
	/*
	Mesh<VertexP3N3T2> newMesh;
	newMesh.Load(filePath);

	LoadRayTracedMesh(filePath);
	*/


	Texture newTexture;
	newTexture.Init(filePath);

	rasterScene.textures.push_back(newTexture);

	int index = LoadRayTracedTexture(filePath);

	return index;
}

int VirtualWorld::AddMaterial(int textureIndex, float reflectivity, float roughness, 
							  bool isRefracting, float refractIndex, float refractability)
{
	Material newMaterial;
	newMaterial.textureIndex = textureIndex;
	
	rasterScene.materials.push_back(newMaterial);

	int index = AddRayTracedMaterial(textureIndex, reflectivity, roughness,  isRefracting,  refractIndex,  refractability);
	return index;
}

int VirtualWorld::LoadMesh(const std::string& filePath, int materialIndex)
{

	Mesh<VertexP3N3T2> newMesh;
	newMesh.Load(filePath);

	rasterScene.meshes.push_back(newMesh);
	int meshIndex = rasterScene.meshes.size()-1;

	Model newModel;
	newModel.meshIndex = meshIndex;
	newModel.materialIndex = materialIndex;
	rasterScene.models.push_back(newModel);

	int index = LoadRayTracedMesh(filePath,materialIndex);


	return index;
}


void VirtualWorld::AddObject(int modelIndex, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& size)
{
	/*
	Object newObject;
	newObject.modelIndex = meshIndex;
	newObject.worldTransform = worldTransform;
	newObject.invWorldTransform = glm::inverse(worldTransform);

	rasterScene.objects.push_back(newObject);

	AddRayTracedObject(meshIndex, worldTransform);
	*/
	if (entityCount < entityCapacity)
	{
		transforms[entityCount].position = pos;
		transforms[entityCount].rotation = rot;
		transforms[entityCount].size = size;

		renders[entityCount].rasterModelIndex = modelIndex;
		renders[entityCount].raytracedModelIndex = modelIndex;

		++entityCount;
	}
	
}





void VirtualWorld::Init(int displayWidth, int displayHeight, Camera* camera)
{
	this->displayWidth = displayWidth;
	this->displayHeight = displayHeight;

	this->camera = camera;

	//Opengl init stuff

	rasterizerShader.Init("res/shaders/rasterizer_vertex_shader.vert", "res/shaders/rasterizer_fragment_shader.frag");
	//testTexture.Init("res/textures/old_house.png");
	//testMesh.Load("res/assets/old_house.obj");

	debugShader.Init("res/shaders/debug_vertex_shader.vert", "res/shaders/debug_fragment_shader.frag");
	std::vector<VertexP3> debugBoxVertices =
	{
		{glm::vec3(0.5,0.5,0.5)},
		{glm::vec3(0.5,0.5,-0.5)},
		{glm::vec3(-0.5,0.5,-0.5)},
		{glm::vec3(-0.5,0.5,0.5)},

		{glm::vec3(0.5,-0.5,0.5)},
		{glm::vec3(0.5,-0.5,-0.5)},
		{glm::vec3(-0.5,-0.5,-0.5)},
		{glm::vec3(-0.5,-0.5,0.5)},
	};

	std::vector<unsigned int> debugBoxIndices =
	{
		0,1,
		1,2,
		2,3,
		3,0,

		0,4,
		1,5,
		2,6,
		3,7,

		4,5,
		5,6,
		6,7,
		7,4
	};

	debugBoxMesh.Init(debugBoxVertices, debugBoxIndices);

	glLineWidth(3.0f);






	//Raytracer init stuff
	std::vector<VertexP2T2> quadVertices =
	{
		{glm::vec2(-1,-1),glm::vec2(0,0)},
		{glm::vec2(1,-1),glm::vec2(1,0)},
		{glm::vec2(1,1),glm::vec2(1,1)},
		{glm::vec2(-1,1),glm::vec2(0,1)}
	};
	std::vector<unsigned int> quadIndices =
	{
		0,1,2,
		2,3,0
	};

	quadMesh.Init(quadVertices, quadIndices);

	quadShader.Init("res/shaders/vertex_shader.vert", "res/shaders/fragment_shader.frag");



	

	//testTexture.Set("res/textures/boxTexture.png");


	cl_int err;

	// 1. Platform
	cl_platform_id platform;
	err = clGetPlatformIDs(1, &platform, nullptr);
	CHECK_ERROR(err);


	unsigned int deviceAmount;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, nullptr, &deviceAmount);
	CHECK_ERROR(err);

	cl_device_id* devices = new cl_device_id[deviceAmount];
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, devices, nullptr);
	CHECK_ERROR(err);

	for (int i = 0;i < deviceAmount;++i)
	{
		unsigned long memSize;
		err = clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(unsigned long), &memSize, nullptr);

		unsigned int computeCores;
		err = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(unsigned int), &computeCores, nullptr);

		char nameBuffer[200];
		err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(char) * 200, nameBuffer, nullptr);

		std::cout << memSize << "\n";
		std::cout << computeCores << "\n";
		std::cout << nameBuffer << '\n';
	}



	cl_device_id device = devices[0]; //For now, later on its better to use the best device available

	delete[] devices;

	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR,   (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR,      (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		0
	};

	context =
		clCreateContext(props, 1, &device, nullptr, nullptr, &err);

	queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
	CHECK_ERROR(err);


	/*
	const char* kernelSource = R"CLC(
        __kernel void render(write_only image2d_t output,
                     int width,
                     int height)
		{
			int2 coord = (int2)(get_global_id(0),
								get_global_id(1));

			int2 localCoord = (int2)(get_local_id(0),get_local_id(1));

			if(coord.x >= width || coord.y >= height)
				return;

			float2 uv = (float2)(
				(localCoord.x + 0.5f) / 16 * (coord.x + 0.5f) / width,
				(localCoord.y + 0.5f) / 16 * (coord.y + 0.5f) / height
			);

			float4 color = (float4)(uv.x, uv.y, 0.2f, 1.0f);

			write_imagef(output, coord, color);
		}
    )CLC";

	*/

	const char* kernelPath = "res/kernels/render_kernel.cl";
	std::string kernelSource = "";

	std::ifstream file(kernelPath);

	std::string linebuffer;
	while (std::getline(file, linebuffer))
	{
		kernelSource.append(linebuffer + '\n');
	}

	file.close();

	const char* kernelSourceC = kernelSource.c_str();
	//std::cout << kernelSourceC << '\n';
	// 7. Program és fordítás
	cl_program program = clCreateProgramWithSource(context, 1,&kernelSourceC, nullptr, &err);
	CHECK_ERROR(err);

	err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
	if (err != CL_SUCCESS) {
		size_t log_size;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
		std::vector<char> log(log_size);
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
		std::cerr << "Build log:\n" << log.data() << std::endl;
	}

	kernel = clCreateKernel(program, "render", &err);
	CHECK_ERROR(err);

	




	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA32F,
		displayWidth,
		displayHeight,
		0,
		GL_RGBA,
		GL_FLOAT,
		nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	clImage =
		clCreateFromGLTexture(context,
			CL_MEM_WRITE_ONLY,
			GL_TEXTURE_2D,
			0,          
			texId,
			&err);

	CHECK_ERROR(err);


	cameraBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(CameraData),
		nullptr,
		&err);

	CHECK_ERROR(err);

	//Load a .obj into VRAM
	scene.bvhMaxDepth = 30;

	/*
	int robotTextureIndex = LoadRayTracedTexture("res/textures/Robot_Texture.png");
	int robotMaterialIndex = AddRayTracedMaterial(robotTextureIndex, 1.0f, 0.1f);
	int robotMeshIndex = LoadRayTracedMesh("res/assets/head.obj", robotMaterialIndex);
	//LoadMesh("res/assets/old_house.obj", 1.0f, 0.1f);
	*/

	std::cout << scene.bvhNodes.size() <<"\n";

	
	//int childCount = 0;
	//int triagCount = 0;
	
	for (int i = 0;i < scene.bvhNodes.size();++i)
	{
		if ((scene.bvhNodes[i].minChild == -1 && scene.bvhNodes[i].maxChild != -1) || 
			(scene.bvhNodes[i].minChild != -1 && scene.bvhNodes[i].maxChild == -1))
		{
			std::cout << "Bad node" << "/n";
			//std::cout << scene.bvhNodes[i].endIndex - scene.bvhNodes[i].startIndex << "\n";
			//triagCount += scene.bvhNodes[i].endIndex - scene.bvhNodes[i].startIndex;
			//++childCount;
		}
	}
	
	//std::cout << "Avarage: " << (float)triagCount / childCount << "\n";
	

	//AddObject(robotMeshIndex, glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-15)));

	/*
	AddObject(1, glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)));
	*/
	//std::cout << scene.bvhNodes.size();
	/*
	for (int i = 0;i < scene.bvhNodes.size();++i)
	{
		std::cout << "Node's Left child: " << scene.bvhNodes[i].minChild << "\n";
		std::cout << "Node's Right child: " << scene.bvhNodes[i].maxChild << "\n\n";
	}
	*/

	cl_int error;

	//Lets upload our positions, indices and attributes, and also our whole meshdata into VRAM cuz why tf not
	scene.triangleVertexPositionsBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(cl_float4)*scene.triangleVertexPositions.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.triangleVertexPositionsBuffer,
		CL_TRUE,
		0,
		sizeof(cl_float4)* scene.triangleVertexPositions.size(),
		scene.triangleVertexPositions.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);





	scene.triangleVertexAttributesBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(Attribute) * scene.triangleVertexAttributes.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.triangleVertexAttributesBuffer,
		CL_TRUE,
		0,
		sizeof(Attribute) * scene.triangleVertexAttributes.size(),
		scene.triangleVertexAttributes.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);





	scene.triangleIndicesBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(cl_int3) * scene.triangleIndices.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.triangleIndicesBuffer,
		CL_TRUE,
		0,
		sizeof(cl_int3) * scene.triangleIndices.size(),
		scene.triangleIndices.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);






	scene.bvhNodesBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(BvhNode) * scene.bvhNodes.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.bvhNodesBuffer,
		CL_TRUE,
		0,
		sizeof(BvhNode) * scene.bvhNodes.size(),
		scene.bvhNodes.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);

	scene.rgbaValuesBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(cl_float4) * scene.rgbaValues.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.rgbaValuesBuffer,
		CL_TRUE,
		0,
		sizeof(cl_float4) * scene.rgbaValues.size(),
		scene.rgbaValues.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);



	scene.textureDatasBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(TextureData) * scene.textureDatas.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.textureDatasBuffer,
		CL_TRUE,
		0,
		sizeof(TextureData) * scene.textureDatas.size(),
		scene.textureDatas.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);





	scene.materialDatasBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(MaterialData) * scene.materialDatas.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.materialDatasBuffer,
		CL_TRUE,
		0,
		sizeof(MaterialData) * scene.materialDatas.size(),
		scene.materialDatas.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);





	scene.meshDatasBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(MeshData) * scene.meshDatas.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	error = clEnqueueWriteBuffer(queue,
		scene.meshDatasBuffer,
		CL_TRUE,
		0,
		sizeof(MeshData) * scene.meshDatas.size(),
		scene.meshDatas.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);





	scene.objectDatasBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_ONLY,
		sizeof(ObjectData) * scene.objectDatas.size(),
		nullptr,
		&error);

	CHECK_ERROR(error);

	








	//Init(vertices, indices);
}

void VirtualWorld::Update()
{
	for (int i = 0;i < entityCount;++i)
	{
		worldTransforms[i].matrix = glm::translate(glm::mat4(1.0f), transforms[i].position) *
			glm::rotate(glm::mat4(1.0f),glm::radians(transforms[i].rotation.x),glm::vec3(1,0,0)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(transforms[i].rotation.y), glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(transforms[i].rotation.z), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), transforms[i].size);

		worldTransforms[i].invMatrix = glm::inverse(worldTransforms[i].matrix);

		//Write data out to objectDatas
		//std::cout << "Updating\n";
		scene.objectDatas[i].meshIndex = renders[i].raytracedModelIndex;
		scene.objectDatas[i].worldTransform = worldTransforms[i].matrix;
		scene.objectDatas[i].invWorldTransform = worldTransforms[i].invMatrix;

		rasterScene.objects[i].modelIndex = renders[i].rasterModelIndex;
		rasterScene.objects[i].worldTransform = worldTransforms[i].matrix;
		rasterScene.objects[i].invWorldTransform = worldTransforms[i].invMatrix;
	}

}

void VirtualWorld::Render(float timePassed)
{

	
	auto start = std::chrono::steady_clock::now();

	if (isRayTracingToggled)
	{
		RayTracedRender(timePassed);
	}
	else
	{
		OpenGLRender(timePassed);
	}

	auto end = std::chrono::steady_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	renderTime = diff.count();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();


	ImGui::Begin("Interface");

	//ImGui::Text("Hello world!");
	/*
	static float value = 0.5f;
	ImGui::SliderFloat("Value", &value, 0.0f, 1.0f);

	if (ImGui::Button("Press me"))
	{
		printf("Button pressed\n");
	}
	*/ 
	ImGui::Text("Render FPS: %f", 1000000.0f/renderTime);

	if (pickedEntityIndex >= 0 && pickedEntityIndex < entityCount)
	{
		ImGui::Separator();
		int i = pickedEntityIndex;
		ImGui::InputFloat("Position x: ", &transforms[i].position.x, -100.0f, 100.0f, "%.3f");
		ImGui::InputFloat("Position y: ", &transforms[i].position.y, -100.0f, 100.0f, "%.3f");
		ImGui::InputFloat("Position z: ", &transforms[i].position.z, -100.0f, 100.0f, "%.3f");
		ImGui::Separator();
		ImGui::SliderFloat("Rotation x: ", &transforms[i].rotation.x, 0.0f, 360.0f);
		ImGui::SliderFloat("Rotation y: ", &transforms[i].rotation.y, 0.0f, 360.0f);
		ImGui::SliderFloat("Rotation z: ", &transforms[i].rotation.z, 0.0f, 360.0f);
		ImGui::Separator();
		ImGui::InputFloat("Size x: ", &transforms[i].size.x, -10.0f, 10.0f, "%.3f");
		ImGui::InputFloat("Size y: ", &transforms[i].size.y, -10.0f, 10.0f, "%.3f");
		ImGui::InputFloat("Size z: ", &transforms[i].size.z, -10.0f, 10.0f, "%.3f");
	}
	
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void VirtualWorld::Resize(int displayWidth, int displayHeight)
{
	this->displayWidth = displayWidth;
	this->displayHeight = displayHeight;

	cl_int err;
	
	err = clReleaseMemObject(clImage);
	CHECK_ERROR(err);

	glDeleteTextures(1, &texId);

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA32F,
		displayWidth,
		displayHeight,
		0,
		GL_RGBA,
		GL_FLOAT,
		nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	

	clImage =
		clCreateFromGLTexture(context,
			CL_MEM_WRITE_ONLY,
			GL_TEXTURE_2D,
			0,          // mip level
			texId,
			&err);

	CHECK_ERROR(err);

	
}

struct Ray
{
	float tMin;
	float tMax;
	glm::vec3 origin;
	glm::vec3 direction;
};

struct TraceResult
{
	float t;
	int entityIndex;
};




TraceResult IntersectBox(const Ray& ray, const AABB& box)
{
	TraceResult retval;
	retval.t = ray.tMax;

	float tFar = FLT_MAX;
	float tNear = -FLT_MAX;

	// x - y - z
	float t1 = (box.min.x - ray.origin.x) / ray.direction.x;
	float t2 = (box.max.x - ray.origin.x) / ray.direction.x;

	float tmp;
	if (t1 > t2)
	{
		tmp = t1;
		t1 = t2;
		t2 = tmp;
	}

	if (tNear < t1)
	{
		tNear = t1;
	}
	if (tFar > t2)
	{
		tFar = t2;
	}

	t1 = (box.min.y - ray.origin.y) / ray.direction.y;
	t2 = (box.max.y - ray.origin.y) / ray.direction.y;


	if (t1 > t2)
	{
		tmp = t1;
		t1 = t2;
		t2 = tmp;
	}

	if (tNear < t1)
	{
		tNear = t1;
	}
	if (tFar > t2)
	{
		tFar = t2;
	}

	t1 = (box.min.z - ray.origin.z) / ray.direction.z;
	t2 = (box.max.z - ray.origin.z) / ray.direction.z;
	if (t1 > t2)
	{
		tmp = t1;
		t1 = t2;
		t2 = tmp;
	}

	if (tNear < t1)
	{
		tNear = t1;
	}
	if (tFar > t2)
	{
		tFar = t2;
	}


	/*
	if (tNear < 0 && tFar > 0)
	{
		retval.t = ray.tMin + 0.0001f;
		return retval;
	}
	*/

	if (tNear <= tFar)
	{
		retval.t = tNear > ray.tMin ? tNear : tFar;
		return retval;
	}

	return retval;
}

TraceResult IntersectTriangle(const Ray& ray, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
{
	TraceResult retval;
	retval.t = ray.tMax;

	glm::vec3 p01 = p1 - p0;
	glm::vec3 p02 = p2 - p0;

	/*
	Here we want to solve for the following:

	ray->origin + t*ray->direction = u*p01 + v* p02 + *v0

	O = ray->origin
	D = ray->direction

	This way our linear system of equations becomes:

	O - *v0 = -t*D + u*p01 + v*p02

	We get rid of the minus sign in front of t by swappping coloums in the LER later on.
	Furthermore, we use the triple scalar product efficiently to find our solutions for u,v,t

	Eventually we get:
					u
	[p01,D,p02] * [ t ] = [O - *v0]
					v

	We want to solve the system above, as efficiently as possible

	*/

	//Firstly we calculate our main denominator determinant
	glm::vec3 pVector = glm::cross(ray.direction, p02);
	float determinant = glm::dot(p01, pVector);

	//Early exit if this determinant is pretty small -> means p01, p02, D are linearly codependent
	//This mainly happens when D is paralell to the triangle's plane
	if (fabs(determinant) < 1e-7f) return retval;
	float invDeterminant = 1.0f / determinant;

	glm::vec3 tVector = ray.origin - p0;
	float u = glm::dot(tVector, pVector) * invDeterminant;
	if (u < 0.0f || u>1.0f) return retval;

	//This is important, we have this exact order here because we got rid of the minus sign in front of t
	glm::vec3 qVector = glm::cross(tVector, p01);
	float v = glm::dot(ray.direction, qVector) * invDeterminant;
	if (v < 0.0f || u + v > 1.0f) return retval;

	float t = glm::dot(p02, qVector) * invDeterminant;

	//if (t < ray->tMin || t > ray->tMax) return retval;

	retval.t = t;
	//weights->x = 1 - u - v;
	//weights->y = u;
	//weights->z = v;

	return retval;
}

TraceResult IntersectNode(Ray& ray, const RayTracedScene& scene, int bvhIndex)
{
	TraceResult retvalResult;
	retvalResult.t = ray.tMax;


	TraceResult boxResult = IntersectBox(ray, scene.bvhNodes[bvhIndex].box);
	//std::cout << "Hitbox\n";
	BvhNode node = scene.bvhNodes[bvhIndex];

	if (boxResult.t > ray.tMin && boxResult.t < ray.tMax)
	{
		
		//We hit the box
		if (node.minChild == -1 && node.maxChild == -1)
		{
			//If the node is a child
			//std::cout << "Child Node\n";
			for (int i = node.startIndex;i < node.endIndex;++i)
			{
				glm::ivec3 triagIndices = glm::ivec3(scene.triangleIndices[i].x, scene.triangleIndices[i].y, scene.triangleIndices[i].z);

				glm::vec3 triagP0 = glm::vec3(scene.triangleVertexPositions[triagIndices.x].x,
					scene.triangleVertexPositions[triagIndices.x].y,
					scene.triangleVertexPositions[triagIndices.x].z);

				glm::vec3 triagP1 = glm::vec3(scene.triangleVertexPositions[triagIndices.y].x,
					scene.triangleVertexPositions[triagIndices.y].y,
					scene.triangleVertexPositions[triagIndices.y].z);

				glm::vec3 triagP2 = glm::vec3(scene.triangleVertexPositions[triagIndices.z].x,
					scene.triangleVertexPositions[triagIndices.z].y,
					scene.triangleVertexPositions[triagIndices.z].z);

				TraceResult triagResult = IntersectTriangle(ray, triagP0, triagP1, triagP2);
				if (triagResult.t > ray.tMin && triagResult.t < ray.tMax && retvalResult.t > triagResult.t)
				{
					retvalResult.t = triagResult.t;
					//ray.tMax = retvalResult.t;
				}
			}
		}
		else
		{
			//if the node is an inner node
			//we go further down the recursion

			TraceResult minChildResult = IntersectNode(ray, scene, scene.bvhNodes[bvhIndex].minChild);
			if (minChildResult.t > ray.tMin && minChildResult.t < ray.tMax && minChildResult.t < retvalResult.t)
			{
				retvalResult.t = minChildResult.t;
			}


			TraceResult maxChildResult = IntersectNode(ray, scene, scene.bvhNodes[bvhIndex].maxChild);

			

			if (maxChildResult.t > ray.tMin && maxChildResult.t < ray.tMax && maxChildResult.t < retvalResult.t)
			{
				retvalResult.t = maxChildResult.t;
			}
		}
	}

	return retvalResult;
}

TraceResult IntersectObject(const Ray& ray, const RayTracedScene& scene, int objectIndex)
{
	Ray transRay;
	transRay.tMin = ray.tMin;
	transRay.tMax = ray.tMax;
	transRay.origin = glm::vec3(scene.objectDatas[objectIndex].invWorldTransform * glm::vec4(ray.origin, 1.0f));
	transRay.direction = glm::vec3(scene.objectDatas[objectIndex].invWorldTransform * glm::vec4(ray.direction, 0.0f));

	/*
	std::cout << "world dir: "
		<< ray.direction.x << " "
		<< ray.direction.y << " "
		<< ray.direction.z << "\n";

	std::cout << "obj dir: "
		<< transRay.direction.x << " "
		<< transRay.direction.y << " "
		<< transRay.direction.z << "\n";

		*/

	TraceResult retvalResult;
	retvalResult.t = ray.tMax;

	int meshIndex = scene.objectDatas[objectIndex].meshIndex;
	TraceResult meshResult = IntersectNode(transRay, scene, scene.meshDatas[meshIndex].bvhRootIndex);
	if (meshResult.t > transRay.tMin && meshResult.t < transRay.tMax)
	{
		retvalResult.t = meshResult.t;
	}

	//retvalResult.entityIndex = objectIndex;

	return retvalResult;
}



void VirtualWorld::Pick(int x, int y)
{
	glm::vec3 camPos = camera->GetPosition();
	
	glm::vec3 camFront = camera->GetFront();
	glm::vec3 camUp = camera->GetUp();
	glm::vec3 camRight = camera->GetRight();


	float displayWidth = 2 * tanf(glm::radians(camera->GetFovx()) / 2);
	float displayHeight = displayWidth / camera->GetAspect();

	float unitWidth = displayWidth / this->displayWidth;
	float unitHeight = displayHeight / this->displayHeight;

	glm::vec3 worldScreenMid = camPos + camFront; //Assumes that d = 1

	glm::vec3 leftUpperPixelMid = worldScreenMid +
		camRight * (displayWidth / 2 - unitWidth / 2) +
		camUp * (displayHeight / 2 - unitHeight / 2);


	glm::vec3 worldRayPixel = leftUpperPixelMid -
		y * unitHeight * camUp -
		x * unitWidth * camRight;


	Ray pickRay;
	pickRay.tMin = camera->GetZNear();
	pickRay.tMax = camera->GetZFar();
	pickRay.origin = camPos;
	pickRay.direction = glm::normalize(worldRayPixel - camPos);

	TraceResult pickResult;
	pickResult.t = pickRay.tMax;
	pickResult.entityIndex = -1;

	for (int i = 0;i < entityCount;++i)
	{
		//std::cout << "Picking entity: " << i << "\n";
		TraceResult currObjectResult = IntersectObject(pickRay, scene, i);
		if (currObjectResult.t < pickRay.tMax && currObjectResult.t > pickRay.tMin && currObjectResult.t < pickResult.t)
		{
			pickResult.t = currObjectResult.t;
			pickResult.entityIndex = i;
		}

	}

	if (pickResult.entityIndex >= 0)
	{
		pickedEntityIndex = pickResult.entityIndex;
		//std::cout << "Picked entity Index set to: " << pickedEntityIndex << "\n";
	}
	else
	{
		pickedEntityIndex = -1;
	}
}


void VirtualWorld::SwitchRenderMode()
{
	if (isRayTracingToggled)
	{
		isRayTracingToggled = false;
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		isRayTracingToggled = true;
	}
}
void VirtualWorld::RenderBoxBVH(int rootIndex, const glm::mat4& objectWorldTransform, int depth)
{
	BvhNode* node = &scene.bvhNodes[rootIndex];

	if (depth == debugDepth)
	{
		glm::mat4 worldTransform = glm::translate(glm::mat4(1.0f), glm::vec3(node->box.min.x, node->box.min.y, node->box.min.z)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(glm::abs(node->box.max.x - node->box.min.x),
				glm::abs(node->box.max.y - node->box.min.y),
				glm::abs(node->box.max.z - node->box.min.z))) *
			glm::translate(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5));

		debugShader.SetUniform<glm::mat4>("uWorldTransform", objectWorldTransform * worldTransform);

		glm::vec3 debugColor;
		if (depth % 3 == 0)
		{
			debugColor = glm::vec3(1, 0, 0);
		}
		else if (depth % 3 == 1)
		{
			debugColor = glm::vec3(0, 1, 0);
		}
		else
		{
			debugColor = glm::vec3(0, 0, 1);
		}

		debugShader.SetUniform<glm::vec3>("debugColor", debugColor);

		debugBoxMesh.Draw();
	}
	

	if (node->minChild >= 0)
	{
		RenderBoxBVH(node->minChild, objectWorldTransform,depth+1);
	}
	if (node->maxChild >= 0)
	{
		RenderBoxBVH(node->maxChild, objectWorldTransform, depth + 1);
	}
}

void VirtualWorld::OpenGLRender(float timePassed)
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLE);
	glEnable(GL_DEPTH_TEST);

	rasterizerShader.Bind();


	glm::mat4 projTransform = camera->GetPerspectiveMatrix();
	glm::mat4 viewTransform = camera->GetViewMatrix();

	

	rasterizerShader.SetUniform<glm::mat4>("uProjectionTransform", projTransform);
	rasterizerShader.SetUniform<glm::mat4>("uViewTransform", viewTransform);
	rasterizerShader.SetUniform<glm::vec3>("uCameraPosition", camera->GetPosition());

	for (int i = 0;i < entityCount;++i)
	{
		glm::mat4 worldTransform = rasterScene.objects[i].worldTransform;
		glm::mat4 inverseTransposeWorldTransform = glm::transpose(rasterScene.objects[i].worldTransform);

		rasterizerShader.SetUniform<glm::mat4>("uWorldTransform", worldTransform);
		rasterizerShader.SetUniform<glm::mat4>("uInverseTransposedWorldTransform", inverseTransposeWorldTransform);
		

		Model model = rasterScene.models[rasterScene.objects[i].modelIndex];
		Mesh<VertexP3N3T2> mesh = rasterScene.meshes[model.meshIndex];
		
		Material material= rasterScene.materials[model.materialIndex];
		Texture currentTexture = rasterScene.textures[material.textureIndex];

		currentTexture.Bind(0);
		rasterizerShader.SetUniform<int>("uTexture", 0);

		mesh.Draw();


		currentTexture.Unbind();
	}
	

	rasterizerShader.Unbind();

	debugShader.Bind();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_DEPTH_TEST);
	debugShader.SetUniform<glm::mat4>("uProjectionTransform", projTransform);
	debugShader.SetUniform<glm::mat4>("uViewTransform", viewTransform);

	for (int i = 0;i < entityCount;++i)
	{
		RenderBoxBVH(scene.meshDatas[scene.objectDatas[i].meshIndex].bvhRootIndex, scene.objectDatas[i].worldTransform,0);
	}

	debugShader.Unbind();
}



void VirtualWorld::RayTracedRender(float timePassed)
{

	glFinish();

	cl_int error;
	error = clEnqueueAcquireGLObjects(queue,
		1,
		&clImage,
		0, nullptr, nullptr);
	CHECK_ERROR(error);

	CameraData cameraData;
	cameraData.position = glm::vec4(camera->GetPosition(),1.0f);
	cameraData.front = glm::vec4(camera->GetFront(),1.0f);
	cameraData.up = glm::vec4(camera->GetUp(),1.0f);
	cameraData.right = glm::vec4(camera->GetRight(),1.0f);
	cameraData.aspect = camera->GetAspect();
	cameraData.fovx = camera->GetFovx();
	cameraData.zNear = camera->GetZNear();
	cameraData.zFar = camera->GetZFar();

	//std::cout << cameraData.front.z << '\n';

	//std::cout << cameraData.aspect;

	

	error = clEnqueueWriteBuffer(queue,
		scene.objectDatasBuffer,
		CL_TRUE,
		0,
		sizeof(ObjectData) * entityCount,
		scene.objectDatas.data(),
		0, nullptr, nullptr);

	CHECK_ERROR(error);



	error = clEnqueueWriteBuffer(queue,
		cameraBuffer,
		CL_TRUE,
		0,
		sizeof(CameraData),
		&cameraData,
		0, nullptr, nullptr);

	CHECK_ERROR(error);

	

	error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clImage);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 1, sizeof(int), &displayWidth);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 2, sizeof(int), &displayHeight);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 3, sizeof(float), &timePassed);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 4, sizeof(cl_mem), &cameraBuffer);

	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 5, sizeof(cl_mem), &scene.triangleVertexPositionsBuffer);

	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 6, sizeof(cl_mem), &scene.triangleVertexAttributesBuffer);

	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 7, sizeof(cl_mem), &scene.triangleIndicesBuffer);

	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 8, sizeof(cl_mem), &scene.bvhNodesBuffer);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 9, sizeof(cl_mem), &scene.rgbaValuesBuffer);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 10, sizeof(cl_mem), &scene.textureDatasBuffer);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 11, sizeof(cl_mem), &scene.materialDatasBuffer);
	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 12, sizeof(cl_mem), &scene.meshDatasBuffer);

	CHECK_ERROR(error);
	error = clSetKernelArg(kernel, 13, sizeof(cl_mem), &scene.objectDatasBuffer);

	CHECK_ERROR(error);
	int objectCount = scene.objectDatas.size();
	error = clSetKernelArg(kernel, 14, sizeof(int), &entityCount);

	CHECK_ERROR(error);

	size_t localSize[2] = { 16, 16 };
	size_t globalSize[2] = { (displayWidth + localSize[0] - 1) / localSize[0] * localSize[0],
							 (displayHeight + localSize[1] - 1) / localSize[1] * localSize[1] };



	
	// ... kód ...
	
	

	error = clEnqueueNDRangeKernel(queue,
		kernel,
		2,
		nullptr,
		globalSize,
		localSize,
		0, nullptr, nullptr);
	CHECK_ERROR(error);

	
	clFinish(queue);

	

	//std::cout<<"Ray tracer: " << diff.count() << std::endl;

	error = clEnqueueReleaseGLObjects(queue,
		1,
		&clImage,
		0, nullptr, nullptr);
	CHECK_ERROR(error);


	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, texId));



	quadShader.Bind();

	quadShader.SetUniform<int>("uTexture", 0);

	quadMesh.Draw();


	quadShader.Unbind();



	glBindTexture(GL_TEXTURE_2D, 0);

}


int VirtualWorld::LoadRayTracedTexture(const std::string& filePath)
{
	int textureWidth;
	int textureHeight;
	int bitsPerPixel;

	unsigned char* byteStream = stbi_load(filePath.c_str(), &textureWidth, &textureHeight, &bitsPerPixel, 4);

	std::vector<RGBA> newRGBAs;
	newRGBAs.reserve(textureWidth * textureHeight);

	if (byteStream != nullptr)
	{
		for (int i = 0;i < textureWidth * textureHeight;++i)
		{
			float r = (byteStream[4 * i] / 256.0f);
			float g = (byteStream[4 * i+1] / 256.0f);
			float b = (byteStream[4 * i+2] / 256.0f);
			float a = (byteStream[4 * i+3] / 256.0f);
			RGBA newRGBA = { r,g,b,a };

			newRGBAs.push_back(newRGBA);
		}

		int textureDataOffset = scene.rgbaValues.size();
		scene.rgbaValues.insert(scene.rgbaValues.end(), newRGBAs.begin(), newRGBAs.end());

		TextureData newTextureData;
		newTextureData.width = textureWidth;
		newTextureData.height = textureHeight;
		newTextureData.offset = textureDataOffset;
		scene.textureDatas.push_back(newTextureData);

		stbi_image_free(byteStream);

		return scene.textureDatas.size() - 1;
	}

	return -1;
}

int VirtualWorld::AddRayTracedMaterial(int textureIndex, float reflectivity, float roughness,
									   bool isRefracting, float refractIndex, float refractability)
{
	MaterialData newMaterialData;
	newMaterialData.reflectivity = reflectivity;
	newMaterialData.roughness = roughness;
	newMaterialData.textureIndex = textureIndex;
	newMaterialData.isRafracting = isRefracting;
	newMaterialData.refractIndex = refractIndex;
	newMaterialData.refractability = refractability;

	int materialIndex = scene.materialDatas.size();

	scene.materialDatas.push_back(newMaterialData);

	return materialIndex;
}

int VirtualWorld::LoadRayTracedMesh(const std::string& filePath, int materialIndex)
{
	std::vector<cl_float4> uniquePositions;
	std::vector<cl_float4> uniqueNormals;
	std::vector<cl_float2> uniqueTexCoords;

	std::vector<cl_int3> finalIndices;
	std::vector<cl_float4> finalPositions;

	std::vector<Attribute> finalAttributes;

	//this is where indexing and vertex assembly happens
	struct IndexKey
	{
		int posInd;
		int normInd;
		int texInd;

		bool operator==(const IndexKey& other) const
		{
			return posInd == other.posInd &&
				normInd == other.normInd &&
				texInd == other.texInd;
		}
	};

	struct IndexKeyHash
	{
		std::size_t operator()(const IndexKey& k) const
		{
			return ((std::hash<int>()(k.posInd) ^
				(std::hash<int>()(k.normInd) << 1)) >> 1) ^
				(std::hash<int>()(k.texInd) << 1);
		}
	};



	std::unordered_map<IndexKey, unsigned int, IndexKeyHash> threeCompToVertexInd;

	//std::string filePath = "res/assets/old_house.obj";

	std::ifstream meshFile(filePath);

	std::string meshLinebuffer;
	while (std::getline(meshFile, meshLinebuffer))
	{
		std::vector<std::string> words;
		//Splitting up the line into words
		std::stringstream stringStream(meshLinebuffer);

		std::string word;

		while (stringStream >> word)
		{
			words.push_back(word);
		}

		//now in words we have all the words.

		if (words.size() < 1) continue;
		if (words[0] == "v")
		{
			cl_float3 newPosition;
			newPosition.x = std::stof(words[1]);
			newPosition.y = std::stof(words[2]);
			newPosition.z = std::stof(words[3]);

			uniquePositions.push_back(newPosition);
		}
		else if (words[0] == "vn")
		{
			cl_float3 newNormal;
			newNormal.x = std::stof(words[1]);
			newNormal.y = std::stof(words[2]);
			newNormal.z = std::stof(words[3]);

			uniqueNormals.push_back(newNormal);
		}
		else if (words[0] == "vt")
		{
			cl_float2 newTexCoord;
			newTexCoord.x = std::stof(words[1]);
			newTexCoord.y = std::stof(words[2]);

			uniqueTexCoords.push_back(newTexCoord);
		}
		else if (words[0] == "f")
		{

			int polygonSize = words.size() - 1;

			std::vector<unsigned int> polygonIndices;

			for (int i = 0;i < polygonSize;++i)
			{
				std::string currentIndexing = words[i + 1];


				std::vector<std::string> diffIndices;

				std::stringstream stringStreamIndexing(currentIndexing);

				char delimeter = '/';

				std::string currentIndex;

				while (std::getline(stringStreamIndexing, currentIndex, delimeter))
				{
					diffIndices.push_back(currentIndex);
				}


				IndexKey indexKey;
				indexKey.posInd = std::stoi(diffIndices[0]);
				indexKey.texInd = std::stoi(diffIndices[1]);
				indexKey.normInd = std::stoi(diffIndices[2]);


				//std::cout << diffIndices[1] << " " << diffIndices[2] << '\n';

				auto mapIt = threeCompToVertexInd.find(indexKey);
				if (mapIt != threeCompToVertexInd.end())
				{
					//we found the key, therefore we already have a vertex specified with these indices

					unsigned int vertexIndex = mapIt->second;
					polygonIndices.push_back(vertexIndex);
				}
				else
				{

					finalPositions.push_back(uniquePositions[indexKey.posInd - 1]);

					Attribute finalAttribute;
					finalAttribute.normal = uniqueNormals[indexKey.normInd - 1];
					finalAttribute.texCoords = uniqueTexCoords[indexKey.texInd - 1];

					finalAttributes.push_back(finalAttribute);

					unsigned int newVertexInd = finalPositions.size() - 1;
					threeCompToVertexInd[indexKey] = newVertexInd;

					polygonIndices.push_back(newVertexInd);
				}
			}

			for (int i = 0;i < polygonSize - 2;++i)
			{
				int alreadyExistingCount = scene.triangleVertexPositions.size();

				cl_int3 triagIndices;

				triagIndices.x = polygonIndices[0] + alreadyExistingCount;
				triagIndices.y = polygonIndices[1 + i] + alreadyExistingCount;
				triagIndices.z = polygonIndices[(2 + i) % polygonSize] + alreadyExistingCount;

				finalIndices.push_back(triagIndices);


			}


		}

	}

	meshFile.close();

	int startIndex = scene.triangleIndices.size();

	scene.triangleVertexPositions.insert(scene.triangleVertexPositions.end(), finalPositions.begin(), finalPositions.end());
	scene.triangleVertexAttributes.insert(scene.triangleVertexAttributes.end(), finalAttributes.begin(), finalAttributes.end());
	scene.triangleIndices.insert(scene.triangleIndices.end(), finalIndices.begin(), finalIndices.end());

	int endIndex  = scene.triangleIndices.size();

	//Construction of BVH Nodes
	BvhNode meshRootNode;
	meshRootNode.startIndex = startIndex;
	meshRootNode.endIndex = endIndex;
	meshRootNode.maxChild = -1;
	meshRootNode.minChild = -1;
	
	AABB rootAABB = GetAABBOfTriangles(startIndex, endIndex);

	meshRootNode.box = rootAABB;
	
	//Okay, we set up the root bvh node of our mesh, now lets split it recursively based on mid split
	//Max recursion depth is defined in scene struct
	int rootIndex = scene.bvhNodes.size(); //VERY IMPORTANTUS TO SET THIS AS THE MESHUS ROOTUS INDEXUS
	
	scene.bvhNodes.push_back(meshRootNode);
	SplitNode(rootIndex, 0);

	MeshData newMesh = { rootIndex,materialIndex};

	scene.meshDatas.push_back(newMesh);

	return scene.meshDatas.size() - 1;
	
}



AABB VirtualWorld::GetAABBOfTriangles(int startIndex, int endIndex)
{
	AABB retval;
	cl_float4 boxMax = { -FLT_MAX,-FLT_MAX,-FLT_MAX,0.0f };
	cl_float4 boxMin = { FLT_MAX ,FLT_MAX ,FLT_MAX ,0.0f };

	for (int i = startIndex;i < endIndex;++i)
	{
		cl_int4 indices = scene.triangleIndices[i];
		for (int j = 0;j < 3;++j)
		{
			cl_float4 currentTriagVertPos;
			if (j == 0)
			{
				currentTriagVertPos = scene.triangleVertexPositions[indices.x];
			}
			else if (j == 1)
			{
				currentTriagVertPos = scene.triangleVertexPositions[indices.y];
			}
			else
			{
				currentTriagVertPos = scene.triangleVertexPositions[indices.z];
			}
			

			if (currentTriagVertPos.x < boxMin.x)
			{
				boxMin.x = currentTriagVertPos.x;
			}
			if (currentTriagVertPos.y < boxMin.y)
			{
				boxMin.y = currentTriagVertPos.y;
			}
			if (currentTriagVertPos.z < boxMin.z)
			{
				boxMin.z = currentTriagVertPos.z;
			}

			if (currentTriagVertPos.x > boxMax.x)
			{
				boxMax.x = currentTriagVertPos.x;
			}
			if (currentTriagVertPos.y > boxMax.y)
			{
				boxMax.y = currentTriagVertPos.y;
			}
			if (currentTriagVertPos.z > boxMax.z)
			{
				boxMax.z = currentTriagVertPos.z;
			}

		}
		

	}

	retval.min = boxMin;
	retval.max = boxMax;

	return retval;
}

float SahBinSplitCost(const glm::vec3 parentBoxSize,const glm::vec3& box1Size, const glm::vec3& box2Size, int box1TriagCount, int box2TriagCount)
{
	float parentBoxArea = 2 * (parentBoxSize.x * parentBoxSize.y + parentBoxSize.x * parentBoxSize.z + parentBoxSize.y * parentBoxSize.z);
	float box1Area = 2 * (box1Size.x * box1Size.y + box1Size.x * box1Size.z + box1Size.y * box1Size.z);
	float box2Area = 2 * (box2Size.x * box2Size.y + box2Size.x * box2Size.z + box2Size.y * box2Size.z);

	return 1.0f + box1Area/parentBoxArea  * box1TriagCount + box2Area/parentBoxArea * box2TriagCount;
}

float SahLeafCost(const glm::vec3& boxSize, int triagCount)
{
	//float boxArea = 2 * (boxSize.x * boxSize.y + boxSize.x * boxSize.z + boxSize.y * boxSize.z);

	return triagCount;
}

void FeedPosToAABB(AABB& box, cl_float4& pos)
{
	if (box.max.x < pos.x)
	{
		box.max.x = pos.x;
	}
	if (box.max.y < pos.y)
	{
		box.max.y = pos.y;
	}
	if (box.max.z < pos.z)
	{
		box.max.z = pos.z;
	}

	if (box.min.x > pos.x)
	{
		box.min.x = pos.x;
	}
	if (box.min.y > pos.y)
	{
		box.min.y = pos.y;

	}
	if (box.min.z > pos.z)
	{
		box.min.z = pos.z;
	}
}

void VirtualWorld::GetSurfaceAreaHeuristicSplit(const BvhNode& node, int& bestAxisInd, float& splitValueAlongAxis)
{
	glm::vec3 boxSize = glm::vec3(node.box.max.x - node.box.min.x, node.box.max.y - node.box.min.y, node.box.max.z - node.box.min.z);

	float* boxSizeArray = glm::value_ptr(boxSize);

	
	
	
	
	//maxAxisInd holds the longest axis
	float leafCost = SahLeafCost(boxSize, node.endIndex - node.startIndex);

	float minCost = leafCost;
	float minSplitValueAlongAxis = FLT_MAX;
	int bestAxisIndex;

	glm::vec3 nodeBoxMin = glm::vec3(node.box.min.x, node.box.min.y, node.box.min.z);

	float* nodeBoxMinArray = glm::value_ptr(nodeBoxMin);

	for (int axis = 0;axis < 3;++axis)
	{
		for (int i = 1;i < scene.sahBinSize;++i)
		{
			float currentBinSplitValueAlongAxis = (float)i / (float)scene.sahBinSize * boxSizeArray[axis] + nodeBoxMinArray[axis];

			int triagCount1 = 0;
			int triagCount2 = 0;

			cl_float4 boxMax = { -FLT_MAX,-FLT_MAX,-FLT_MAX,0.0f };
			cl_float4 boxMin = { FLT_MAX ,FLT_MAX ,FLT_MAX ,0.0f };

			AABB box1;//{ boxMax,boxMin };
			box1.max = boxMax;
			box1.min = boxMin;



			AABB box2;
			box2.max = boxMax;
			box2.min = boxMin;

			for (int j = node.startIndex;j < node.endIndex;++j)
			{
				cl_int4 indices = scene.triangleIndices[j];

				cl_float4 a = scene.triangleVertexPositions[indices.x];
				cl_float4 b = scene.triangleVertexPositions[indices.y];
				cl_float4 c = scene.triangleVertexPositions[indices.z];

				glm::vec3 triagMid;
				triagMid.x = (a.x + b.x + c.x) / 3.0f;
				triagMid.y = (a.y + b.y + c.y) / 3.0f;
				triagMid.z = (a.z + b.z + c.z) / 3.0f;

				float* triagMidArray = glm::value_ptr(triagMid);

				float compareValueAlongAxis = triagMidArray[axis];

				if (compareValueAlongAxis <= currentBinSplitValueAlongAxis)
				{
					++triagCount1;

					FeedPosToAABB(box1, a);
					FeedPosToAABB(box1, b);
					FeedPosToAABB(box1, c);
				}
				else
				{
					++triagCount2;
					FeedPosToAABB(box2, a);
					FeedPosToAABB(box2, b);
					FeedPosToAABB(box2, c);
				}
			}

			glm::vec3 boxSize1 = glm::vec3(box1.max.x - box1.min.x, box1.max.y - box1.min.y, box1.max.z - box1.min.z);
			glm::vec3 boxSize2 = glm::vec3(box2.max.x - box2.min.x, box2.max.y - box2.min.y, box2.max.z - box2.min.z);

			if (triagCount1 == 0 || triagCount2 == 0) continue;


			float currentSahBinCostValue = SahBinSplitCost(boxSize, boxSize1, boxSize2, triagCount1, triagCount2);

			if (currentSahBinCostValue < minCost)
			{
				minCost = currentSahBinCostValue;
				bestAxisIndex = axis;

				minSplitValueAlongAxis = currentBinSplitValueAlongAxis;
			}
		}
	}

	

	if (minCost < leafCost)
	{
		bestAxisInd = bestAxisIndex;
		splitValueAlongAxis = minSplitValueAlongAxis;
	}
	else
	{
		bestAxisInd = -1;
		splitValueAlongAxis = 0.0f;
	}

}


void VirtualWorld::DebugNode(int nodeIndex)
{
	BvhNode node = scene.bvhNodes[nodeIndex];
	std::cout << "\nNode Index: " << nodeIndex << "\n";
	std::cout << "Node minChild: " << node.minChild << " node maxCHild: " << node.maxChild << "\n";
	std::cout << "Node startIndex: " << node.startIndex << " node endIndex: " << node.endIndex << "\n";
	std::cout << "Node box: \n";
	std::cout << "           Min : (x: " << node.box.min.x << " y: " << node.box.min.y << " z: " << node.box.min.z << " )\n";
	std::cout << "           Max : (x: " << node.box.max.x << " y: " << node.box.max.y << " z: " << node.box.max.z << " )\n";
}

void VirtualWorld::SplitNode(int nodeIndex, int depth)
{
	if (depth > scene.bvhMaxDepth) return;

	//std::cout << "depth: " << depth << "\n";

	BvhNode node = scene.bvhNodes[nodeIndex];

	int allIndexCount = node.endIndex - node.startIndex;
	int minSideIndexCount = 0;
	int maxSideIndexCount = 0;

	//float midSplitValue;

	int bestAxisInd; //0 x 1 y 2 z
	float splitValue;
	GetSurfaceAreaHeuristicSplit(node, bestAxisInd, splitValue);

	if (bestAxisInd < 0) return;

	//BackSwapping
	int frontIndex = node.startIndex;
	int lastIndex = node.endIndex - 1;
	while (frontIndex <= lastIndex)
	{
		cl_int4 indices = scene.triangleIndices[frontIndex];
		
		cl_float4 a = scene.triangleVertexPositions[indices.x];
		cl_float4 b = scene.triangleVertexPositions[indices.y];
		cl_float4 c = scene.triangleVertexPositions[indices.z];

		glm::vec3 triagMid;
		triagMid.x = (a.x + b.x + c.x)/3.0f;
		triagMid.y = (a.y + b.y + c.y)/3.0f;
		triagMid.z = (a.z + b.z + c.z)/3.0f;

		float* triagMidArray = glm::value_ptr(triagMid);

		float compareValue = triagMidArray[bestAxisInd];
		

		if (compareValue <= splitValue) // If the current traigs mid is on the min side we good
		{
			++frontIndex;

			++minSideIndexCount;
		}
		else
		{
			//Swap frontIndex indices and lastIndex indices
			cl_int4 temp = indices;

			scene.triangleIndices[frontIndex] = scene.triangleIndices[lastIndex];
			scene.triangleIndices[lastIndex] =temp;

			--lastIndex;
		}
	}

	maxSideIndexCount = allIndexCount - minSideIndexCount;

	if (minSideIndexCount > 0 && maxSideIndexCount > 0)
	{
		BvhNode minSideNode;
		minSideNode.minChild = -1;
		minSideNode.maxChild = -1;
		minSideNode.startIndex = node.startIndex;
		minSideNode.endIndex = node.startIndex + minSideIndexCount;

		AABB minSideAABB = GetAABBOfTriangles(minSideNode.startIndex, minSideNode.endIndex);

		minSideNode.box = minSideAABB;

		int minChildIndex = scene.bvhNodes.size();

		scene.bvhNodes[nodeIndex].minChild = minChildIndex;

		scene.bvhNodes.push_back(minSideNode);

		SplitNode(minChildIndex, depth + 1);

		BvhNode maxSideNode;
		maxSideNode.minChild = -1;
		maxSideNode.maxChild = -1;
		maxSideNode.startIndex = node.startIndex + minSideIndexCount;
		maxSideNode.endIndex = node.endIndex;

		AABB maxSideAABB = GetAABBOfTriangles(maxSideNode.startIndex, maxSideNode.endIndex);

		maxSideNode.box = maxSideAABB;

		int maxChildIndex = scene.bvhNodes.size();

		scene.bvhNodes[nodeIndex].maxChild = maxChildIndex;

		scene.bvhNodes.push_back(maxSideNode);

		SplitNode(maxChildIndex, depth + 1);

		
	}
	
}


/*
void VirtualWorld::AddRayTracedObject(int meshIndex, const glm::mat4& worldTransform
{
	ObjectData newObjectData = {meshIndex , worldTransform , glm::inverse(worldTransform)};
	scene.objectDatas.push_back(newObjectData);
}

*/