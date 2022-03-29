#include "app.h"


App::App()
{
	//set member variables
	windowWidth = 800;
	windowHeight = 450;
	//init glfw window
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
glfwSetErrorCallback(error_callback);
if (!glfwInit())
			throw std::runtime_error("failed to initialise glfw!");

	window = glfwCreateWindow(windowWidth, windowHeight, "openGL", nullptr, nullptr);
	if(!window)
	{
		glfwTerminate();
		throw std::runtime_error("failed to create glfw window!");
	}
  glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	if (glfwRawMouseMotionSupported())
    	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

  render = new Render(window, glm::vec2(windowWidth, windowHeight));

  loadAssets();
}

App::~App()
{
	if(submitDraw.joinable())
		submitDraw.join();
	delete render;
	glfwTerminate();
}

void App::loadAssets()
{
	testTex = render->LoadTexture("textures/error copy.png");
}

void App::run()
{
  while (!glfwWindowShouldClose(window))
  {
    update();
    if(windowWidth != 0 && windowHeight != 0)
      draw();
  }
}

void App::resize(int windowWidth, int windowHeight)
{
	glViewport(0, 0, windowWidth, windowHeight);
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
}

void App::update()
{
#ifdef TIME_APP_DRAW_UPDATE
	auto start = std::chrono::high_resolution_clock::now();
#endif


	glfwPollEvents();

	if (input.Keys[GLFW_KEY_F] && !previousInput.Keys[GLFW_KEY_F])
	{
		if (glfwGetWindowMonitor(window) == nullptr)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, NULL, 100, 100, windowWidth, windowHeight, mode->refreshRate);
		}
	}
	if(input.Keys[GLFW_KEY_ESCAPE] && !previousInput.Keys[GLFW_KEY_ESCAPE])
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	postUpdate();
#ifdef TIME_APP_DRAW_UPDATE
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout
		 << "update: "
         << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
		 << " microseconds" << std::endl;
#endif
}

void App::postUpdate()
{
	previousInput = input;
	input.offset = 0;
}


void App::draw()
{
#ifdef TIME_APP_DRAW_UPDATE
	auto start = std::chrono::high_resolution_clock::now();
#endif

render->Begin2DDraw();

render->DrawQuad(testTex, glmhelper::getModelMatrix(glm::vec4(400, 200, 100, 50), 0),
	glm::vec4(1, 1, 1, 1), glm::vec4(0 , 0, 1, 1));

render->DrawQuad(Resource::Texture(), glmhelper::getModelMatrix(glm::vec4(100, 200, 100, 200), 0),
	glm::vec4(1, 1, 1, 1), glm::vec4(0, 0, 1, 1));

render->EndDraw(finishedDrawSubmit);
//submitDraw = std::thread(&Render::EndDraw, render, std::ref(finishedDrawSubmit));

glfwSwapBuffers(window);

#ifdef TIME_APP_DRAW_UPDATE
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout
	<< "draw: "
	<< std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
	<< " microseconds" << std::endl;
#endif
}

#pragma region GLFW_CALLBACKS


void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->resize(width, height);
}

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.X = xpos;
	app->input.Y = ypos;
}
void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.offset = yoffset;
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Keys[key] = false;
		}
	}
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

	if (button >= 0 && button < 8)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Buttons[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Buttons[button] = false;
		}
	}
}

void App::error_callback(int error, const char* description)
{
    throw std::runtime_error(description);
}

#pragma endregion
