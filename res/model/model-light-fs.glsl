#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"

uniform vec2 viewportSize;

flat in vec2 pointCenter;
flat in float pointSize;
out vec4 fragColor;

void main()
{
	float centerDistance = sqrt(4.0)*length(gl_FragCoord.xy-pointCenter)/pointSize;
	fragColor = vec4(1.0-centerDistance);

}
