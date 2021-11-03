#pragma once

#include <memory>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include "Scene.h"
#include "Interactor.h"
#include "Renderer.h"

namespace minity
{
	struct KeyFrame {
		glm::vec3 backgroundColor;

		glm::mat4 c_rotate;
		glm::vec3 c_translate;
		glm::vec3 c_scale;

		glm::mat4 l_rotate;
		glm::vec3 l_translate;
		glm::vec3 l_scale;

		glm::vec3 explosion;
	};

	class Viewer
	{
	public:
		Viewer(GLFWwindow* window, Scene* scene);
		~Viewer();

		void display();

		GLFWwindow * window();
		Scene* scene();

		glm::ivec2 viewportSize() const;

		glm::vec3 backgroundColor() const;
		glm::mat4 modelTransform() const;
		glm::mat4 viewTransform() const;
		glm::mat4 lightTransform() const;
		glm::mat4 projectionTransform() const;

		void setBackgroundColor(const glm::vec3& c);
		void setViewTransform(const glm::mat4& m);
		void setModelTransform(const glm::mat4& m);
		void setLightTransform(const glm::mat4& m);
		void setProjectionTransform(const glm::mat4& m);

		glm::mat4 modelViewTransform() const;
		glm::mat4 modelViewProjectionTransform() const;

		glm::mat4 modelLightTransform() const;
		glm::mat4 modelLightProjectionTransform() const;

		std::vector<KeyFrame> getKeyFrames();
		bool isAnimationOn();
		void addFrame(KeyFrame frame);
		void removeFrame();


		void saveImage(const std::string & filename);

		glm::vec3 m_explosion;


	private:

		void beginFrame();
		void endFrame();
		void renderUi();
		void mainMenu();

		static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		GLFWwindow* m_window;
		Scene *m_scene;

		std::vector<std::unique_ptr<Interactor>> m_interactors;
		std::vector<std::unique_ptr<Renderer>> m_renderers;

		int currentFrame = 0;

		glm::vec3 m_backgroundColor = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::mat4 m_modelTransform = glm::mat4(1.0f);
		glm::mat4 m_viewTransform = glm::mat4(1.0f);
		glm::mat4 m_projectionTransform = glm::mat4(1.0f);
		glm::mat4 m_lightTransform = glm::mat4(1.0f);
		glm::vec4 m_viewLightPosition = glm::vec4(0.0f, 0.0f,-sqrt(3.0f),1.0f);

		std::vector<KeyFrame> m_keyFrames = std::vector<KeyFrame>();
		bool m_playAnimation = false;

		bool m_showUi = true;
		bool m_saveScreenshot = false;
	};

	/**
	 * @brief Performs standard decomposition of a transformation matrix into translation, rotation and scale parts respectively.
	 * Note: The out rotation is an affine space rotation matrix, meaning only the inner 3x3 part affects rotation and the rest is the identity matrix.
	 * @see https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
	 * @param matrix The matrix to decompose
	 * @param translation Out parameter for translation
	 * @param rotation Out parameter for rotation
	 * @param scale Out parameter for scale
	 */
	void matrixDecompose(const glm::mat4& matrix, glm::vec3& translation, glm::mat4& rotation, glm::vec3& scale, bool preMultipliedRotation);
}