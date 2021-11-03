#pragma once
#include "Renderer.h"
#include "Viewer.h"
#include <memory>

#include <glm/glm.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Framebuffer.h>
#include <globjects/Renderbuffer.h>
#include <globjects/Texture.h>
#include <globjects/base/File.h>
#include <globjects/TextureHandle.h>
#include <globjects/NamedString.h>
#include <globjects/base/StaticStringSource.h>

namespace minity
{
	class Viewer;

	class ModelRenderer : public Renderer
	{
	public:
		ModelRenderer(Viewer *viewer);
		virtual void display();
		glm::vec3 catmullRom(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
		glm::quat catmullRom(float t, glm::quat p0, glm::quat p1, glm::quat p2, glm::quat p3);
	private:
		std::unique_ptr<globjects::VertexArray> m_lightArray = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_lightVertices = std::make_unique<globjects::Buffer>();
	};

}