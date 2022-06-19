#version 450 core
in vec2 VertexOutUV;

out vec3 color;

uniform sampler2D TextureSampler;

void main()
{
	color = texture(TextureSampler, VertexOutUV).rgb;
}