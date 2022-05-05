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

  glDeleteProgram(vs);
  glDeleteProgram(fs);

  return program;
}

int main()
{
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr); // Windowed
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  glewInit();

  const float positions[6] = {
    0.0f, 0.5f,
    -0.5f, -0.5f,
    0.5f, -0.5f
  };

  unsigned int buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

  ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

  std::cout << source.VertexSource << std::endl;
  std::cout << source.FragmentSource << std::endl;


  unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
  glUseProgram(shader);

  while(!glfwWindowShouldClose(window) )
  {
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    
    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}
