#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 transformations[100];

void main(){

	gl_Position = uProjectionMatrix * uModelViewMatrix * transformations[gl_InstanceID] * vec4(aPosition, 1);

}