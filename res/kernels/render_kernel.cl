
#define maxStackSize 32


typedef struct CameraData
{
	float4 position;
	
	float4 front;
	float4 up;
	float4 right;

	float fovx;
	float aspect;
	
	float zNear;
	//Right hand coordinate system, where front - z up - y left - x
	float zFar;
} CameraData;

typedef struct
{
	int data[maxStackSize];
	int count;

}Stack;

void StackInit(Stack* stack)
{
	stack->count = 0;
}

int StackIsEmpty(Stack* stack)
{
	return stack->count == 0;
}

void StackSetEmpty(Stack* stack)
{
	stack->count = 0;
}

void StackPush(Stack* stack ,int newItem)
{
	stack->data[stack->count] = newItem;
	++stack->count;
}

int StackTop(Stack* stack)
{
	return stack->data[stack->count - 1];
}

int StackPop(Stack* stack)
{
	--stack->count;
	return stack->data[stack->count];
}


typedef struct Attributes
{
	float4 normal;
	float2 texCoord;
}Attributes;

/*
typedef struct
{
	__global float3* positions;
	int vertexCount;

	__global Attributes* attributes;

	__global int3* indices;
	int indexCount;

	float reflectivity;
	float roughness;


} MeshData;
*/
typedef struct
{
	float3 min;
	float3 max;
} AABB;

typedef struct
{
	int startIndex;
	int endIndex;
	int maxChild;
	int minChild;
	AABB box;
}BvhNode;

typedef struct
{
	int offset;
	int width;
	int height;

}TextureData;

typedef struct
{
	int textureIndex;

	float reflectivity;
	float roughness;

	int isRefracting;
	float refractIndex;
	float refractability;

}MaterialData;

typedef struct
{
	int bvhRootIndex;
	int materialIndex;
}MeshData;

typedef struct
{
	int meshIndex;
	float worldTransform[16];
	float invWorldTransform[16];
} ObjectData;

typedef struct
{
	__global float3* triangleVertexPositions;
	__global Attributes* triangleVertexAttributes;
	__global int3* triangleIndices;
	__global BvhNode* bvhNodes;
	__global float4* rgbaValues;
	__global TextureData* textureDatas;
	__global MaterialData* materialDatas;
	__global MeshData* meshDatas;
	__global ObjectData* objectDatas;
	int objectCount;
	float3 cameraPosition;
	
}Scene;

typedef struct
{
	float3 color;
	float reflectivity;

} Material;


typedef struct Ray
{
	float3 origin;
	float tMin;
	float3 direction;
	float tMax;
	float3 invDirection;

} Ray;

typedef struct TraceResult
{
	float3 normal;
	float2 texCoords;
	float t; //If t reaches tmax of our ray, we discard(we traced too far)
	int materialIndex;

} TraceResult;

typedef struct BoxTraceResult
{
	float t;

} BoxTraceResult;

typedef struct
{
	float3 point;
	float3 normal;

	Material material;

} Plane;

typedef struct
{
	float3 mid;
	float radius;
	Material material;

} Sphere;

typedef struct
{
	float3 p1;
	float3 p2;
	float3 p3;
	Material material;

}Triangle;



float3 Reflect(float3* v, float3* n)
{
	return (*v - 2 * *n * dot(*v, *n));
}

bool Refract(float3* v, float3* n, float eta,float3* outVec)
{
	float cosAlpha = dot(-*v, *n);
	float underSqr = 1.0f - (1 - cosAlpha * cosAlpha) / (eta * eta);

	if (underSqr < 0.0f)
	{
		return false; //this is numerically shit, TIR happend
	}

	*outVec =  ( *v/eta + *n * (cosAlpha/eta - sqrt(underSqr)) );

	return true;
}

float3 Matrix4x4MultVec4(float* matrix4x4, float4* vector)
{
	//The matrix is represented in coloumn wise order
	float4 retval = (float4)(0,0,0,0);
	retval.x = dot((float4)(matrix4x4[0], matrix4x4[4], matrix4x4[8], matrix4x4[12]), *vector);
	retval.y = dot((float4)(matrix4x4[1], matrix4x4[5], matrix4x4[9], matrix4x4[13]), *vector);
	retval.z = dot((float4)(matrix4x4[2], matrix4x4[6], matrix4x4[10], matrix4x4[14]), *vector);
	//W doesn't matter
	//retval.z = dot((float4)(matrix4x4[0], matrix4x4[4], matrix4x4[8], matrix4x4[12]), *vector);
	return retval.xyz;
}

