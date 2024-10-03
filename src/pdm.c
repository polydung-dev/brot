#include <string.h>

#include "pdm.h"

void swap(float* a, float* b) {
  float c = *a;
  *a = *b;
  *b = c;
}

float pdm_dot_v4(vec4 a, vec4 b) {
  return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}

void pdm_identity_m4(mat4 mat) {
  memset(mat, 0, sizeof(mat4));

  mat[0][0] = 1;
  mat[1][1] = 1;
  mat[2][2] = 1;
  mat[3][3] = 1;
}


void pdm_transpose_m4(mat4 mat) {
  swap(&mat[0][1], &mat[1][0]);
  swap(&mat[0][2], &mat[2][0]);
  swap(&mat[0][3], &mat[3][0]);
  swap(&mat[1][2], &mat[2][1]);
  swap(&mat[1][3], &mat[3][1]);
  swap(&mat[2][3], &mat[3][2]);
}

void pdm_mult_m4(mat4 other, mat4 mat) {
  // column major, so transpose and mulitply "backwards"
  mat4 tmp;
  pdm_transpose_m4(mat);

  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      float r = pdm_dot_v4(other[y], mat[x]);
      tmp[y][x] = r;
    }
  }

  memcpy(mat, tmp, sizeof(tmp));
}

void pdm_translate_m4(mat4 mat, vec3 transform) {
  mat4 tmp;
  pdm_identity_m4(tmp);

  tmp[3][0] = transform[0];
  tmp[3][1] = transform[1];
  tmp[3][2] = transform[2];
  tmp[3][3] = 1;

  pdm_mult_m4(tmp, mat);
}

void pdm_scale_m4(mat4 mat, vec3 transform) {
  mat4 tmp;
  pdm_identity_m4(tmp);

  tmp[0][0] = transform[0];
  tmp[1][1] = transform[1];
  tmp[2][2] = transform[2];
  tmp[3][3] = 1;

  pdm_mult_m4(tmp, mat);
}

void pdm_ortho(mat4 mat, float left, float right, float bottom, float top) {
  mat4 mat_t;
  pdm_identity_m4(mat_t);
  mat_t[2][2] = -1;
  vec3 t = {
    -(right + left) / 2.0f,
    -(top + bottom) / 2.0f,
    0.0f,
  };
  pdm_translate_m4(mat_t, t);

  mat4 mat_s;
  pdm_identity_m4(mat_s);
  vec3 s = {
    2.0f / (right - left),
    2.0f / (top - bottom),
    1.0f
  };
  pdm_scale_m4(mat_s, s);

  pdm_mult_m4(mat_t, mat_s);
  memcpy(mat, mat_s, sizeof(mat_s));
}
