// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>

#define width 3
#define height 2

#define brightness 2
#define factor (100/brightness)

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

static void DrawOnCanvas(Canvas *canvas) {
  /*
   * Let's create a simple animation. We use the canvas to draw
   * pixels. We wait between each step to have a slower animation.
   */
  //canvas->Fill(0, 0, 255);
  
  int a = 0;
  while(1){
    a++;
    for(int x = 0; x <width*32; x++) {
      for(int y = 0; y < height*32; y++) {
        int xx = 4*x, yy = 4*y + a%1024;
	int val = (xx ^ yy);
	char r = val * (0.5+0.5*sin(0.01*a)),
             g = val * (0.5+0.5*sin(0.01*a + 0.5)),
	     b = val * (0.5+0.5*sin(0.01*a + 1.4));
        canvas->SetPixel(x, y, r/2, g/2, b/2);
      }
    }
    usleep(11000);
  }
}

int main(int argc, char *argv[]) {
  /*
   * Set up the RGBMatrix. It implements a 'Canvas' interface.
   */
  RGBMatrix::Options options;
  options.rows = 32;    // A 32x32 display. Use 16 when this is a 16x32 display.
  options.chain_length = width;  // Number of boards chained together.
  options.parallel = height;      // Number of chains in parallel (1..3).
  Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &options);


  DrawOnCanvas(canvas);    // Using the canvas.

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
