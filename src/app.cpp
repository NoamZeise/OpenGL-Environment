#include "app.h"


App::App()
{
    windowWidth = INITIAL_WINDOW_WIDTH;
	windowHeight = INITIAL_WINDOW_HEIGHT;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		throw std::runtime_error("failed to initialise glfw!");

	Render::SetGLFWWindowHints();

	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL App", nullptr, nullptr);
	if(!window)
	{
		glfwTerminate();
		throw std::runtime_error("failed to create glfw window!");
	}


	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, glfwRawMouseMotionSupported());

	int width = windowWidth;
	int height = windowHeight;
	if (settings::USE_TARGET_RESOLUTION) {
		width = settings::TARGET_WIDTH;
		height = settings::TARGET_HEIGHT;
	}

	render = new Render(window, glm::vec2(windowWidth, windowHeight));

	if (settings::FIXED_RATIO)
		glfwSetWindowAspectRatio(window, width, height);

	loadAssets();

	cam3d = Camera::FirstPerson(glm::vec3(3.0f, 0.0f, 2.0f));
	audioManager.Play("audio/test.wav", true, 0.5);
	finishedDrawSubmit = true;
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
	testModel = render->LoadModel("models/testScene.fbx");
  testTex = render->LoadTexture("textures/error.png");
  testFont = render->LoadFont("textures/Roboto-Black.ttf");
	render->EndResourceLoad();
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
	if (submitDraw.joinable())
    submitDraw.join();
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	if(render != nullptr && windowWidth != 0 && windowHeight != 0)
      render->FramebufferResize();
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

	cam3d.update(input, previousInput, timer);

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
	timer.Update();
	render->set3DViewMatrixAndFov(cam3d.getViewMatrix(), cam3d.getZoom());
}


void App::draw()
{
#ifdef TIME_APP_DRAW_UPDATE
	auto start = std::chrono::high_resolution_clock::now();
#endif

render->DrawModel(
			testModel, glm::translate(glm::mat4(1.0f), glm::vec3(0*10, 0*10, 0)),
			glm::inverseTranspose(cam3d.getViewMatrix() * glm::mat4(1.0f)));

render->Begin2DDraw();

render->DrawString(testFont, "test", glm::vec2(400, 100), 100, -0.5,
										glm::vec4(1), 90.0f);

render->DrawQuad(testTex,
									glmhelper::getModelMatrix(glm::vec4(0, 0, 400, 400), 0, 0),
									glm::vec4(1, 0, 1, 0.3), glm::vec4(0, 0, 1, 1));

render->EndDraw(finishedDrawSubmit);

//submitDraw = std::thread(&Render::EndDraw, render, std::ref(finishedDrawSubmit));

#ifdef TIME_APP_DRAW_UPDATE
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout
	<< "draw: "
	<< std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
	<< " microseconds" << std::endl;
#endif
}

glm::vec2 App::correctedPos(glm::vec2 pos)
{
  if (settings::USE_TARGET_RESOLUTION)
    return glm::vec2(
        pos.x * ((float)settings::TARGET_WIDTH / (float)windowWidth),
        pos.y * ((float)settings::TARGET_HEIGHT / (float)windowHeight));

  return glm::vec2(pos.x, pos.y);
}

glm::vec2 App::correctedMouse()
{
  return correctedPos(glm::vec2(input.X, input.Y));
}

/*
*       GLFW CALLBACKS
*/

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
