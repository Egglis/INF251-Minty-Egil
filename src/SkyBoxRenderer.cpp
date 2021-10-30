#include "SkyBoxRenderer.h"
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

#include <stb_image.h>


using namespace minity;
using namespace gl;
using namespace glm;
using namespace globjects;

SkyBoxRenderer::SkyBoxRenderer(Viewer* viewer) : Renderer(viewer)
{
	m_lightVertices->setStorage(std::array<vec3, 1>({ vec3(0.0f) }), GL_NONE_BIT);
	auto lightVertexBinding = m_lightArray->binding(0);
	lightVertexBinding->setBuffer(m_lightVertices.get(), 0, sizeof(vec3));
	lightVertexBinding->setFormat(3, GL_FLOAT);
	m_lightArray->enable(0);
	m_lightArray->unbind();

	createShaderProgram("skybox", {
		{ GL_VERTEX_SHADER,"./res/skybox/skybox-vs.glsl" },
		{ GL_GEOMETRY_SHADER,"./res/skybox/skybox-gs.glsl" },
		{ GL_FRAGMENT_SHADER,"./res/skybox/skybox-fs.glsl" }, });
	
}


void SkyBoxRenderer::display()
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

	auto shaderProgramModelBase = shaderProgram("skybox");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	viewer()->scene()->skybox()->vertexArray().bind();

	const std::vector<Group>& groups = viewer()->scene()->skybox()->groups();
	const std::vector<Material>& materials = viewer()->scene()->skybox()->materials();

	static std::vector<bool> groupEnabled(groups.size(), true);
	static bool wireframeEnabled = true;
	static vec4 wireframeLineColor = vec4(1.0f);
	static float scaleSkybox = 10.0f;

	unsigned int skyboxTexture = viewer()->scene()->skyboxTexture;


	if (ImGui::BeginMenu("SkyBox"))
	{
		ImGui::Checkbox("Wireframe Enabled", &wireframeEnabled);
		ImGui::SliderFloat("Skybox Scale", &scaleSkybox, 0.1f, 200.0f);
		if (wireframeEnabled)
		{
			if (ImGui::CollapsingHeader("Wireframe"))
			{
				ImGui::ColorEdit4("Line Color", (float*)&wireframeLineColor, ImGuiColorEditFlags_AlphaBar);
			}
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

	vec4 worldCameraPosition = inverseModelViewMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 worldLightPosition = inverseModelLightMatrix * vec4(0.0f, 0.0f, 0.0f, 1.0f);


	shaderProgramModelBase->setUniform("modelViewProjectionMatrix", modelViewProjectionMatrix);
	shaderProgramModelBase->setUniform("modelViewMatrix", modelViewMatrix*0.001f);
	shaderProgramModelBase->setUniform("viewMatrix", viewMatrix);
	shaderProgramModelBase->setUniform("modelLightMatrix", modelLightMatrix);
	shaderProgramModelBase->setUniform("normalMatrix", normalMatrix);

	shaderProgramModelBase->setUniform("viewportSize", viewportSize);
	shaderProgramModelBase->setUniform("worldCameraPosition", vec3(worldCameraPosition));

	shaderProgramModelBase->setUniform("worldLightPosition", vec3(worldLightPosition));
	
	shaderProgramModelBase->setUniform("wireframeEnabled", wireframeEnabled);
	shaderProgramModelBase->setUniform("wireframeLineColor", wireframeLineColor);



	shaderProgramModelBase->use();

	for (uint i = 0; i < groups.size(); i++)
	{
		if (groupEnabled.at(i))
		{
			const Material& material = materials.at(groups.at(i).materialIndex);

			mat4 scaleMat = scale(mat4(1.0f), vec3(scaleSkybox));
			shaderProgramModelBase->setUniform("scale", scaleMat);

			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

			viewer()->scene()->skybox()->vertexArray().drawElements(GL_TRIANGLES, groups.at(i).count(), GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * groups.at(i).startIndex));
			

		}
	}


	shaderProgramModelBase->release();

	viewer()->scene()->skybox()->vertexArray().unbind();


}

