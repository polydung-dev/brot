#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "debug/gl_debug.h"

#include "brot.h"
#include "error.h"
#include "pdm.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define THREAD_COUNT 4
thrd_t threads[THREAD_COUNT];
Task* tasks[THREAD_COUNT];

unsigned char* display_buffer;
mtx_t buffer_mutex;

int main() {
  mtx_init(&buffer_mutex, mtx_plain);

  atexit(glfwTerminate);
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(
    WINDOW_WIDTH, WINDOW_HEIGHT, "mandelbrot", NULL, NULL
  );

  if (window == NULL) {
    fprintf(stderr, "Failed to create window!\n");
    quick_exit(BROT_WINDOW_ERROR);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGL(glfwGetProcAddress)) {
    fprintf(stderr, "Failed to load OpenGL!\n");
    return BROT_OPENGL_ERROR;
  }

  enable_gl_debug_output();

  // vertex array init ////////////////////////////////////////////////////////
  float vertices[] = {
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    0.0, 1.0
  };
  unsigned char indices[] = {0, 1, 2, 0, 2, 3};

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0
  );

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW
  );

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // shader program ///////////////////////////////////////////////////////////
  const GLuint shader_program = glCreateProgram();

  {
    int success;
    char info_log[512];

    const GLuint vshader_id = glCreateShader(GL_VERTEX_SHADER);
    const char* vshader_string =
    "#version 330 core\n"
    "layout (location = 0) in vec2 a_position;\n"
    "out vec2 f_tex_coords;\n"
    "uniform mat4 mvp_matrix;\n"
    "void main () {\n"
    "  f_tex_coords = a_position;\n"
    "  gl_Position = mvp_matrix * vec4(a_position, 0.0, 1.0);\n"
    "}\n"
    ;
    glShaderSource(vshader_id, 1, &vshader_string, NULL);
    glCompileShader(vshader_id);

    glGetShaderiv(vshader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
      memset(info_log, 0, 512);
      glGetShaderInfoLog(vshader_id, 512, NULL, info_log);
      fprintf(stderr, "Vertex shader compilation failed\n %s\n", info_log);
      return BROT_SHADER_ERROR;
    }

    const GLuint fshader_id = glCreateShader(GL_FRAGMENT_SHADER);
    const char* f_shader_string =
    "#version 330 core\n"
    "in vec2 f_tex_coords;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D texture0;\n"
    "void main () {\n"
    "  FragColor = texture(texture0, f_tex_coords);\n"
    "}\n"
    ;
    glShaderSource(fshader_id, 1, &f_shader_string, NULL);
    glCompileShader(fshader_id);

    glGetShaderiv(fshader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
      memset(info_log, 0, 512);
      glGetShaderInfoLog(fshader_id, 512, NULL, info_log);
      fprintf(stderr, "Fragment shader compilation failed\n %s\n", info_log);
      return BROT_SHADER_ERROR;
    }

    glAttachShader(shader_program, vshader_id);
    glAttachShader(shader_program, fshader_id);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      memset(info_log, 0, 512);
      glGetProgramInfoLog(shader_program, 512, NULL, info_log);
      fprintf(stderr, "Program linking failed\n %s\n", info_log);
      return BROT_SHADER_ERROR;
    }

    glDetachShader(shader_program, fshader_id);
    glDetachShader(shader_program, vshader_id);
    glDeleteShader(fshader_id);
    glDeleteShader(vshader_id);
  }

  glUseProgram(shader_program);

  // set uniforms /////////////////////////////////////////////////////////////
  mat4 model;
  pdm_identity_m4(model);
  vec3 window_scale = {WINDOW_WIDTH, WINDOW_HEIGHT, 1.0};
  pdm_scale_m4(model, window_scale);

  mat4 view;
  pdm_identity_m4(view);

  mat4 projection;
  pdm_identity_m4(projection);
  pdm_ortho(projection, 0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

  mat4 mvp_matrix;
  pdm_identity_m4(mvp_matrix);
  pdm_mult_m4(projection, mvp_matrix);
  pdm_mult_m4(view, mvp_matrix);
  pdm_mult_m4(model, mvp_matrix);

  const GLint loc = glGetUniformLocation(shader_program, "mvp_matrix");
  glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)(mvp_matrix));

  // display buffer & texture init ////////////////////////////////////////////
  display_buffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4);
  memset(display_buffer, 0, WINDOW_WIDTH * WINDOW_HEIGHT * 4);

  GLuint display_texture;
  glGenTextures(1, &display_texture);
  glBindTexture(GL_TEXTURE_2D, display_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    WINDOW_WIDTH, WINDOW_HEIGHT,
    0, GL_RGBA, GL_UNSIGNED_BYTE, display_buffer
  );
  glGenerateMipmap(GL_TEXTURE_2D);

  // create tasks /////////////////////////////////////////////////////////////
  float aspect_ratio = (float)WINDOW_HEIGHT / (float)WINDOW_WIDTH;
  float min_x = -2.0;
  float max_x = min_x + 3.0;

  float h = ((max_x - min_x) * aspect_ratio);
  float min_y = -h / 2.0;

  float src_slice_h = h / THREAD_COUNT;
  float dst_slice_h = (float)WINDOW_HEIGHT / THREAD_COUNT;

  // todo: implement a queue and thread pool
  for (int i = 0; i < THREAD_COUNT; ++i) {
    viewport src_viewport = {
      min_x, min_y + (src_slice_h * i), max_x, min_y + (src_slice_h * (i + 1))
    };
    viewport dst_viewport = {
      0, dst_slice_h * i, WINDOW_WIDTH, dst_slice_h * (i + 1)
    };

    Task* task = malloc(sizeof(Task));
    task->dst_buf        = display_buffer;
    task->mutex          = &buffer_mutex;
    task->buffer_width   = WINDOW_WIDTH;
    task->buffer_height  = WINDOW_HEIGHT;
    task->src_viewport   = src_viewport;
    task->dst_viewport   = dst_viewport;

    tasks[i] = task;
  }

  // spawn threads ////////////////////////////////////////////////////////////
  for (int i = 0; i < THREAD_COUNT; ++i) {
    thrd_t thread;

    int trv = thrd_create(
      &thread, calculate_mandelbrot_region, (void*)(tasks[i])
    );
    if (trv != thrd_success) {
      fprintf(stderr, "Thread creation failed!\n");
      return 1;
    }
    threads[i] = thread;
  }

  // main loop ////////////////////////////////////////////////////////////////
  glClearColor(0.1, 0.1, 0.1, 1.0);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // update display texture /////////////////////////////////////////////////
    int lock = mtx_trylock(&buffer_mutex);
    if (lock == thrd_busy) {
      // skip frame if busy
      printf("main: mutex busy\n");
      continue;
    }

    if (lock == thrd_error) {
      printf("main: mutex error!\n");
      continue;
    }

    glTexSubImage2D(
      GL_TEXTURE_2D, 0,
      0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
      GL_RGBA, GL_UNSIGNED_BYTE, display_buffer
    );

    int unlock = mtx_unlock(&buffer_mutex);
    // ???
    if (unlock == thrd_error) {
      printf("main: error: cannot unlock\n");
      break;
    }

    // display image //////////////////////////////////////////////////////////
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_program);
    glBindTexture(GL_TEXTURE_2D, display_texture);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL);

    glfwSwapBuffers(window);
  }

  for (int i = 0; i < THREAD_COUNT; ++i) {
    int res;
    thrd_join(threads[i], &res);
    free(tasks[i]);
  }

  glDeleteTextures(1, &display_texture);

  glDeleteProgram(shader_program);

  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  glfwDestroyWindow(window);
  free(display_buffer);
  return 0;
}
