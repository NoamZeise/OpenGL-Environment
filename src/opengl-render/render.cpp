#include "render.h"

Render::Render(GLFWwindow *window, glm::vec2 target)
{
  this->window = window;
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::runtime_error("failed to load glad");

  basicShader = new Shader("shaders/b.vert", "shaders/b.frag");
  basicShader->Use();
  glUniform1i(basicShader->Location("image"), 0);

  Resize(target.x, target.y);

  view = glm::mat4(1.0f);
  //glUniform1i(basicShader.Location("image"), 0);

  std::vector<Vertex> quadVerts = {
          Vertex(0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
          Vertex(1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
          Vertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
          Vertex(0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
          Vertex(1.0f, 1.0f, 0.0f, 1.0f, 1.0f),
          Vertex(1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
        };
  std::vector<unsigned int> quadInds =  {0, 1, 2, 3, 4, 5};
  quad = new VertexData(quadVerts, quadInds);

  textureLoader = new Resource::TextureLoader();
  textureLoader->LoadTexture("textures/error.png");
}

Render::~Render()
{
  delete quad;
  delete basicShader;
  delete textureLoader;
}

void Render::Resize(int width, int height)
{
    proj = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
}

Resource::Texture Render::LoadTexture(std::string filepath)
{
  return textureLoader->LoadTexture(filepath);
}

Render::Draw::Draw(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset)
{
  this->tex = tex;
  this->model = model;
  this->colour = colour;
  this->texOffset = texOffset;
}


void Render::Begin2DDraw()
{
  currentDraw = 0;
}

void Render::EndDraw(std::atomic<bool>& submit)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  basicShader->Use();

  glUniformMatrix4fv(basicShader->Location("projection"), 1, GL_FALSE, &proj[0][0]);
  glUniformMatrix4fv(basicShader->Location("view"), 1, GL_FALSE, &view[0][0]);

  for(unsigned int i = 0; i < currentDraw; i++)
  {
    glUniformMatrix4fv(basicShader->Location("model"), 1, GL_FALSE, &drawCalls[i].model[0][0]);
    glUniform4fv(basicShader->Location("spriteColour"), 1, &drawCalls[i].colour[0]);
    glUniform1i(basicShader->Location("enableTex"), GL_TRUE); //disable textures for now
    //glUniform1i(basicShader.Location("enableFont"), GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    textureLoader->Bind(drawCalls[i].tex);
    quad->Draw(GL_TRIANGLES);
  }
  submit = true;
}

void Render::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset)
{
  if(currentDraw < MAX_DRAWS)
  {
    drawCalls[currentDraw] = Draw(texture, modelMatrix, colour, texOffset);

    currentDraw++;
  }
}
