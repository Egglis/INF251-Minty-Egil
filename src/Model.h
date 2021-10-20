#pragma once

#include <glm/glm.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <globjects/Texture.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>

#include <vector>

namespace minity
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};

	struct Group
	{
		std::string name;
		glm::uint materialIndex = 0;
		glm::uint startIndex = 0;
		glm::uint endIndex = 0;
		
		glm::uint count() const
		{
			return endIndex - startIndex + 1;
		}

	};

	struct Material
	{
		std::string name;
		glm::vec3 ambient = glm::vec3(0.0f);
		glm::vec3 diffuse = glm::vec3(0.0f);
		glm::vec3 specular = glm::vec3(0.0f);
		float shininess = 0.0f;

		std::shared_ptr<globjects::Texture> ambientTexture;
		std::shared_ptr<globjects::Texture> diffuseTexture;
		std::shared_ptr<globjects::Texture> specularTexture;
		std::shared_ptr<globjects::Texture> shininessTexture;
		std::shared_ptr<globjects::Texture> bumpTexture;
	};

	class Model
	{
	public:
		Model();
		Model(const std::string& filename);
		void load(const std::string& filename);
		const std::string & filename() const;

		const std::vector<Group> & groups() const;
		const std::vector<Vertex> & vertices() const;
		const std::vector<glm::uint> & indices() const;
		const std::vector<Material> & materials() const;

		glm::vec3 minimumBounds() const;
		glm::vec3 maximumBounds() const;

		globjects::VertexArray & vertexArray();
		globjects::Buffer & vertexBuffer();
		globjects::Buffer & indexBuffer();

	private:

		std::string m_filename;
		
		std::vector < Group > m_groups;
		std::vector < Vertex > m_vertices;
		std::vector < glm::uint > m_indices;
		std::vector < Material > m_materials;

		glm::vec3 m_minimumBounds = glm::vec3(0.0);
		glm::vec3 m_maximumBounds = glm::vec3(0.0);

		std::unique_ptr<globjects::VertexArray> m_vertexArray = std::make_unique<globjects::VertexArray>();
		std::unique_ptr<globjects::Buffer> m_vertexBuffer = std::make_unique<globjects::Buffer>();
		std::unique_ptr< globjects::Buffer > m_indexBuffer = std::make_unique<globjects::Buffer>();

	};
}