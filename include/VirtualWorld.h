#pragma once
#ifndef VIRTUALWORLD_H
#define VIRTUALWORLD_H

//The OpenGL Extension Wrangler
#include <GL/glew.h>

//The OpenCL library
#include <CL/cl_gl.h>
#include <GL/wglew.h>

#include "Debug.h"

#include "Camera.h"

//My OpenGL Abstraction layers
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "ShadowMap.h"
#include "Vertex.h"

//My OpenCL Abstraction libraries

//nothing yet cuz im bad

#include "Utils.h"

//Temporary struct that stores the camera data
struct CameraData
{
	glm::vec4 position;
	glm::vec4 front;
	glm::vec4 up;
	glm::vec4 right;

	float fovx;
	float aspect;
	float zNear;	//Right hand coordinate system, where front - z up - y left - x
	float zFar;
};

struct Attribute
{
	cl_float3 normal;
	cl_float2 texCoords;
	float padding1;
	float padding2;
};

struct AABB
{
	cl_float4 max;
	cl_float4 min;
};

struct BvhNode
{
	int startIndex;
	int endIndex;
	int maxChild;
	int minChild;
	AABB box;
};

struct RGBA
{
	float r;
	float g;
	float b;
	float a;
};

struct TextureData
{
	int offset;
	int width;
	int height;
};

struct MaterialData
{
	int textureIndex;

	float reflectivity;
	float roughness;

	int isRafracting;
	float refractIndex;
	float refractability;
};

struct MeshData
{
	int bvhRootIndex;

	int materialIndex;
};

struct Material
{
	int textureIndex;
};

struct Model
{
	int meshIndex;
	int materialIndex;
};

struct Object
{
	int modelIndex;
	glm::mat4 worldTransform;
	glm::mat4 invWorldTransform;
};


struct ObjectData
{
	int meshIndex;
	glm::mat4 worldTransform;
	glm::mat4 invWorldTransform;
};

struct RasterizedScene
{
	std::vector<Mesh<VertexP3N3T2>> meshes;
	std::vector<Texture> textures;
	std::vector<Material> materials;
	std::vector<Model> models;
	std::vector<Object> objects;
};

struct RayTracedScene
{
	std::vector<cl_float4> triangleVertexPositions;
	cl_mem triangleVertexPositionsBuffer;

	std::vector<Attribute> triangleVertexAttributes;
	cl_mem triangleVertexAttributesBuffer;

	std::vector<cl_int4> triangleIndices;
	cl_mem triangleIndicesBuffer;

	std::vector<BvhNode> bvhNodes;
	cl_mem bvhNodesBuffer;

	std::vector<RGBA> rgbaValues;
	cl_mem rgbaValuesBuffer;

	std::vector<TextureData> textureDatas;
	cl_mem textureDatasBuffer;

	std::vector<MaterialData> materialDatas;
	cl_mem materialDatasBuffer;

	std::vector<MeshData> meshDatas;
	cl_mem meshDatasBuffer;

	std::vector<ObjectData> objectDatas;
	cl_mem objectDatasBuffer;

	int bvhMaxDepth = 30;
	int sahBinSize = 32;
};

//ECS components
struct Transform
{
	glm::vec3 position;
	glm::vec3 size;
	glm::vec3 rotation;
};

struct WorldTransform
{
	glm::mat4 matrix;
	glm::mat4 invMatrix;
};

struct RenderData
{
	int rasterModelIndex;
	int raytracedModelIndex;
	AABB worldBox;
};


class VirtualWorld
{
public:
	int debugDepth = 0;

private:
	bool isRayTracingToggled;

	int displayWidth;
	int displayHeight;

	Camera* camera;

	
	float renderTime;
	//Simplified ECS parts----------------------------------
	int entityCapacity;
	int entityCount;

	Transform* transforms;
	WorldTransform* worldTransforms;
	RenderData* renders;

	int pickedEntityIndex = 0;

	//------Everything needed for rasterized rendering-------
	Shader rasterizerShader;

	//Texture testTexture;
	//Mesh<VertexP3N3T2> testMesh;

	Shader debugShader;
	Mesh<VertexP3, Lines> debugBoxMesh;
	
	RasterizedScene rasterScene;

	//------Everything needed for ray traced rendering-------
	Mesh<VertexP2T2> quadMesh;
	Shader quadShader;
	unsigned int texId;

	//OpenCL stuff
	cl_context context;
	cl_command_queue queue;
	cl_kernel kernel;
	cl_mem clImage;

	cl_mem cameraBuffer;
	//MeshData meshData;
	RayTracedScene scene;
	
	

public:
	VirtualWorld();
	~VirtualWorld();

	int LoadTexture(const std::string& filePath);
	int AddMaterial(int textureIndex, float reflectivity, float roughness, bool isRefracting, float refractIndex, float refractability);
	int LoadMesh(const std::string& filePath, int materialIndex);
	void AddObject(int modelIndex,const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& size);

	void Init(int displayWidth, int displayHeight,Camera* camera);

	void Update();
	void Render(float timePassed);

	void Resize(int displayWidth, int displayHeight);
	void Pick(int x, int y);
	void SwitchRenderMode();

private:
	void OpenGLRender(float timePassed);
	void RayTracedRender(float timePassed);

	AABB GetAABBOfTriangles(int startIndex, int endIndex);
	void SplitNode(int nodeIndex, int depth);
	void GetSurfaceAreaHeuristicSplit(const BvhNode& node, int& longestAxisInd, float& splitValueAlongAxis);

	int LoadRayTracedTexture(const std::string& filePath);
	int AddRayTracedMaterial(int textureIndex, float reflectivity, float roughness, bool isRefracting, float refractIndex, float refractability);
	int LoadRayTracedMesh(const std::string& filePath,int materialIndex);
	//void AddRayTracedObject(int meshIndex, const glm::mat4& worldTransform);

	void RenderBoxBVH(int rootIndex, const glm::mat4& objectWorldTransform, int depth);
	void DebugNode(int nodeIndex);
};





#endif
