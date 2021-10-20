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

	private:
		std::unique_ptr<Model> m_model;
	};


}