float3 Matrix4x4TransposeMultVec4(float* matrix4x4, float4* vector)
{
	//The matrix is represented in coloumn wise order
	float4 retval = (float4)(0, 0, 0, 0);
	retval.x = dot((float4)(matrix4x4[0], matrix4x4[1], matrix4x4[2], matrix4x4[3]), *vector);
	retval.y = dot((float4)(matrix4x4[4], matrix4x4[5], matrix4x4[6], matrix4x4[7]), *vector);
	retval.z = dot((float4)(matrix4x4[8], matrix4x4[9], matrix4x4[10], matrix4x4[11]), *vector);
	//W doesn't matter
	//retval.z = dot((float4)(matrix4x4[0], matrix4x4[4], matrix4x4[8], matrix4x4[12]), *vector);
	return retval.xyz;
}

float4 Texture(float2* uv, int textureIndex, Scene* scene)
{
	

	TextureData* textureData = &scene->textureDatas[textureIndex];
	int rgbaIndex = textureData->offset +
		textureData->width * (int)(uv->y * textureData->height) +
		(int)(textureData->width * uv->x);

	return scene->rgbaValues[rgbaIndex];
}


/*
TraceResult IntersectPlane(Ray* ray, Plane* plane)
{
	TraceResult retval;
	retval.t = ray->tMax;

	float t = dot(plane->point - ray->origin,plane->normal) / dot(ray->direction, plane->normal);
	
	retval.t = t;
	retval.normal = plane->normal;
	retval.material = plane->material;

	return retval;
}

TraceResult IntersectSphere(Ray* ray, Sphere* sphere)
{
	float a = dot(ray->direction, ray->direction);
	float b = 2 * dot(ray->direction, ray->origin - sphere->mid);
	float c = dot(ray->origin - sphere->mid, ray->origin - sphere->mid) - sphere->radius * sphere->radius;

	//we need the solutions of a 2nd grade polynom

	float determinant = b * b - 4 * a * c;

	TraceResult retval;
	retval.t = ray->tMax;
	if (determinant < 0.0f)
	{
		return retval;
	}
	else if (determinant > 0.0f)
	{
		float sqrDeter = sqrt(determinant);
		float t1 = (-b + sqrDeter) / (2 * a);
		float t2 = (-b - sqrDeter) / (2 * a);

		retval.t = min(t1, t2);
		if (retval.t < ray->tMin)
		{
			retval.t = max(t1, t2);
		}

		retval.normal = (ray->origin + retval.t * ray->direction) - sphere->mid;
		retval.material = sphere->material;
	}
	else//if deter == 0
	{
		retval.t = -b / 2 / a;

		retval.normal = (ray->origin + retval.t * ray->direction) - sphere->mid;
		retval.material = sphere->material;
	}

	return retval;
}

TraceResult IntersectTriangle(Ray* ray, Triangle* triag)
{
	TraceResult retval;
	retval.t = ray->tMax;

	float3 p21 = triag->p2 - triag->p1;
	float3 p31 = triag->p3 - triag->p1;

	float3 p21xp31 = cross(p21, p31);

	float3 planeNormal = normalize(p21xp31);

	planeNormal = -planeNormal;

	Plane trigPlane = { triag->p1,planeNormal };

	TraceResult planeHitResult = IntersectPlane(ray, &trigPlane);
	if (planeHitResult.t > ray->tMin && planeHitResult.t < ray->tMax)
	{
		float t = planeHitResult.t;
		float3 hitPoint = ray->origin + t * ray->direction;

		float signedTrigArea = dot(p21xp31, planeNormal);

		float alpha = dot(cross(triag->p2 - hitPoint, triag->p3 - hitPoint), planeNormal) / signedTrigArea;
		float beta = dot(cross(triag->p3 - hitPoint, triag->p1 - hitPoint), planeNormal) / signedTrigArea;
		float gamma = dot(cross(triag->p1 - hitPoint, triag->p2 - hitPoint), planeNormal) / signedTrigArea;

		if (alpha > 0 && beta > 0 && gamma > 0)
		{
			retval.t = t;

			retval.normal = planeNormal;
			retval.material = triag->material;
		}
	}

	return retval;
}
*/


