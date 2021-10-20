#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"

uniform mat4 modelViewProjectionMatrix;
uniform vec2 viewportSize;

in vec3 position;
flat out vec2 pointCenter;
flat out float pointSize;

void main()
{
	const float size = 27.0;

	vec4 pos = modelViewProjectionMatrix*vec4(position,1.0);
	
	pointCenter.xy = ((pos.xy / pos.w)+vec2(1.0))*viewportSize*0.5;
	pointSize = size;

	gl_Position = pos;
	gl_PointSize = size;
	
}