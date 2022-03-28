#include "shader.h"

Shader::Shader(const char* VertexShaderPath, const char* FragmentShaderPath)
{
	std::ifstream in("shaders/b.vert");
	std::string vshaderSource((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();
	const char* vertexShaderSource = vshaderSource.c_str();

	std::ifstream fin("shaders/b.frag");
	std::string fshaderSource((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
	fin.close();
	const char* fragmentShaderSource = fshaderSource.c_str();

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
				glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// fragment shader
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		// check for shader compile errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
				glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// link shaders
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		// check for linking errors
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
				glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
	glDeleteProgram(shaderProgram);
}

unsigned int Shader::compileShader(const char* path, bool isFragmentShader)
{
	std::string dir = path;
	unsigned int shader;
	if (isFragmentShader)
		shader = glCreateShader(GL_FRAGMENT_SHADER);
	else
		shader = glCreateShader(GL_VERTEX_SHADER);

	//load shader source file into string
	std::ifstream in(path);
	std::string shaderSource((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();
	const char* source = shaderSource.c_str();
	glShaderSource(shader, 1, &source, NULL);

	glCompileShader(shader);

	int isCompiled;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (!isCompiled)
	{
		int logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		char* errorLog = new char[logSize];
		glGetShaderInfoLog(shader, logSize, &logSize, errorLog);

		std::cerr << "failed to compile shader " << path << "\n" << errorLog << std::endl;

		delete[] errorLog;
		errorLog = nullptr;
		glDeleteShader(shader);
	}

	return shader;
}

void Shader::Use()
{
	glUseProgram(shaderProgram);
}

unsigned int Shader::Location(const std::string& uniformName) const
{
	return glGetUniformLocation(shaderProgram, uniformName.c_str());
}
