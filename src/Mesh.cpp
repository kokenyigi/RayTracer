#include "Mesh.h"


template<>
void Mesh<VertexP3N3T2>::Load(const std::string& filePath)
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;

	std::vector<unsigned int> indices;
	std::vector<VertexP3N3T2> vertices;

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

	std::ifstream file(filePath);

	std::string linebuffer;
	while (std::getline(file, linebuffer))
	{
		std::vector<std::string> words;
		//Splitting up the line into words
		std::stringstream stringStream(linebuffer);

		std::string word;

		while (stringStream >> word)
		{
			words.push_back(word);
		}

		//now in words we have all the words.

		if (words.size() < 1) continue;
		if (words[0] == "v")
		{
			glm::vec3 newPosition;
			newPosition.x = std::stof(words[1]);
			newPosition.y = std::stof(words[2]);
			newPosition.z = std::stof(words[3]);

			positions.push_back(newPosition);
		}
		else if (words[0] == "vn")
		{
			glm::vec3 newNormal;
			newNormal.x = std::stof(words[1]);
			newNormal.y = std::stof(words[2]);
			newNormal.z = std::stof(words[3]);

			normals.push_back(newNormal);
		}
		else if (words[0] == "vt")
		{
			glm::vec2 newTexCoord;
			newTexCoord.x = std::stof(words[1]);
			newTexCoord.y = std::stof(words[2]);

			texCoords.push_back(newTexCoord);
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
					VertexP3N3T2 newVertex;
					newVertex.position = positions[indexKey.posInd - 1];
					newVertex.normal = normals[indexKey.normInd - 1];
					newVertex.textureCoord = texCoords[indexKey.texInd - 1];

					vertices.push_back(newVertex);

					unsigned int newVertexInd = vertices.size() - 1;
					threeCompToVertexInd[indexKey] = newVertexInd;

					polygonIndices.push_back(newVertexInd);
				}
			}

			for (int i = 0;i < polygonSize - 2;++i)
			{
				indices.push_back(polygonIndices[0]);
				indices.push_back(polygonIndices[1 + i]);
				indices.push_back(polygonIndices[(2 + i) % polygonSize]);
			}


		}

	}

	file.close();

	

	//finally, initialize the mesh
	Init(vertices, indices);
}