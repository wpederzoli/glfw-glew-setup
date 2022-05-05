#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>

struct ShaderProgramSource
{
  std::string VertexSource;
  std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filePath)
{
  std::ifstream stream(filePath);

  enum class ShaderType
  {
    NONE = -1, VERTEX = 0, FRAGMENT = 1
  };

  std::string line;
  std::stringstream ss[2];
  ShaderType type = ShaderType::NONE;

  while(getline(stream, line) )
  {
    if(line.find("#shader") != std::string::npos)
    {
      if(line.find("vertex") != std::string::npos)
        type = ShaderType::VERTEX;
      else if(line.find("fragment") != std::string::npos)
        type = ShaderType::FRAGMENT;
    }
    else
    {
      ss[(int)type] << line << '\n';
    }
  }

  return { ss[0].str(), ss[1].str() };
};

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
  unsigned int id = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE)
  {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char* message = (char*)alloca(length * sizeof(char) );
    glGetShaderInfoLog(id, length, &length, message);

    std::cout << "Failed to compile " << 
      (GL_FRAGMENT_SHADER == type ? "vertex " : "fragment ") << 
      "shader!\n" << message << std::endl;

    glDeleteProgram(id);

    return 0;
  }

  return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
  unsigned int program =  glCreateProgram();
  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

int main()
{
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "Shaders test", nullptr, nullptr); // Windowed
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  glewInit();

  std::cout << glGetString(GL_VERSION) << std::endl;

  float positions[] = {
    -0.5f, -0.5f,
    0.5f, -0.5f,
    0.5f, 0.5f,
    -0.5f, 0.5f,
  };

  unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
  };


  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

  unsigned int ibo;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);


  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

  ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

  unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
  glUseProgram(shader);

  int location = glGetUniformLocation(shader, "u_Color");
  float r = 0.0f;
  float increment = 0.05f;

  while(!glfwWindowShouldClose(window) )
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if(r > 1.0f)
      increment = -0.05f;
    if(r < 0.0f)
      increment = 0.05f;

    r += increment;

    glUniform4f(location, r, 0.3f, 0.8f, 1.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}