TraceResult IntersectMeshTriangle(Ray* ray, float3* v0, float3* v1, float3* v2, float3* weights)
{
	TraceResult retval;
	retval.t = ray->tMax;

	float3 p01 = *v1 - *v0;
	float3 p02 = *v2 - *v0;

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
	float3 pVector = cross(ray->direction, p02);
	float determinant = dot(p01, pVector);

	//Early exit if this determinant is pretty small -> means p01, p02, D are linearly codependent
	//This mainly happens when D is paralell to the triangle's plane
	if (fabs(determinant) < 1e-7f) return retval;
	float invDeterminant = 1.0f / determinant;

	float3 tVector = ray->origin - *v0;
	float u = dot(tVector, pVector) * invDeterminant;
	if (u < 0.0f || u>1.0f) return retval;

	//This is important, we have this exact order here because we got rid of the minus sign in front of t
	float3 qVector = cross(tVector, p01);
	float v = dot(ray->direction,qVector) * invDeterminant;
	if (v < 0.0f || u + v > 1.0f) return retval;

	float t = dot(p02,qVector)*invDeterminant;

	//if (t < ray->tMin || t > ray->tMax) return retval;

	retval.t = t;	
	weights->x = 1 - u - v;
	weights->y = u;
	weights->z = v;

	return retval;
}



BoxTraceResult IntersectAABB(Ray* ray, AABB* box)
{
	BoxTraceResult retval;
	retval.t = ray->tMax;

	float tFar = FLT_MAX;
	float tNear = -FLT_MAX;

	// x - y - z
	float t1 = (box->min.x - ray->origin.x) * ray->invDirection.x;
	float t2 = (box->max.x - ray->origin.x) * ray->invDirection.x;

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

	t1 = (box->min.y - ray->origin.y)  * ray->invDirection.y;
	t2 = (box->max.y - ray->origin.y) * ray->invDirection.y;

	
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

	t1 = (box->min.z - ray->origin.z)  * ray->invDirection.z;
	t2 = (box->max.z - ray->origin.z) * ray->invDirection.z;
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

	
	
	if (tNear < 0 && tFar > 0)
	{
		retval.t = ray->tMin + 0.0001f;
		return retval;
	}
	
	if (tNear <= tFar)
	{
		retval.t = tNear > ray->tMin ? tNear : tFar;
		return retval;
	}
	
	return retval;
}



