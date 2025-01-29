#ifndef DEBUG_GL_DEBUG_H_
#define DEBUG_GL_DEBUG_H_

#include "glad/gl.h"
#include <GLFW/glfw3.h> // APIENTRY

int enable_gl_debug_output();
void APIENTRY gl_debug_message_callback(
  GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
  const GLchar* message, const void* userParam
);


#endif // DEBUG_GL_DEBUG_H_
