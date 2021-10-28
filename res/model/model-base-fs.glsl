#version 400
#extension GL_ARB_shading_language_include : require
#include "/model-globals.glsl"


uniform vec3 worldCameraPosition;
uniform vec3 worldLightPosition;
uniform bool wireframeEnabled;
uniform vec4 wireframeLineColor;
uniform mat3 normalMatrix;


// Texture
uniform sampler2D diffuseTexture;
uniform sampler2D ambientTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;
uniform sampler2D tangentTexture;
uniform bool diffuseTexBool;
uniform bool ambientTexBool;
uniform bool specularTexBool;
uniform bool normalTexBool;
uniform bool tangentTexBool;

// Procedual Bump mapping
uniform bool procedualBumpMap;
uniform float A;
uniform float k;


// Shading type
uniform bool blinnPhong;
uniform bool toonShading;


// Blinn-Phong Shading
uniform vec4 ambientColor;
uniform vec4 specularColor;
uniform vec4 diffuseColor;
uniform float specularExponent;
uniform float diffuseIntensity;
uniform float ambientIntensity;
uniform float specularIntensity;

// Skybox and reflections/refractions
uniform samplerCube skybox;
uniform bool reflectionBool;
uniform bool onlyReflection;
uniform bool ambientReflection;
uniform bool refractionBool;
uniform float refractionRatio;

in fragmentData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	vec3 tangent;
	noperspective vec3 edgeDistance;
} fragment;

out vec4 fragColor;

// Calculates the new intesities based on a fixed gradient
float toonIntensityCalc(float intensity){
		float gradient = 0.0f;
		if (intensity > 0.97) {
			gradient = 1.0f;
		} else if (intensity > 0.5) {
			gradient = 0.6f;
		} else if (intensity > 0.25) {
			gradient = 0.4f;
		} else if (intensity > 0.10) {
			gradient = 0.2f;
		} else {
			gradient = 0.0f;
		}
	return gradient;
}

vec4 calculateModel(vec3 lightDirection, vec3 viewDirection, vec3 normal){
	vec4 result = vec4(0.5,0.5,0.5,1.0);

	// Skybox reflections/refractions
	vec4 reflectionColor = vec4(1.0f);
	vec3 I = normalize(fragment.position - worldCameraPosition);
	vec3 R = vec3(1.0f);
	if(reflectionBool) {
		R = reflect(I, normalize(normal));
	} else if (refractionBool) {
		R = refract(I, normalize(normal),refractionRatio);
	}
	reflectionColor = vec4(texture(skybox, R).rgb, 1.0);

	// Ambient
	vec4 ambient = ambientColor * ambientIntensity;

	// Diffuse
	float shading = dot(lightDirection, normal); 
	vec4 diffuse = diffuseColor * shading * diffuseIntensity;

	// Specular
	float specular_value = 0.0f;
	if(shading > 0.0){
		vec3 reflection = reflect(-lightDirection, normal);
		vec3 cameraVector = normalize(viewDirection);
		vec3 H = normalize(lightDirection+cameraVector);

		float specAngle = 0.0f;
		specAngle = max(dot(normal,H),0.0);
		specular_value = pow(specAngle, specularExponent);
	}
	vec4 specular = specularColor * specular_value * specularIntensity;

	// Shading Models
	if(toonShading){
		ambient = ambientColor * toonIntensityCalc(ambientIntensity);
		diffuse = diffuseColor * toonIntensityCalc(shading) * diffuseIntensity;
		specular = specularColor * toonIntensityCalc(specular_value) * specularIntensity;
	} else if (blinnPhong) {
		ambient = ambientColor * ambientIntensity;
		diffuse = diffuseColor * shading * diffuseIntensity;
		specular = specularColor * specular_value * specularIntensity;
	}

	// Texturing
	vec4 diffuseTextureColor = vec4(1,1,1,1);
	vec4 ambientTextureColor = vec4(1,1,1,1);
	vec4 specularTextureColor = vec4(1,1,1,1);
	
	if(diffuseTexBool)	diffuseTextureColor = texture(diffuseTexture, fragment.texCoord);
	if(ambientTexBool)	ambientTextureColor = texture(ambientTexture, fragment.texCoord);
	if(specularTexBool) specularTextureColor = texture(specularTexture, fragment.texCoord);

	if(ambientReflection) {
		result = diffuse*diffuseTextureColor + specular*specularTextureColor + reflectionColor;
		return result;
	}
	result = diffuse*diffuseTextureColor + specular*specularTextureColor + ambient*ambientTextureColor;
	return result;
}

// Partial derivative of function on u
float bu(float u, float v){
	return A*k*pow(sin(k*v),2)*sin(2*k*u);
}

// Partial derivative of function on v
float bv(float u, float v) {
	return A*k*pow(sin(k*u),2)*sin(2*k*v);
}


// Calcultes the new normal after procedual bump mapping
vec3 doProcedualBumpMapping(vec3 normal, vec3 tangent, vec3 bitangent){
	vec3 newNormal;
	vec2 pos = vec2(fragment.texCoord.x, fragment.texCoord.y);

	newNormal = normal + bu(pos.x, pos.y) * cross(tangent, normal) + bv(pos.x,pos.y) * cross(normal, bitangent);
	return newNormal;
}


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

	vec3 normal = normalize(fragment.normal);
	vec3 lightDirection = normalize(worldLightPosition - fragment.position);
	vec3 viewDirection = normalize(worldCameraPosition - fragment.position);

	vec3 tangent = normalize(fragment.tangent);
	vec3 bitangent = cross(tangent, fragment.normal);

	// Normal Texture
	if(normalTexBool) {
		normal = texture(normalTexture, fragment.texCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0);
	// Tangent Texture
	} 
	if (tangentTexBool) {
		normal = normalize(texture(normalTexture, fragment.texCoord).rgb * 2 - 1);
		vec3 texNormal = texture(tangentTexture, fragment.texCoord).rgb * 2 - 1;
		vec3 newNormal;
		mat3 TBN = mat3(tangent, bitangent, normal);
		newNormal = TBN * texNormal;
		newNormal = normalize(newNormal);

		normal = newNormal;
	}

	// Procedual Bump Mapping
	if(procedualBumpMap){
		vec3 newNormal = doProcedualBumpMapping(normal, tangent, bitangent);
		normal = newNormal;
	}

	// Only Reflections/Refraction
	if(onlyReflection){
		vec3 I = normalize(fragment.position - worldCameraPosition);
		vec3 R = vec3(1.0f);
		if(reflectionBool) {
			R = reflect(I, normalize(normal));
		} else if (refractionBool) {
			R = refract(I, normalize(normal),refractionRatio);
		}
		vec4 reflectionColor = vec4(texture(skybox, R).rgb, 1.0);
		result = reflectionColor;
		fragColor = result;
		return;
	}

	result.rgba = calculateModel(lightDirection, viewDirection, normal);
	fragColor = result;
}













