#version 450 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertex_color;

out vec3 VertexLastColor;

void main(){
	gl_Position.xyz = vertexPosition_modelspace;
	gl_Position.w = 1.0f;

	VertexLastColor = vertex_color;
}