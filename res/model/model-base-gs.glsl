#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec2 viewportSize;

in vertexData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
} vertices[];

out fragmentData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	noperspective vec3 edgeDistance;
} fragment;

void main(void)
{
	vec2 p[3];
	vec2 v[3];

	for (int i=0;i<3;i++)
		p[i] = 0.5 * viewportSize *  gl_in[i].gl_Position.xy/gl_in[i].gl_Position.w;

	v[0] = p[2]-p[1];
	v[1] = p[2]-p[0];
	v[2] = p[1]-p[0];

	float area = abs(v[1].x*v[2].y - v[1].y * v[2].x);

	for (int i=0;i<3;i++)
	{
		gl_Position = gl_in[i].gl_Position;
		fragment.position = vertices[i].position;
		fragment.normal = vertices[i].normal;
		fragment.texCoord = vertices[i].texCoord;
		
		vec3 ed = vec3(0.0);
		ed[i] = area / length(v[i]);
		fragment.edgeDistance = ed;

		EmitVertex();
	}

	EndPrimitive();
}