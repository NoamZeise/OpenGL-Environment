#ifndef APP_H
#define APP_H

#include <atomic>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <audio.h>
#include <input.h>
#include <timer.h>
#include <glmhelper.h>

#include "opengl-render/render.h"
#include "opengl-render/resources/resources.h"
#include <config.h>
#include "camera.h"

//#define TIME_APP_DRAW_UPDATE
//#define MULTI_UPDATE_ON_SLOW_DRAW

class App
{
public:
	App();
	~App();
	void run();
	void resize(int windowWidth, int windowHeight);

#pragma region GLFW_CALLBACKS
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void error_callback(int error, const char* description);
#pragma endregion
	Input input;

private:
	void loadAssets();
	void update();
	void postUpdate();
	void draw();

	glm::vec2 correctedPos(glm::vec2 pos);
	glm::vec2 correctedMouse();

	const int INITIAL_WINDOW_WIDTH = 1000;
  const int INITIAL_WINDOW_HEIGHT = 700;

	GLFWwindow* window;
	Render* render;
	int windowWidth, windowHeight;
	Input previousInput;
	Timer timer;
	Camera::FirstPerson cam3d;
	Audio::Manager audioManager;

        Resource::Texture testTex;
    Resource::Font testFont;
    Resource::Model testModel;
};

#endif
