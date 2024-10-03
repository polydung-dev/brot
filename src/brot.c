#include <stddef.h>
#include <stdio.h>

#include "brot.h"

#define MAX_ITERATIONS 256

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
  float offset = val - from_min;

  return to_min + (scale * offset);
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
      dst_y_offset, task->dst_viewport.max_y,
      task->src_viewport.min_y, task->src_viewport.max_y
    );

    for (size_t x = 0; x < dst_col_count; ++x) {
      if ((x + dst_x_offset) >= task->buffer_width) {
        break;
      }

      size_t dst_index = (x + dst_x_offset + dst_offset) * 4;

      float src_re = map(
        x + dst_x_offset,
        dst_x_offset, task->dst_viewport.max_x,
        task->src_viewport.min_x, task->src_viewport.max_x
      );

      complex z = {0, 0};
      complex c = {src_re, src_im};

      // julia
      // complex z = {src_re, src_im};
      // complex c = {0, 0.8};

      int iter_count = 0;
      while(iter_count < MAX_ITERATIONS) {
        z = complex_add(complex_mul(z, z), c);
        ++iter_count;

        if ((z.re * z.re) + (z.im * z.im) >= 4.0) {
          break;
        }
      }

      unsigned char colour = 0x00;
      if (iter_count < MAX_ITERATIONS) {
        colour = iter_count;
      }

      int lock = mtx_lock(task->mutex);
      if (lock == thrd_error) {
        printf("thread: mutex error!\n");
        continue;
      }

      task->dst_buf[dst_index    ] = colour;
      task->dst_buf[dst_index + 1] = colour;
      task->dst_buf[dst_index + 2] = colour;
      task->dst_buf[dst_index + 3] = 0xff;

      int unlock = mtx_unlock(task->mutex);
      if (unlock == thrd_error) {
        printf("thread: error: cannot unlock\n");
        return 1;
      }
    }
  }

  return 0;
}
