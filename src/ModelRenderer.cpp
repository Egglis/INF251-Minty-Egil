#include "ModelRenderer.h"
#include <globjects/base/File.h>
#include <globjects/State.h>
#include <iostream>
#include <filesystem>
#include <imgui.h>
#include "Viewer.h"
#include "Scene.h"
#include "Model.h"
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace minity;
using namespace gl;
using namespace glm;
using namespace globjects;

ModelRenderer::ModelRenderer(Viewer* viewer) : Renderer(viewer)
{
	m_lightVertices->setStorage(std::array<vec3, 1>({ vec3(0.0f) }), GL_NONE_BIT);
	auto lightVertexBinding = m_lightArray->binding(0);
	lightVertexBinding->setBuffer(m_lightVertices.get(), 0, sizeof(vec3));
	lightVertexBinding->setFormat(3, GL_FLOAT);
	m_lightArray->enable(0);
	m_lightArray->unbind();

	createShaderProgram("model-base", {
		{ GL_VERTEX_SHADER,"./res/model/model-base-vs.glsl" },
		{ GL_GEOMETRY_SHADER,"./res/model/model-base-gs.glsl" },
		{ GL_FRAGMENT_SHADER,"./res/model/model-base-fs.glsl" },
		}, 
		{ "./res/model/model-globals.glsl" });

	createShaderProgram("model-light", {
		{ GL_VERTEX_SHADER,"./res/model/model-light-vs.glsl" },
		{ GL_FRAGMENT_SHADER,"./res/model/model-light-fs.glsl" },
		}, { "./res/model/model-globals.glsl" });
}

bool setup = true;

