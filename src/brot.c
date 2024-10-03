#include <stddef.h>
#include <stdio.h>

#include "brot.h"

typedef struct complex {
  float re;
  float im;
} complex;

complex complex_add(complex a, complex b) {
  complex c = {a.re + b.re, a.im + b.im};
  return c;
}

complex complex_mul(complex a, complex b) {
  complex c = {
    (a.re * b.re) - (a.im * b.im),
    (a.re * b.im) + (a.im * b.re)
  };

  return c;
}

float map(
  float val, float from_min, float from_max, float to_min, float to_max
) {
  float from_range = from_max - from_min;
  float to_range = to_max - to_min;

  float scale = to_range / from_range;
  float offset = to_min - from_min;

  return (val * scale) + offset;
}

int calculate_mandelbrot_region(void* args) {
  Task* task = (Task*)args;

  size_t dst_x_offset = task->dst_viewport.min_x;
  size_t dst_y_offset = task->dst_viewport.min_y;
  size_t dst_col_count = task->dst_viewport.max_x - dst_x_offset;
  size_t dst_row_count = task->dst_viewport.max_y - dst_y_offset;

  for (size_t y = 0; y < dst_row_count; ++y) {
    if ((y + dst_y_offset) >= task->buffer_height) {
      break;
    }

    size_t dst_offset = (y + dst_y_offset) * task->buffer_width;

    float src_im = map(
      y + dst_y_offset,
      0, task->buffer_height,
      task->src_viewport.min_y, task->src_viewport.max_y
    );

    for (size_t x = 0; x < dst_col_count; ++x) {
      if ((x + dst_x_offset) >= task->buffer_width) {
        break;
      }

      size_t dst_index = (x + dst_x_offset + dst_offset) * 4;

      float src_re = map(
        x + dst_x_offset,
        0, task->buffer_width,
        task->src_viewport.min_x, task->src_viewport.max_x
      );

      task->dst_buf[dst_index    ] = src_re * 256;
      task->dst_buf[dst_index + 1] = src_im * 256;
      task->dst_buf[dst_index + 2] = 0x00;
      task->dst_buf[dst_index + 3] = 0xff;
    }
  }

  return 0;
}
