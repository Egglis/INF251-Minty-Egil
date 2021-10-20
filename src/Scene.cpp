#include "Scene.h"
#include "Model.h"
#include <iostream>

using namespace minity;

Scene::Scene()
{
	m_model = std::make_unique<Model>();
	m_skybox = std::make_unique<Model>();
}

Model * Scene::model()
{
	return m_model.get();
}

Model* Scene::skybox() {
	return m_skybox.get();
}