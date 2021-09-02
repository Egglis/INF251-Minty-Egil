#pragma once
#include "Renderer.h"
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

	class RaytraceRenderer : public Renderer
	{
	public:
		RaytraceRenderer(Viewer *viewer);
		virtual void display();

	private:
		std::unique_ptr<globjects::VertexArray> m_quadArray = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_quadVertices = std::make_unique<globjects::Buffer>();
	};

}