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

	private:
		std::unique_ptr<Model> m_model;
		std::unique_ptr<Model> m_skybox;

	};


}