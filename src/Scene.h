#pragma once

#include <memory>

namespace minity
{
	class Model;

	class Scene
	{
	public:
		Scene();
		Model* model();
		Model* skybox();
		unsigned int skyboxTexture;
	private:
		std::unique_ptr<Model> m_model;
		std::unique_ptr<Model> m_skybox;
	};


}