TraceResult IntersectMesh(Ray* ray, Scene* scene, int objectIndex)
{
	TraceResult retval;
	retval.t = ray->tMax;

	bool hasHitSmth = false;
	int3 hitTriagIndices;
	float3 hitPointWeights;

	ObjectData* objectData = &scene->objectDatas[objectIndex];
	MeshData* meshData = &scene->meshDatas[objectData->meshIndex];

	int nodeIntersectCount = 0;

	Ray transformedRay;
	transformedRay.tMin = ray->tMin;
	transformedRay.tMax = ray->tMax;

	transformedRay.origin = Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->origin, 1.0f));//Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->origin,1.0f)); ray->origin;
	transformedRay.direction = Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->direction, 0.0f));//Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->direction, 0.0f)); ray->direction;
	transformedRay.invDirection = (float3)(1.0f,1.0f,1.0f) / transformedRay.direction;

	BoxTraceResult currentNodeResult = IntersectAABB(&transformedRay, &scene->bvhNodes[meshData->bvhRootIndex].box);

	//If we hit the rootNode
	if (currentNodeResult.t > transformedRay.tMin && currentNodeResult.t < transformedRay.tMax)
	{
		Stack bvhStack;
		StackInit(&bvhStack);
		StackPush(&bvhStack, meshData->bvhRootIndex);
		int currentNode;

		//Then we work on the root node
		while (!StackIsEmpty(&bvhStack))
		{
			currentNode = StackPop(&bvhStack);
			BvhNode* node = &scene->bvhNodes[currentNode];

			currentNodeResult = IntersectAABB(&transformedRay, &scene->bvhNodes[currentNode].box);


			//TraceResult currentNodeResult = IntersectAABB(&transformedRay, &node->box);
			//++nodeIntersectCount;
			if (currentNodeResult.t > transformedRay.tMin && currentNodeResult.t < transformedRay.tMax)
			{
				if (node->minChild + node->maxChild < -1) //node is leaf
				{
					for (int i = node->startIndex;i < node->endIndex;++i)
					{
						int3 triInd = scene->triangleIndices[i];

						float3 weights;

						TraceResult currentResult = IntersectMeshTriangle(&transformedRay,
							&scene->triangleVertexPositions[triInd.x],
							&scene->triangleVertexPositions[triInd.y],
							&scene->triangleVertexPositions[triInd.z], &weights);

						if (currentResult.t > transformedRay.tMin && currentResult.t < transformedRay.tMax && currentResult.t < retval.t)
						{
							hasHitSmth = true;
							//we found an intersection better than any before inside the mesh

							retval.t = currentResult.t;////currentResult.t;//
							transformedRay.tMax = currentResult.t;

							hitTriagIndices = triInd;
							hitPointWeights = weights;
						}
					}
				}
				else
				{
					//If we are here, we know that this is an inner node
					//Meaning it has atleast one child

					float compareValue1 = IntersectAABB(&transformedRay, &scene->bvhNodes[node->minChild].box).t;
					float compareValue2 = IntersectAABB(&transformedRay, &scene->bvhNodes[node->maxChild].box).t;


					if (compareValue1 < transformedRay.tMax && compareValue2 < transformedRay.tMax)
					{
						if (compareValue1 < compareValue2)
						{
							StackPush(&bvhStack, node->maxChild);
							StackPush(&bvhStack, node->minChild);
						}
						else
						{
							StackPush(&bvhStack, node->minChild);
							StackPush(&bvhStack, node->maxChild);
						}
					}
					else if (compareValue1 < transformedRay.tMax)
					{
						StackPush(&bvhStack, node->minChild);
					}
					else if (compareValue2 < transformedRay.tMax)
					{
						StackPush(&bvhStack, node->maxChild);
					}
				}
			}
			
		}
	}

	if (hasHitSmth)
	{
		float3 normal = hitPointWeights.x * scene->triangleVertexAttributes[hitTriagIndices.x].normal.xyz +
			hitPointWeights.y * scene->triangleVertexAttributes[hitTriagIndices.y].normal.xyz +
			hitPointWeights.z * scene->triangleVertexAttributes[hitTriagIndices.z].normal.xyz;

		float2 uv = hitPointWeights.x * scene->triangleVertexAttributes[hitTriagIndices.x].texCoord.xy +
			hitPointWeights.y * scene->triangleVertexAttributes[hitTriagIndices.y].texCoord.xy +
			hitPointWeights.z * scene->triangleVertexAttributes[hitTriagIndices.z].texCoord.xy;

		//Later on we have to write a better function
		float3 worldNormal = Matrix4x4TransposeMultVec4(objectData->invWorldTransform, &(float4)(normal, 0.0f));

		retval.normal = worldNormal;
		retval.texCoords = uv;

		retval.materialIndex = meshData->materialIndex; //Texture(&uv, meshData->textureIndex, scene).xyz;//retval.normal;//for now(float3)(retval.t/5.0f, 0,0); 
		/*
		if (dot(normal, transformedRay.direction) > 0)
		{
			retval.normal *= -1;
		}
		
		
		retval.material.reflectivity = meshData->reflectivity;
		*/
	}
	

	return retval;
}

