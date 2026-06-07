#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal; 
layout(location = 2) in vec2 aTextureCoords;


uniform mat4 uWorldTransform;
uniform mat4 uInverseTransposedWorldTransform;
uniform mat4 uViewTransform;
uniform mat4 uProjectionTransform;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTextureCoords;


void main()
{
    gl_Position = uProjectionTransform  * uViewTransform * uWorldTransform * vec4(aPosition.xyz,1.0);
    vPosition = vec3(uWorldTransform * vec4(aPosition.xyz,1.0));
    vNormal = vec3(uInverseTransposedWorldTransform *  vec4(aNormal,0.0f));//just worldTransform for now
    vTextureCoords = aTextureCoords;
    
};