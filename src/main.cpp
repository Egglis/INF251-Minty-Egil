#include <iostream>

#include <glbinding/Version.h>
#include <glbinding/Binding.h>
#include <glbinding/FunctionCall.h>
#include <glbinding/CallbackMask.h>

#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/Meta.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/ValidVersions.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <tinyfiledialogs.h>

#include "Scene.h"
#include "Model.h"
#include "Viewer.h"
#include "Interactor.h"
#include "Renderer.h"

#include <stb_image.h>


using namespace gl;
using namespace glm;
using namespace globjects;
using namespace minity;

void error_callback(int errnum, const char * errmsg)
{
	globjects::critical() << errnum << ": " << errmsg << std::endl;
}


unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	stbi_set_flip_vertically_on_load(false);
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			std::cout << "Loading: " << faces[i] << std::endl;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

int main(int argc, char *argv[])
{
	// Initialize GLFW
	if (!glfwInit())
		return 1;

	glfwSetErrorCallback(error_callback);

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Create a context and, if valid, make it current
	GLFWwindow * window = glfwCreateWindow(1280, 720, "minity", NULL, NULL);

	if (window == nullptr)
	{
		globjects::critical() << "Context creation failed - terminating execution.";

		glfwTerminate();
		return 1;
	}

	// Make context current
	glfwMakeContextCurrent(window);

	// Initialize globjects (internally initializes glbinding, and registers the current context)
	globjects::init([](const char * name) {
		return glfwGetProcAddress(name);
	});

	// Enable debug logging
	globjects::DebugMessage::enable();
	
	globjects::debug()
		<< "OpenGL Version:  " << glbinding::aux::ContextInfo::version() << std::endl
		<< "OpenGL Vendor:   " << glbinding::aux::ContextInfo::vendor() << std::endl
		<< "OpenGL Renderer: " << glbinding::aux::ContextInfo::renderer() << std::endl;

	std::string fileName = "./dat/bunny.obj";

	if (argc > 1)
		fileName = std::string(argv[1]);
	else
	{
		const char *filterExtensions[] = { "*.obj" };
		const char *openfileName = tinyfd_openFileDialog("Open File", "./", 1, filterExtensions, "Wavefront Files (*.obj)", 0);

		if (openfileName)
			fileName = std::string(openfileName);
	}

	std::vector<std::string> faces = {
		".\\res\\skyboxtex\\right.jpg",
		".\\res\\skyboxtex\\left.jpg",
		".\\res\\skyboxtex\\top.jpg",
		".\\res\\skyboxtex\\bottom.jpg",
		".\\res\\skyboxtex\\front.jpg",
		".\\res\\skyboxtex\\back.jpg",
	};

	auto scene = std::make_unique<Scene>();
	scene->model()->load(fileName);
	scene->skybox()->load(".\\res\\skyboxtex\\cube.obj");
	scene->skyboxTexture = loadCubemap(faces);
	auto viewer = std::make_unique<Viewer>(window, scene.get());

	// Scaling the model's bounding box to the canonical view volume
	vec3 boundingBoxSize = scene->model()->maximumBounds() - scene->model()->minimumBounds();
	float maximumSize = std::max( std::max(boundingBoxSize.x, boundingBoxSize.y), boundingBoxSize.z );
	mat4 modelTransform =  scale(vec3(2.0f) / vec3(maximumSize)); 
	modelTransform = modelTransform * translate(-0.5f*(scene->model()->minimumBounds() + scene->model()->maximumBounds()));
	viewer->setModelTransform(modelTransform);

	glfwSwapInterval(0);

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		viewer->display();
		//glFinish();
		glfwSwapBuffers(window);
	}

	// Destroy window
	glfwDestroyWindow(window);

	// Properly shutdown GLFW
	glfwTerminate();

	return 0;
}