bool ShadowIntersectMesh(Ray* ray, Scene* scene, int objectIndex)
{
	ObjectData* objectData = &scene->objectDatas[objectIndex];
	MeshData* meshData = &scene->meshDatas[objectData->meshIndex];

	Ray transformedRay;
	transformedRay.tMin = ray->tMin;
	transformedRay.tMax = ray->tMax;

	transformedRay.origin = Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->origin, 1.0f));//Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->origin,1.0f)); ray->origin;
	transformedRay.direction = Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->direction, 0.0f));//Matrix4x4MultVec4(objectData->invWorldTransform, &(float4)(ray->direction, 0.0f)); ray->direction;
	transformedRay.invDirection = (float3)(1.0f, 1.0f, 1.0f) / transformedRay.direction;

	BoxTraceResult currentNodeResult = IntersectAABB(&transformedRay, &scene->bvhNodes[meshData->bvhRootIndex].box);

	//If we hit the rootNode
	if (currentNodeResult.t > transformedRay.tMin && currentNodeResult.t < transformedRay.tMax)
	{
		Stack bvhStack;
		StackInit(&bvhStack);
		StackPush(&bvhStack, meshData->bvhRootIndex);
		int currentNode;

		//Then we work on the root node
		while (!StackIsEmpty(&bvhStack))
		{
			currentNode = StackPop(&bvhStack);
			BvhNode* node = &scene->bvhNodes[currentNode];

			currentNodeResult = IntersectAABB(&transformedRay, &scene->bvhNodes[currentNode].box);

			if (currentNodeResult.t > transformedRay.tMin && currentNodeResult.t < transformedRay.tMax)
			{
				if (node->minChild + node->maxChild < -1) //node is leaf
				{
					for (int i = node->startIndex;i < node->endIndex;++i)
					{
						int3 triInd = scene->triangleIndices[i];

						float3 weights;

						TraceResult currentResult = IntersectMeshTriangle(&transformedRay,
							&scene->triangleVertexPositions[triInd.x],
							&scene->triangleVertexPositions[triInd.y],
							&scene->triangleVertexPositions[triInd.z], &weights);

						if (currentResult.t > transformedRay.tMin && currentResult.t < transformedRay.tMax)
						{
							return true;
							//we found an intersection better than any before inside the mesh
						}
					}
				}
				else
				{
					//If we are here, we know that this is an inner node
					//Meaning it has atleast one child

					float compareValue1 = IntersectAABB(&transformedRay, &scene->bvhNodes[node->minChild].box).t;
					float compareValue2 = IntersectAABB(&transformedRay, &scene->bvhNodes[node->maxChild].box).t;



					if (compareValue1 < transformedRay.tMax && compareValue2 < transformedRay.tMax)
					{
						if (compareValue1 < compareValue2)
						{
							StackPush(&bvhStack, node->maxChild);
							StackPush(&bvhStack, node->minChild);
						}
						else
						{
							StackPush(&bvhStack, node->minChild);
							StackPush(&bvhStack, node->maxChild);
						}
					}
					else if (compareValue1 < transformedRay.tMax)
					{
						StackPush(&bvhStack, node->minChild);
					}
					else if (compareValue2 < transformedRay.tMax)
					{
						StackPush(&bvhStack, node->maxChild);
					}
				}
			}
			
		}
	}

	return false;
}

/*
TraceResult TraceRay(Ray* ray,Plane* planes,int planeCount,Sphere* spheres, int sphereCount,
							  Triangle* triangles, int triangleCount,AABB* boxes, int boxCount,
							  Scene* scene)
*/
TraceResult TraceRay(Ray* ray,Scene* scene)
{
	TraceResult retval;
	retval.t = ray->tMax;

	/*
	for (int i = 0;i < planeCount;++i)
	{
		TraceResult current = IntersectPlane(ray, &planes[i]);
		if (current.t > ray->tMin && current.t < ray->tMax && current.t < retval.t)
		{
			retval = current;
		}
	}

	for (int i = 0;i < sphereCount;++i)
	{
		TraceResult current = IntersectSphere(ray, &spheres[i]);
		if (current.t > ray->tMin && current.t < ray->tMax && current.t < retval.t)
		{
			retval = current;
		}
	}

	for (int i = 0;i < triangleCount;++i)
	{
		TraceResult current = IntersectTriangle(ray, &triangles[i]);
		if (current.t > ray->tMin && current.t < ray->tMax && current.t < retval.t)
		{
			retval = current;
		}
	}
	*/

	for (int i = 0;i < scene->objectCount;++i)
	{
		TraceResult current = IntersectMesh(ray, scene,i);
		if (current.t > ray->tMin && current.t < ray->tMax && current.t < retval.t)
		{
			retval = current;
		}
	}

	
	//BoxTraceResult current = IntersectAABB(ray, &boxes[0]);
	//retval.material.color = (float3)(current.t/10.0f, 0, 0);
	return retval;
}

/*
bool TraceShadowRay(Ray* ray, Plane* planes, int planeCount, Sphere* spheres, int sphereCount,
	Triangle* triangles, int triangleCount, AABB* boxes, int boxCount,
	Scene* scene)
*/
bool TraceShadowRay(Ray* ray, Scene* scene)
{
	/*
	for (int i = 0;i < planeCount;++i)
	{
		TraceResult current = IntersectPlane(ray, &planes[i]);
		if (current.t > ray->tMin && current.t < ray->tMax)
		{
			return true;
		}
	}

	for (int i = 0;i < sphereCount;++i)
	{
		TraceResult current = IntersectSphere(ray, &spheres[i]);
		if (current.t > ray->tMin && current.t < ray->tMax)
		{
			return true;
		}
	}

	for (int i = 0;i < triangleCount;++i)
	{
		TraceResult current = IntersectTriangle(ray, &triangles[i]);
		if (current.t > ray->tMin && current.t < ray->tMax)
		{
			return true;
		}
	}
	*/


	for (int i = 0;i < scene->objectCount;++i)
	{
		bool current = ShadowIntersectMesh(ray, scene, i);
		if (current)
		{
			return true;
		}
	}

	return false;
}


