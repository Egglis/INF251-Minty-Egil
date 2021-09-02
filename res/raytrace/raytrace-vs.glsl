#version 400
#extension GL_ARB_shading_language_include : require
#include "/raytrace-globals.glsl"

in vec2 position;
out vec2 fragPosition;

out vec2 pp;
void main()
{
	fragPosition = position;
	gl_Position = vec4(position,0.0,1.0);
}