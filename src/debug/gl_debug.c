#include <stdio.h>

#include "glad/gl.h"
#include <GLFW/glfw3.h> // APIENTRY

#include "gl_debug.h"

int enable_gl_debug_output() {
  int flags = 0;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    // ensure callback is called in the same thread and scope
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_message_callback, NULL);
    // disable message filtering
    glDebugMessageControl(
      GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE
    );

    return 1;
  }

  return 0;
}

void APIENTRY gl_debug_message_callback(
  GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
  const GLchar *message, const void *userParam
) {
  printf("---\n");
  printf("DEBUG MESSAGE ( %i ) : %s\n", id, message);

  printf("Source  : ");
  switch (source) {
    case GL_DEBUG_SOURCE_API:
     printf("api");
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
     printf("window system");
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
     printf("shader compiler");
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
     printf("third party");
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
     printf("application");
      break;
    case GL_DEBUG_SOURCE_OTHER:
     printf("other");
      break;
    default:
     printf("unknown");
      break;
  }

 printf("\n");

 printf("Type    : ");
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
     printf("error");
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
     printf("deprecated behaviour");
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
     printf("undefined behaviour");
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
     printf("portability");
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
     printf("performance");
      break;
    case GL_DEBUG_TYPE_MARKER:
     printf("marker");
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
     printf("push group");
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
     printf("pop group");
      break;
    case GL_DEBUG_TYPE_OTHER:
     printf("other");
      break;
    default:
     printf("unknown");
      break;
  }

 printf("\n");

 printf("Severity : ");
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
     printf("high");
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
     printf("medium");
      break;
    case GL_DEBUG_SEVERITY_LOW:
     printf("low");
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
     printf("notification");
      break;
    default:
     printf("unknown");
      break;
  }

 printf("\n");
}