typedef struct 
{
	Ray ray;
	float3 throughPutLight;
	int depth;
}RayTraceData;


float3 Trace(Ray* ray, Scene* scene)
{
	/*
	Plane planes[1];
	planes[0].point = (float3)(0, -20, 0);
	planes[0].normal = (float3)(0, 1, 0);
	planes[0].material.reflectivity = 1.0f;
	planes[0].material.color = (float3)(0.0f, 0.3f, 1.0f);

	Sphere spheres[4];
	spheres[0].radius = 1.0f;
	spheres[0].mid = (float3)(0, 30, 0);
	spheres[0].material.reflectivity = 1.0f;
	spheres[0].material.color = (float3)(0.5f,0.0f,0.5f);

	spheres[1].radius = 2.0f;
	spheres[1].mid = (float3)(17, 8, 0);
	spheres[1].material.reflectivity = 1.0f;
	spheres[1].material.color = (float3)(0.0f, 0.5f, 1.0f);

	spheres[2].radius = 0.8f;
	spheres[2].mid = (float3)(0, 9, -10);
	spheres[2].material.reflectivity = 1.0f;
	spheres[2].material.color = (float3)(0.1f, 0.5f, 0.3f);

	spheres[3].radius = 2.2f;
	spheres[3].mid = (float3)(-11, 8, 0);
	spheres[3].material.reflectivity = 0.5f;
	spheres[3].material.color = (float3)(1.0f, 0.0f, 0.0f);

	Triangle triangles[1];

	triangles[0].p1 = (float3)(3, 3, 0);
	triangles[0].p2 = (float3)(-5, 2, -2);
	triangles[0].p3 = (float3)(6, 1, -4);
	triangles[0].material.reflectivity = 1.0f;
	triangles[0].material.color = (float3)(0.0f,1.0f,0.0f);

	AABB boxes[1];

	boxes[0].min = (float3)(0, 0, 16);
	boxes[0].max = (float3)(1, 1, 17);
	//boxes[0].material.reflectivity = 1.0f;
	//boxes[0].material.color = (float3)(1.0f, 0.0f, 1.0f);
	*/

	//The upper bound of bounces that are allowed
	



	//point light
	//-------------------All light Data-------------------//
	float3 lightPos = (float3)(10,10,0);

	float ambientStength = 0.6f;
	float3 ambientColor = (float3)(0.4f, 0.4f, 0.4f);

	float3 diffuseColor = (float3)(0.8f,0.8f,0.8f);


	

	//base color;
	float3 accumulatedLight = (float3)(0.0f, 0.0f, 0.0f);

	//TraceResult primaryResult = TraceRay(ray, scene); //TraceRay(ray, planes, 1, spheres, 4, triangles, 0, boxes, 1,scene);
	//primaryResult.normal = normalize(primaryResult.normal);

	const int bounceLimit = 3;

	/*
	RayTraceData stackData[bounceLimit * 2];
	int stackCount = 0;

	stackData[0].ray = *ray;
	stackData[0].depth = 0;
	stackData[0].
	stackCount = 1;
	*/

	int bounceCount = 0;

	TraceResult currentResult;
	Ray currentRay = *ray;
	//Ray secondaryRay = *ray;
	float3 throughPutLight = (float3)(1.0f, 1.0f, 1.0f);
	bool hasHitSmth = true;

	while (bounceCount < bounceLimit && hasHitSmth) //One more, becuase there is always a primary Ray
	{
		//Popping our currently analyzed Ray, with its data
		/*
		Ray currentRay = stackData[stackCount - 1].ray;
		int currentDepth = stackData[stackCount - 1].depth;
		float3 currentThroughPutLight = stackData[stackCount - 1].throughPutLight;
		--stackCount;
		*/

		currentResult = TraceRay(&currentRay, scene);

		

		//Check if the currently processed Ray even hit something
		if (currentResult.t > currentRay.tMin && currentResult.t < currentRay.tMax)
		{
			//When we hit something, we collect the hit object's materialData
			int materialIndex = currentResult.materialIndex;
			int textureIndex = scene->materialDatas[materialIndex].textureIndex;
			float3 objectColor = Texture(&currentResult.texCoords, textureIndex, scene).xyz;

			float reflectivity = scene->materialDatas[materialIndex].reflectivity;
			int isRefracting = scene->materialDatas[materialIndex].isRefracting;
			float refractability = scene->materialDatas[materialIndex].refractability;
			float refractIndex = scene->materialDatas[materialIndex].refractIndex;

			currentResult.normal = normalize(currentResult.normal);

			float3 functionalNormal = currentResult.normal;
			if (dot(currentResult.normal, currentRay.direction) > 0.0f)
			{
				functionalNormal *= -1;
				refractIndex = 1.0f / refractIndex;
			}
			

			//We only normalize here, for performance sake
			

			//If the ray hit something, we launch a shadowRay to determine basic illumination
			Ray shadowRay;

			float3 hitPoint = currentRay.origin + currentResult.t * currentRay.direction;

			float3 toLight = lightPos - hitPoint;

			

			float lightDistance = length(toLight);

			toLight /= lightDistance; // now toLight is normalized

			shadowRay.tMin = 0.001f;
			shadowRay.tMax = lightDistance - 0.0001f;
			shadowRay.origin = hitPoint + functionalNormal*0.0001f;
			shadowRay.direction = toLight;

			bool didShadowRayHitSmth = TraceShadowRay(&shadowRay, scene);

			

			float3 diffuseLight = (float3)(0, 0, 0);
			float3 specularLight = (float3)(0, 0, 0);
			float3 ambientLight = (float3)(0, 0, 0);
			if (!didShadowRayHitSmth)
			{
				//Here, the given point is directly illuminated
				//accumulatedColor = (float3)(0, 0, 0);

				float diffuseStrength = max(dot(functionalNormal, toLight), 0.0f);
				diffuseLight =  diffuseStrength * (float3)(1.0f,1.0f,1.0f) ;
				float3 halfDir = normalize(toLight + scene->cameraPosition);
				float specularStrength = pow(max(dot(halfDir, functionalNormal), 0.0f), 32.0f);
				specularLight = specularStrength * (float3)(1.0f, 1.0f, 1.0f);
			}
			
			if (bounceCount == 0)
			{
				ambientLight = ambientStength * ambientColor;
			}
			
			



			float3 shadedLight = (ambientLight + diffuseLight+specularLight);

			
			

			accumulatedLight += throughPutLight * shadedLight * objectColor;// * shadedLight;


			//Here, we will check if according to the bounceLimit constant have we reached the bottom of our stack traversal
			//Or not.
			if (bounceCount < bounceLimit)
			{
				
				bool canRefract = true;
				if (isRefracting != 0)
				{
					//If the material of our object is refracting, then we also send out a refractRay
					Ray refractRay;
					refractRay.tMin = currentRay.tMin;
					refractRay.tMax = currentRay.tMax;
					refractRay.origin = hitPoint - functionalNormal*0.001f;
					
					canRefract = Refract(&currentRay.direction, &functionalNormal, refractIndex,&refractRay.direction);

					if (canRefract)
					{
						throughPutLight *= refractability;

						currentRay = refractRay;
					}
					
					//int refractDepth = currentDepth + 1;
					//float3 refractThroughPutLight = currentThroughPutLight * refractability;
					/*
					stackData[stackCount].ray = refractRay;
					stackData[stackCount].depth = refractDepth;
					stackData[stackCount].throughPutLight = refractThroughPutLight;
					++stackCount;
					*/
				}
				if(isRefracting == 0 || !canRefract)
				{
					//We know, that this ray is an inner ray
					//If this is an inner ray, we can split it into a reflective and a refractive component
					//This split is based on the material properties of the object hit
					Ray reflectRay;
					reflectRay.tMin = currentRay.tMin;
					reflectRay.tMax = currentRay.tMax;
					reflectRay.origin = hitPoint + functionalNormal*0.001f;
					reflectRay.direction = Reflect(&currentRay.direction, &functionalNormal);

					throughPutLight *= reflectivity;

					currentRay = reflectRay;
					//int reflectDepth = currentDepth + 1;
					//float3 reflectThroughPutLight = currentThroughPutLight * reflectivity;
					/*
					stackData[stackCount].ray = reflectRay;
					stackData[stackCount].depth = reflectDepth;
					stackData[stackCount].throughPutLight = reflectThroughPutLight;
					++stackCount;
					*/
				}
			}
			else
			{
				//This is a Leaf Ray, in this case we dont have to do anything
			}
			//throughPutLight *=  reflectivity;
		}
		else
		{
			//We add the black nothingness to the accum raycolor
			//Should be swapped later for hitColor(ray,scene) function or smth
			//We can also implement sky, or cubemaps here
			accumulatedLight += (float3)(0.0f,0.0f,0.0f);
			hasHitSmth = false;
		}

		++bounceCount;



		
	}

	

	

	

	return accumulatedLight;
}


