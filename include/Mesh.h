#pragma once
#ifndef MESH_H
#define MESH_H

//#include <iostream>

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
//#include <stdc++.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // transzformációkhoz
#include <glm/gtc/type_ptr.hpp>   

#include "Debug.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexLayout.h"

//renderType:
// 3-> triangles
// 2-> lines
// 1->dots(not yet implemented)
enum RenderType
{
	Triangles,
	Lines,
	Points,
};

template <typename VertexType, RenderType renderType = Triangles>
class Mesh
{
private:
	VertexArray m_VAO;
	VertexBuffer m_VBO;
	IndexBuffer m_IBO;
	
	int m_vertexCount;
	int m_indexCount;
public:
	Mesh();
	

	void Init(const std::vector<VertexType>& vertices, const std::vector<unsigned int>& indices);
	void Load(const std::string& filePath);


	void Draw();
};

template<typename VertexType, RenderType renderType>
Mesh<VertexType, renderType>::Mesh() : m_vertexCount(0),m_indexCount(0)
{
	//std::cout << "Mesh" << std::endl;
}



template<typename VertexType, RenderType renderType>
void Mesh<VertexType, renderType>::Init(const std::vector<VertexType>& vertices, const std::vector<unsigned int>& indices)
{
	m_VBO.Init(vertices.data(), sizeof(VertexType) * vertices.size());
	m_IBO.Init(indices.data(), sizeof(unsigned int) * indices.size());
	m_indexCount = indices.size();

	m_VAO.Init(m_VBO, m_IBO);

	VertexLayout<VertexType> layout;

	layout.Apply();

	m_VAO.Unbind();

	m_VBO.Unbind();
	m_IBO.Unbind();
}

template<typename VertexType, RenderType renderType>
void Mesh<VertexType, renderType>::Draw()
{
	

	m_VAO.Bind();

	GLenum glEnum;
	switch (renderType)
	{
	case RenderType::Triangles:
		glEnum = GL_TRIANGLES;
		break;
	case RenderType::Lines:
		glEnum = GL_LINES;
		break;
	case RenderType::Points:
		glEnum = GL_POINTS;
		break;
	}

	GLCall(glDrawElements(glEnum, m_indexCount, GL_UNSIGNED_INT, nullptr));

	m_VAO.Unbind();
}





//-------------------------Mesh Loader function---------------------------


template<>
void Mesh<VertexP3N3T2>::Load(const std::string& filePath);



template<typename UnwrittenVertexType, RenderType renderType>
void Mesh<UnwrittenVertexType, renderType>::Load(const std::string& filePath)
{
	static_assert(sizeof(UnwrittenVertexType) == 0, "[STATIC_ASSERT_ERROR]: Mesh Loader for this specific type hasn't been declared yet!\n");
}

#endif