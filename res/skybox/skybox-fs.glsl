#version 400
#extension GL_ARB_shading_language_include : require

uniform vec3 worldCameraPosition;
uniform vec3 worldLightPosition;
uniform bool wireframeEnabled;
uniform vec4 wireframeLineColor;
uniform mat3 normalMatrix;
uniform sampler2D ambientTexture;


in fragmentData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	noperspective vec3 edgeDistance;
} fragment;

out vec4 fragColor;


// Main function
void main()
{	
	vec4 result = vec4(0.5,0.5,0.5,1.0);

	if (wireframeEnabled)
	{
		float smallestDistance = min(min(fragment.edgeDistance[0],fragment.edgeDistance[1]),fragment.edgeDistance[2]);
		float edgeIntensity = exp2(-1.0*smallestDistance*smallestDistance);
		result.rgb = mix(result.rgb,wireframeLineColor.rgb,edgeIntensity*wireframeLineColor.a);
		fragColor = result;
	}

	result.rgba = texture(ambientTexture,fragment.texCoord).rgba*2 -1;

	

	fragColor = result;
}