/*
float radians(float degrees)
{
	return degrees / 180.0f * m_PI;
}
*/
/*
typedef struct
{
	__global float3* triangleVertexPositions;
	__global Attribute* triangleVertexAttributes;
	__global int3* triangleIndices;
	__global MeshData* meshDatas;
	__global ObjectData* objectDatas;
	int objectCount;

}Scene;
*/

__kernel void render(write_only image2d_t output,
                     int width,
                     int height,

					 float timePassed,
					 __constant CameraData* cam,

					 __global float3* triangleVertexPositions,
					 __global Attributes* triangleVertexAttributes,
					 __global int3* triangleIndices,
					 __global BvhNode* bvhNodes,
					 __global float4* rgbaValues,
					 __global TextureData* textureDatas,
					 __global MaterialData* materialDatas,
					 __global MeshData* meshDatas,
					 __global ObjectData* objectDatas,
					 int objectCount)
{
	float m_PI = 3.1415926536f;

	
	Scene scene;
	scene.triangleVertexPositions = triangleVertexPositions;
	scene.triangleVertexAttributes = triangleVertexAttributes;
	scene.triangleIndices = triangleIndices;
	scene.bvhNodes = bvhNodes;
	scene.rgbaValues = rgbaValues;
	scene.textureDatas = textureDatas;
	scene.materialDatas = materialDatas;
	scene.meshDatas = meshDatas;
	scene.objectDatas = objectDatas;
	scene.objectCount = objectCount;
	scene.cameraPosition = cam->position.xyz;



	int2 coord = (int2)(get_global_id(0),
						get_global_id(1));

	int2 localCoord = (int2)(get_local_id(0),get_local_id(1));

	if(coord.x >= width || coord.y >= height)
		return;

	float2 uv = (float2)(
		(coord.x) / width,
		(coord.y) / height
	);

	float3 camPos = cam->position.xyz;
	float3 camFront = cam->front.xyz;
	float3 camUp = cam->up.xyz;
	float3 camRight = cam->right.xyz;


	float displayWidth = 2 * tan(radians(cam->fovx) / 2);
	float displayHeight = displayWidth / cam->aspect;

	float unitWidth = displayWidth / width;
	float unitHeight = displayHeight / height;

	float3 worldScreenMid = camPos + camFront; //Assumes that d = 1
	
	float3 leftDownerPixelMid = worldScreenMid + 
								camRight * (displayWidth/2 - unitWidth / 2) -
								camUp * (displayHeight/2 - unitHeight/2);
	

	float3 worldRayPixel = leftDownerPixelMid +
		coord.y * unitHeight * camUp -
		coord.x * unitWidth * camRight;

	Ray primaryRay;
	primaryRay.origin = camPos;
	primaryRay.tMin = cam->zNear;
	primaryRay.tMax = cam->zFar;
	primaryRay.direction = normalize(worldRayPixel - primaryRay.origin);


	//primaryRay.direction = 0.5 * (primaryRay.direction + 1.0f);

	//TraceResult result = TraceRay(&primaryRay, planes, 1,spheres,4,triangles,1,boxes,1);

	

	float4 color = (float4)(Trace(&primaryRay,&scene),1.0f);

	/*
	if (result.t < primaryRay.tMax)
	{
		color = (float4)(result.t/20.0f, 0,0, 1.0f);
	}
	else
	{
		color = (float4)(0, 0, 1, 1.0f);
	}
	*/
	//float4 color = (float4)(primaryRay.direction.x, primaryRay.direction.y, primaryRay.direction.z, 1.0f);
	//float4 color = (float4)(-1*(camRight.x/2), -1*(camRight.y/2), -1*(camRight.z/2), 1.0f);

	write_imagef(output, coord, color);
}