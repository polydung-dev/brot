#ifndef PDM_H_
#define PDM_H_

/**
 * @file
 *
 * A minimal maths library for the sole porpose of creating a matrix for an
 * OpenGL shader program.
 */

typedef float vec3[3];
typedef float vec4[4];
typedef vec4 mat4[4];

void swap(float* a, float* b);

/**
 * @param [in]    a
 * @param [in]    b
 *
 * @returns dot product.
 */
float pdm_dot_v4(vec4 a, vec4 b);

/**
 * @param [inout] mat
 */
void pdm_identity_m4(mat4 mat);

/**
 * @param [inout] mat
 */
void pdm_transpose_m4(mat4 mat);

/**
 * @param [in]    other
 * @param [inout] mat
 */
void pdm_mult_m4(mat4 other, mat4 mat);

/**
 * @param [inout] mat
 * @param [in]    transform
 */
void pdm_translate_m4(mat4 mat, vec3 transform);

/**
 * @param [inout] mat
 * @param [in]    transform
 */
void pdm_scale_m4(mat4 mat, vec3 transform);

/**
 *
 * @param [inout] mat
 * @param         left
 * @param         right
 * @param         bottom
 * @param         top
 */
void pdm_ortho(
  mat4 mat,
  float left, float right, float bottom, float top
);

#endif // MAT_H_
