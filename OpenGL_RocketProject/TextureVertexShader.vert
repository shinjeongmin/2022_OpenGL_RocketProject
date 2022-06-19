#version 450 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexInUV;

out vec2 VertexOutUV;

uniform mat4 MVP;

void main() 
{
	gl_Position =  MVP * vec4(vertexPosition_modelspace, 1.0f);

	VertexOutUV = vertexInUV;
}