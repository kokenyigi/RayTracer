#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uWorldTransform;
uniform mat4 uViewTransform;
uniform mat4 uProjectionTransform;

void main()
{
    gl_Position = uProjectionTransform  * uViewTransform * uWorldTransform * vec4(aPosition.xyz,1.0); 
};