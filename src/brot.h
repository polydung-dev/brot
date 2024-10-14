#ifndef BROT_H_
#define BROT_H_

#include <threads.h>

typedef struct viewport {
  float min_x;
  float min_y;
  float max_x;
  float max_y;
} viewport;

typedef struct Task {
  unsigned char* dst_buf; //!< output buffer
  mtx_t* mutex;       //!< mutex for the buffer
  float buffer_width;     //!< width of output buffer (in pixels)
  float buffer_height;    //!< height of output buffer (in pixels)
  viewport src_viewport;  //!< (real numbers)
  viewport dst_viewport;  //!< (screen pixels}
} Task;

int calculate_mandelbrot_region(void* args);

#endif // BROT_H_
