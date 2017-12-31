#ifndef GLUTIL_HPP
#define GLUTIL_HPP

#include <cstdlib>
#include <stdexcept>

#include <GL/glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */
#include <GLFW/glfw3.h>

namespace GLFW {
  static void error_callback(int e, const char *d) {
    printf("Error %d: %s\n", e, d);
  }

  void init_glfw3() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      throw std::runtime_error("glfw: unable to init");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif
  }

  GLFWwindow *new_window(const int w, const int h, const char *name) {
    GLFWwindow *win = glfwCreateWindow(w, h, name, nullptr, nullptr);
    if(win== nullptr) {
      throw std::runtime_error("new_window returns null");
    }
    glfwMakeContextCurrent(win);
    return win;
  }

  void get_win_size(GLFWwindow *win, int &w, int &h) {
    glfwGetWindowSize(win, &w, &h);
  }
} // namespace GLFW

namespace GLEW {
  void init_glew() {
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("unable to setup glfw");
    }
  }
} // namespace GLEW

#endif /* end of include guard: GLUTIL_HPP */
