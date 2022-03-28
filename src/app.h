#ifndef APP_H
#define APP_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <thread>
#include <atomic>

#include <input.h>
#include <glmhelper.h>

#include "opengl-render/render.h"
#include "opengl-render/resources.h"
#include "opengl-render/shader.h"

//#define TIME_APP_DRAW_UPDATE
//#define MULTI_UPDATE_ON_SLOW_DRAW

const bool FIXED_RATIO = false;
const int TARGET_WIDTH = 1920;
const int TARGET_HEIGHT = 1080;

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
  Input previousInput;
private:
	void loadAssets();
	void update();
	void postUpdate();
	void draw();

	GLFWwindow* window;
	Render* render;
	Shader* shader;
	int windowWidth, windowHeight;

	std::thread submitDraw;
	std::atomic<bool> finishedDrawSubmit;
};

#endif
