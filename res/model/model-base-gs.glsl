#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec2 viewportSize;
uniform sampler2D tangentTexture;
uniform sampler2D normalTexture;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelLightMatrix;
uniform mat3 normalMatrix;
uniform vec3 worldCameraPosition;
uniform vec3 worldLightPosition;

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
	vec3 tangent;
	noperspective vec3 edgeDistance;
} fragment;


vec3 calculateTangent(vec3 edge1, vec3 edge2, vec2 deltaUV1, vec2 deltaUV2){
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	vec3 tangent;
	tangent.x = f * (edge1.x * deltaUV2.y - edge2.x * deltaUV1.y);
	tangent.y = f * (edge1.y * deltaUV2.y - edge2.y * deltaUV1.y);
	tangent.z = f * (edge1.z * deltaUV2.y - edge2.z * deltaUV1.y);
	return tangent;
}

void main(void)
{
	vec2 p[3];
	vec2 v[3];

	vec3 tangent1; 
	vec3 tangent2; 
	vec3 tangent3; 
	vec3 bitangent;


	for (int i=0;i<3;i++)
		p[i] = 0.5 * viewportSize *  gl_in[i].gl_Position.xy/gl_in[i].gl_Position.w;

	v[0] = p[2]-p[1];
	v[1] = p[2]-p[0];
	v[2] = p[1]-p[0];

	vec3 v0 = vertices[0].position;
	vec3 v1 = vertices[1].position;
	vec3 v2 = vertices[2].position;

	vec2 st0 = vertices[0].texCoord;
	vec2 st1 = vertices[1].texCoord;
	vec2 st2 = vertices[2].texCoord;

	tangent1 = calculateTangent(v1 - v0, v2 - v0, st1 - st0, st2 - st0);

	float area = abs(v[1].x*v[2].y - v[1].y * v[2].x);	


	for (int i=0;i<3;i++)
	{
		gl_Position = gl_in[i].gl_Position;
		fragment.position = vertices[i].position;
		fragment.normal = vertices[i].normal;
		fragment.texCoord = vertices[i].texCoord;
		fragment.tangent = tangent1;
		vec3 ed = vec3(0.0);
		ed[i] = area / length(v[i]);
		fragment.edgeDistance = ed;

		EmitVertex();
	}

	EndPrimitive();
}