#version 450 core

in vec3 VertexLastColor;

out vec3 color;

void main(){
	color = VertexLastColor;
}