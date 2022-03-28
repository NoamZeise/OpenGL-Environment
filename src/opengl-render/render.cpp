#include "render.h"

Render::Render(GLFWwindow *window, glm::vec2 target)
{
  this->window = window;
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::runtime_error("failed to load glad");

  basicShader = new Shader("shaders/b.vert", "shaders/b.frag");

  Resize(target.x, target.y);

  view = glm::mat4(1.0f);
  //glUniform1i(basicShader.Location("image"), 0);

  float vertices[] = {
      0.5f,  0.5f, 0.0f,  // top right
      0.5f, -0.5f, 0.0f,  // bottom right
     -0.5f, -0.5f, 0.0f,  // bottom left
     -0.5f,  0.5f, 0.0f   // top left
 };
 unsigned int indices[] = {  // note that we start from 0!
     0, 1, 3,  // first Triangle
     1, 2, 3   // second Triangle
 };
 quad = new VertexData(&vertices[0], 12, &indices[0], 6);
}

Render::~Render()
{
  delete quad;
  delete basicShader;
}

void Render::Resize(int width, int height)
{
    proj = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
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
    //glUniform1i(basicShader.Location("enableTexture"), GL_FALSE); //disable textures for now
    //glUniform1i(basicShader.Location("enableFont"), GL_FALSE);
    //glActiveTexture(GL_TEXTURE0);
    //texture.Bind();
    quad->Draw(GL_TRIANGLES);
  }
  submit = true;
}

void Render::DrawQuad(const Resource::Texture& texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset)
{
  if(currentDraw < MAX_DRAWS)
  {
    drawCalls[currentDraw] = Draw(texture, modelMatrix, colour, texOffset);

    currentDraw++;
  }
}
