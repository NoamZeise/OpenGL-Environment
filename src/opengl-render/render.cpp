#include "render.h"

Render::Render(GLFWwindow *window, glm::vec2 target)
{
  glfwMakeContextCurrent(window);

  this->window = window;
  this->width = target.x;
  this->height = target.y;

  this->targetResolution = target;

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::runtime_error("failed to load glad");

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);


  blinnPhongShader = new Shader("shaders/3D-lighting.vert", "shaders/blinnphong.frag");
  blinnPhongShader->Use();
  glUniform1i(blinnPhongShader->Location("image"), 0);

  flatShader = new Shader("shaders/flat.vert", "shaders/flat.frag");
  flatShader->Use();
  glUniform1i(flatShader->Location("image"), 0);

  view2D = glm::mat4(1.0f);

  std::vector<Vertex2D> quadVerts = {
    Vertex2D(0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
    Vertex2D(1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
    Vertex2D(0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
    Vertex2D(0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
    Vertex2D(1.0f, 1.0f, 0.0f, 1.0f, 1.0f),
    Vertex2D(1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
  };
  std::vector<unsigned int> quadInds =  {0, 1, 2, 3, 4, 5};
  quad = new VertexData(quadVerts, quadInds);

  textureLoader = new Resource::TextureLoader();
  fontLoader = new  Resource::FontLoader();
  modelLoader = new Resource::ModelLoader();
  textureLoader->LoadTexture("textures/error.png");

  FramebufferResize();
}

Render::~Render()
{
  delete quad;
  delete blinnPhongShader;
  delete flatShader;
  delete textureLoader;
  delete fontLoader;
  delete modelLoader;
}

Resource::Texture Render::LoadTexture(std::string filepath)
{
  return textureLoader->LoadTexture(filepath);
}

Resource::Model Render::LoadModel(std::string filepath)
{
  return modelLoader->LoadModel(filepath, textureLoader);
}

Resource::Font Render::LoadFont(std::string filepath)
{
  return fontLoader->LoadFont(filepath, textureLoader);
}

void Render::set3DViewMatrixAndFov(glm::mat4 view, float fov)
{
  this->fov = fov;
  view3D = view;
  proj3D = glm::perspective(glm::radians(fov),
			((float)width) / ((float)height), 0.1f, 500.0f);
}

Render::Draw2D::Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset)
{
  this->tex = tex;
  this->model = model;
  this->colour = colour;
  this->texOffset = texOffset;
}

Render::Draw3D::Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix)
{
  this->model = model;
  this->modelMatrix = modelMatrix;
  this->normalMatrix = normalMatrix;
}


void Render::Begin2DDraw()
{
  current2DDraw = 0;
}

void Render::Begin3DDraw()
{
  current3DDraw = 0;
}

void Render::EndDraw(std::atomic<bool>& submit)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//Draw 2D

  flatShader->Use();

  glUniformMatrix4fv(flatShader->Location("projection"), 1, GL_FALSE, &proj2D[0][0]);
  glUniformMatrix4fv(flatShader->Location("view"), 1, GL_FALSE, &view2D[0][0]);
  for(unsigned int i = 0; i < current2DDraw; i++)
  {
    glUniformMatrix4fv(flatShader->Location("model"), 1, GL_FALSE, &draw2DCalls[i].model[0][0]);
    glUniform4fv(flatShader->Location("spriteColour"), 1, &draw2DCalls[i].colour[0]);
    glUniform4fv(flatShader->Location("texOffset"), 1, &draw2DCalls[i].texOffset[0]);
    glUniform1i(flatShader->Location("enableTex"), GL_TRUE);
    glActiveTexture(GL_TEXTURE0);
    textureLoader->Bind(draw2DCalls[i].tex);
    quad->Draw(GL_TRIANGLES);
  }

//Draw 3D

  blinnPhongShader->Use();
  glUniformMatrix4fv(blinnPhongShader->Location("projection"), 1, GL_FALSE, &proj3D[0][0]);
  glUniformMatrix4fv(blinnPhongShader->Location("view"), 1, GL_FALSE, &view3D[0][0]);
  LightingParameters lighting;
  lighting.direction = glm::transpose(glm::inverse(view3D)) * lighting.direction;
  glUniform4fv(blinnPhongShader->Location("lighting.ambient"), 1, &lighting.ambient[0]);
  glUniform4fv(blinnPhongShader->Location("lighting.diffuse"), 1, &lighting.diffuse[0]);
  glUniform4fv(blinnPhongShader->Location("lighting.specular"), 1, &lighting.specular[0]);
  glUniform4fv(blinnPhongShader->Location("lighting.direction"), 1, &lighting.direction[0]);
  glm::vec4 colourWhite = glm::vec4(1);
  for(unsigned int i = 0; i < current3DDraw; i++)
  {
    glUniformMatrix4fv(blinnPhongShader->Location("model"), 1, GL_FALSE, &draw3DCalls[i].modelMatrix[0][0]);
    glUniformMatrix4fv(blinnPhongShader->Location("normalMat"), 1, GL_FALSE, &draw3DCalls[i].normalMatrix[0][0]);
    glUniform4fv(blinnPhongShader->Location("spriteColour"), 1, &colourWhite[0]);
    glUniform1i(blinnPhongShader->Location("enableTex"), GL_TRUE);
    modelLoader->DrawModel(draw3DCalls[i].model, textureLoader);
  }

  glfwSwapBuffers(window);
  submit = true;
}

void Render::DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat)
{
  if(current3DDraw < MAX_3D_DRAWS)
  {
    draw3DCalls[current3DDraw++] = Draw3D(model, modelMatrix, normalMat);
  }
}

void Render::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset)
{
  if(current2DDraw < MAX_2D_DRAWS)
  {
    draw2DCalls[current2DDraw++] = Draw2D(texture, modelMatrix, colour, texOffset);
  }
}

void Render::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour)
{
  DrawQuad(texture, modelMatrix, colour, glm::vec4(0, 0, 1, 1));
}

void Render::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix)
{
  DrawQuad(texture, modelMatrix, glm::vec4(1), glm::vec4(0, 0, 1, 1));
}

void Render::DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate)
{
  auto draws = fontLoader->DrawString(font, text, position, size, depth, colour, rotate);

  for(const auto &draw: draws)
  {
    DrawQuad(draw.tex, draw.model, draw.colour);
  }
}

void Render::FramebufferResize()
{
  glfwGetWindowSize(window, &this->width, &this->height);
  glViewport(0, 0, width, height);

  float deviceRatio = (float)width /
                  (float)height;
  float virtualRatio = targetResolution.x / targetResolution.y;
  float xCorrection = width / targetResolution.x;
  float yCorrection = height / targetResolution.y;

  float correction;
  if (virtualRatio < deviceRatio) {
    correction = yCorrection;
  } else {
    correction = xCorrection;
  }
  proj2D = glm::ortho(
      0.0f, (float)width / correction, (float)height / correction, 0.0f, -10.0f, 10.0f);

  set3DViewMatrixAndFov(view3D, fov);
}