void ModelRenderer::display()
{
	// Save OpenGL state
	auto currentState = State::currentState();

	// retrieve/compute all necessary matrices and related properties
	const mat4 viewMatrix = viewer()->viewTransform();
	const mat4 inverseViewMatrix = inverse(viewMatrix);
	const mat4 modelViewMatrix = viewer()->modelViewTransform();
	const mat4 inverseModelViewMatrix = inverse(modelViewMatrix);
	const mat4 modelLightMatrix = viewer()->modelLightTransform();
	const mat4 inverseModelLightMatrix = inverse(modelLightMatrix);
	const mat4 modelViewProjectionMatrix = viewer()->modelViewProjectionTransform();
	const mat4 inverseModelViewProjectionMatrix = inverse(modelViewProjectionMatrix);
	const mat4 projectionMatrix = viewer()->projectionTransform();
	const mat4 inverseProjectionMatrix = inverse(projectionMatrix);
	const mat3 normalMatrix = mat3(transpose(inverseModelViewMatrix));
	const mat3 inverseNormalMatrix = inverse(normalMatrix);
	const vec2 viewportSize = viewer()->viewportSize();

	auto shaderProgramModelBase = shaderProgram("model-base");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	viewer()->scene()->model()->vertexArray().bind();

	const std::vector<Group>& groups = viewer()->scene()->model()->groups();
	const std::vector<Material>& materials = viewer()->scene()->model()->materials();

	static std::vector<bool> groupEnabled(groups.size(), true);
	static bool wireframeEnabled = true;
	static bool lightSourceEnabled = false;
	static vec4 wireframeLineColor = vec4(1.0f);

	// Shading Settings
	static bool blinnPhong = true;
	static bool toonShading = false;

	// Lighting Varibles 
	static vec4 ambientColor = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	static vec4 diffuseColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	static vec4 specularColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	static float shine = 1.0f;
	static float ambientIntensity = 1.0f;
	static float specularIntensity = 1.0f;
	static float diffuseIntensity = 1.0f;

	// Light Controls
	static bool manLightPos = false;
	static float lightX = 0.0f;
	static float lightY = 0.0f;
	static float lightZ = 0.0f;

	// Textures:
	static bool diffuseTexture = true;
	static bool ambientTexture = true;
	static bool specularTexture = true;
	static bool normalTexture = false;
	static bool tangentTexture = false;

	// Procedual Bump mapping
	static bool procedualBumpMap = false;
	static float A = 0.001;
	static float k = 100;

	// Reflections/Refractions
	static bool reflectionBool = false;
	static bool refractionBool = false;
	static float n1 = 1.0f;
	static float n2 = 1.33f;
	static float ratio = n1 / n2;
	static const char* items[]{"Air - 1.000","Water - 1.333","Ice - 1.309","Glass - 1.523","Diamond - 2.422"};
	static const float values[]{1.00f, 1.33f, 1.309f, 1.52f, 2.42f};
	static int selectedItem1 = 0;
	static int selectedItem2 = 0;
	static bool onlyReflection = false;
	static bool ambientReflection = false;

	// Animation
	static float explotion = 0.0f;
	static float timeStep = 0.0f;

	unsigned int skyboxTexture = viewer()->scene()->skyboxTexture;

	if (ImGui::BeginMenu("Model"))
	{
		ImGui::Checkbox("Wireframe Enabled", &wireframeEnabled);
		ImGui::Checkbox("Light Source Enabled", &lightSourceEnabled);
		if (wireframeEnabled)
		{
			if (ImGui::CollapsingHeader("Wireframe"))
			{
				ImGui::ColorEdit4("Line Color", (float*)&wireframeLineColor, ImGuiColorEditFlags_AlphaBar);
			}
		}
		if (ImGui::CollapsingHeader("Textures")) {
			ImGui::Checkbox("Diffuse Texture", &diffuseTexture);
			ImGui::Checkbox("Ambient Texture", &ambientTexture);
			ImGui::Checkbox("Specular Texture", &specularTexture);
			ImGui::Checkbox("Normal Texture", &normalTexture);
			ImGui::Checkbox("Tangent Texture", &tangentTexture);
			ImGui::Checkbox("Procedual Bump Mapping", &procedualBumpMap);
			ImGui::SliderFloat("Amplitude: A", &A,0,100);
			ImGui::SliderFloat("Frequnecy: k", &k,0,100);
		}

		if (ImGui::CollapsingHeader("Reflections and Refractions")) {
			ImGui::Checkbox("Reflections", &reflectionBool);
			if (reflectionBool) { refractionBool = false; }
			ImGui::Checkbox("Refractions", &refractionBool);
			if (refractionBool) { 
				reflectionBool = false;  
				ImGui::ListBox("From", &selectedItem1, items, IM_ARRAYSIZE(items), 5);
				ImGui::ListBox("To", &selectedItem2, items, IM_ARRAYSIZE(items), 5);
				n1 = values[selectedItem1];
				n2 = values[selectedItem2];
				ImGui::SliderFloat("Manual: n1", &n1, 0.1f, 10.0f);
				ImGui::SliderFloat("Manual: n2", &n2, 0.1f, 10.0f);
				ratio = n1 / n2;
			}


			ImGui::Checkbox("Only reflections/Refraction", &onlyReflection);
			if (onlyReflection) {ambientReflection = false;}
			ImGui::Checkbox("Ambient reflections/Refraction", &ambientReflection);
			if (ambientReflection) { onlyReflection = false;}
		}

		if (ImGui::CollapsingHeader("Lighting")) {
			ImGui::Checkbox("Blinn Phong Shading", &blinnPhong);
			if (blinnPhong) { toonShading = false; }
			ImGui::Checkbox("Toon Shading", &toonShading);
			if (toonShading) { blinnPhong = false; }


			if (ImGui::CollapsingHeader("Lighting Settings")) {

					ImGui::ColorEdit4("Ambient Color", (float*)&ambientColor, ImGuiColorEditFlags_AlphaBar);
					ImGui::SliderFloat("Ambient Intensity", &ambientIntensity, 0.0f, 100.0f);

					ImGui::ColorEdit4("Diffuse Color", (float*)&diffuseColor, ImGuiColorEditFlags_AlphaBar);
					ImGui::SliderFloat("Diffuse Intensity", &diffuseIntensity, 0.0f, 100.0f);

					ImGui::ColorEdit4("Specular Color", (float*)&specularColor, ImGuiColorEditFlags_AlphaBar);
					ImGui::SliderFloat("Shine", &shine, 0.0f, 200.0f);
					ImGui::SliderFloat("Specular Intensity", &specularIntensity, 0.0f, 100.0f);
			}
			
		}
		
			if (ImGui::CollapsingHeader("Light Position")) {
				ImGui::Checkbox("Manual Light Control", &manLightPos);
				ImGui::SliderFloat("x", &lightX, -15.0f, 15.0f);
				ImGui::SliderFloat("y", &lightY, -15.0f, 15.0f);
				ImGui::SliderFloat("z", &lightZ, -15.0f, 15.0f);
			}
		

		if (ImGui::CollapsingHeader("Groups"))
		{
			for (uint i = 0; i < groups.size(); i++)
			{
				bool checked = groupEnabled.at(i);
				ImGui::Checkbox(groups.at(i).name.c_str(), &checked);
				groupEnabled[i] = checked;
			}

		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Animations")) {
		ImGui::SliderFloat("Explosion: ", &explotion, -1, 5);
		ImGui::SliderFloat("Timeline", &timeStep, 0.0f, 1.0f);
		ImGui::EndMenu();
	}

	vec4 worldCameraPosition = inverseModelViewMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 worldLightPosition = inverseModelLightMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);


	shaderProgramModelBase->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix);
	shaderProgramModelBase->setUniform("modelViewMatrix", modelViewMatrix);
	shaderProgramModelBase->setUniform("viewMatrix", viewMatrix);
	shaderProgramModelBase->setUniform("modelLightMatrix", modelLightMatrix);
	shaderProgramModelBase->setUniform("normalMatrix", normalMatrix);
	shaderProgramModelBase->setUniform("procedualBumpMap", procedualBumpMap);
	shaderProgramModelBase->setUniform("viewportSize", viewportSize);
	shaderProgramModelBase->setUniform("worldCameraPosition", vec3(worldCameraPosition));
	shaderProgramModelBase->setUniform("wireframeEnabled", wireframeEnabled);
	shaderProgramModelBase->setUniform("wireframeLineColor", wireframeLineColor);
	shaderProgramModelBase->setUniform("blinnPhong", blinnPhong);
	shaderProgramModelBase->setUniform("toonShading", toonShading);
	shaderProgramModelBase->setUniform("A", A);
	shaderProgramModelBase->setUniform("k", k);
	shaderProgramModelBase->setUniform("reflectionBool", reflectionBool);
	shaderProgramModelBase->setUniform("onlyReflection", onlyReflection);
	shaderProgramModelBase->setUniform("ambientReflection", ambientReflection);
	shaderProgramModelBase->setUniform("refractionBool", refractionBool);
	shaderProgramModelBase->setUniform("refractionRatio", ratio);
	if (manLightPos) {
		shaderProgramModelBase->setUniform("worldLightPosition", vec3(worldLightPosition) + vec3(lightX, lightY, lightZ));
	}
	else {
		shaderProgramModelBase->setUniform("worldLightPosition", vec3(worldLightPosition));
	}
	
	vec3 centerModel = (viewer()->scene()->model()->maximumBounds() + viewer()->scene()->model()->minimumBounds()) * 0.5f;

	shaderProgramModelBase->use();

	for (uint i = 0; i < groups.size(); i++)
	{
		if (groupEnabled.at(i))
		{
			const Material& material = materials.at(groups.at(i).materialIndex);

			if (setup) {
				diffuseColor = vec4(material.diffuse, 1.0f);
				ambientColor = vec4(material.ambient, 1.0f);
				specularColor = vec4(material.specular, 1.0f);
				shine = material.shininess;
				if (material.diffuseTexture) diffuseTexture = true;
				if (material.ambientTexture) ambientTexture = true;
				if (material.specularTexture) specularTexture = true;
				if (material.normalTexture) normalTexture = true;
				setup = false;
			}

			// Blin Phong
			shaderProgramModelBase->setUniform("diffuseColor", diffuseColor);
			shaderProgramModelBase->setUniform("ambientColor", ambientColor);
			shaderProgramModelBase->setUniform("specularColor", specularColor);
			shaderProgramModelBase->setUniform("specularExponent", shine);

			shaderProgramModelBase->setUniform("diffuseIntensity", diffuseIntensity);
			shaderProgramModelBase->setUniform("ambientIntensity", ambientIntensity);
			shaderProgramModelBase->setUniform("specularIntensity", specularIntensity);

			shaderProgramModelBase->setUniform("diffuseTexBool", diffuseTexture);
			shaderProgramModelBase->setUniform("ambientTexBool", ambientTexture);
			shaderProgramModelBase->setUniform("specularTexBool", specularTexture);
			shaderProgramModelBase->setUniform("normalTexBool", normalTexture);
			shaderProgramModelBase->setUniform("tangentTexBool", tangentTexture);

			
			// Get Direction from center of mass, and "push" 


			mat4 trans = translate(mat4(1.0f), (groups.at(i).centerMass-centerModel)*vec3(explotion));

			shaderProgramModelBase->setUniform("explotion", explotion);
			shaderProgramModelBase->setUniform("transformation", trans);


			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

			if (material.diffuseTexture && diffuseTexture)
			{
				//diffuseTexture = true;
				shaderProgramModelBase->setUniform("diffuseTexture", 6);
				material.diffuseTexture->bindActive(6);
			}
			if (material.ambientTexture && ambientTexture)
			{
				shaderProgramModelBase->setUniform("ambientTexture", 1);
				material.ambientTexture->bindActive(1);

			}
			if (material.specularTexture && specularTexture)
			{
				shaderProgramModelBase->setUniform("specularTexture", 2);
				material.specularTexture->bindActive(2);
			}

			if (material.normalTexture && normalTexture) {
				shaderProgramModelBase->setUniform("normalTexture", 3);
				material.normalTexture->bindActive(3);
			}
			if (material.tangentTexture) {
				shaderProgramModelBase->setUniform("tangentTexture", 4);
				material.tangentTexture->bindActive(4);
			}

			viewer()->scene()->model()->vertexArray().drawElements(GL_TRIANGLES, groups.at(i).count(), GL_UNSIGNED_INT, (void*)(sizeof(GLuint)*groups.at(i).startIndex));


			if (material.diffuseTexture)
			{
				material.diffuseTexture->unbind();
			}
			if (material.ambientTexture)
			{
				material.ambientTexture->unbind();
			}
			if (material.specularTexture)
			{
				material.specularTexture->unbind();
			}
			if (material.normalTexture) {
				material.normalTexture->unbind();
			}
			if (material.tangentTexture) {
				material.tangentTexture->unbind();
			}

		}
	}


	shaderProgramModelBase->release();

	viewer()->scene()->model()->vertexArray().unbind();


	if (lightSourceEnabled)
	{
		auto shaderProgramModelLight = shaderProgram("model-light");

		// Checks if manual light controlls are enabled
		if (manLightPos) {
			shaderProgramModelLight->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix * translate((inverseModelLightMatrix), vec3(lightX, lightY, lightZ)));
		}
		else {
			shaderProgramModelLight->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix* inverseModelLightMatrix);

		}
		shaderProgramModelLight->setUniform("viewportSize", viewportSize);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		m_lightArray->bind();

		shaderProgramModelLight->use();
		m_lightArray->drawArrays(GL_POINTS, 0, 1);
		shaderProgramModelLight->release();

		m_lightArray->unbind();

		glDisable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	// Restore OpenGL state (disabled to to issues with some Intel drivers)
	// currentState->apply();

}


