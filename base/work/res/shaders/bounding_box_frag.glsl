#version 330 core

out vec4 color;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

void main(){
	color = vec4(0.0,0.0,0.0,1.0);
}