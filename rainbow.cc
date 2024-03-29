#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;
using namespace std;

#define COLORS 2

int    NUM    = 1000;
double SPEED  = 0.4;
double OFFSET = 0.2;
double ACCEL  = 0.9995;
double TURNSPEED = 0.3;
double GSPEED = 0.0003;
int    SPOKES = 4;
int    PULSELEN  = 1;
double DECAY  = 0.95;
double BRIGHTNESS = 0.8;
//double BRIGHTNESS = 1;

#define WIDTH  3
#define HEIGHT 2

static int leds[WIDTH*32][HEIGHT*32][3];
static double gturn;
static double gshift;
static unsigned int pulse;

typedef struct {
  public:
    double x;
    double y;
    double xs;
    double ys;
    int r;
    int g;
    int b;
    bool held;
} star;

void initcolors();

const unsigned part_width = 5;
const unsigned part_height= 4;
unsigned part_x = 158;
unsigned part_y = 4;
const unsigned part_scale = 3;

/*
bool image_partial[part_height][part_width] = {
{1,1,1,1,1},
{1,1,1,0,1},
{0,0,0,1,0},
{0,0,0,0,0}};
*/
bool image_partial[part_height][part_width] = {
{0,0,0,0,0},
{0,1,0,0,0},
{1,0,1,1,1},
{1,1,1,1,1}};

static bool inImage(int x, int y){
  return image_partial[y/part_scale%part_height][x/part_scale%part_width];
}

static void DrawFull(Canvas *canvas){
  for(int x = 0; x < WIDTH*32; x++) {
    for(int y = 0; y < HEIGHT*32; y++) {
      if(x < part_x or x >= part_x + part_scale*part_width
      or y < part_y or y >= part_y + part_scale*part_height){
        canvas->SetPixel(x, y, leds[x][y][0]*BRIGHTNESS, leds[x][y][1]*BRIGHTNESS, leds[x][y][2]*BRIGHTNESS);
      } else {
        if(inImage(x-part_x, y-part_y)){
          canvas->SetPixel(x, y, 255*BRIGHTNESS, 255*BRIGHTNESS, 255*BRIGHTNESS);
        } else {
          canvas->SetPixel(x, y,   0,   0,   0);
        }
      }
      leds[x][y][0] *= DECAY;
      leds[x][y][1] *= DECAY;
      leds[x][y][2] *= DECAY;
    }
  }
}

static double hues[COLORS][3];

void DrawOnCanvas(Canvas *canvas) {
  star stars[NUM];

  srand(time(NULL));

  while(1){

    gturn = fmod(gturn + cos(47*gshift)*cos(17*gshift)*TURNSPEED, 2*M_PI);
    gshift = fmod(gshift + GSPEED, 2*M_PI);
    pulse = (pulse + 1) % PULSELEN;
    initcolors();
    for(int i = 0; i < NUM; i++) {
      star* s = &(stars[i]);
      if(!s->held) {
        s->x += s->xs;
        s->y += s->ys;
        s->xs *= ACCEL;
        s->ys *= ACCEL;
        int ix = (int)(s->x + WIDTH*16 - 1);
        int iy = (int)(s->y + HEIGHT*16 - 1);
        if(0<=ix &&ix< WIDTH*32 && 0 <= iy && iy < HEIGHT*32 && fabs(s->xs)+fabs(s->ys) >= 0.002) {
          leds[ix][iy][0] = min(255, leds[ix][iy][0] + s->r);
          leds[ix][iy][1] = min(255, leds[ix][iy][1] + s->g);
          leds[ix][iy][2] = min(255, leds[ix][iy][2] + s->b);
        } else {
          s->held = true;
        }
      } else {
        if(!pulse){
          double dir = (double)rand();
          int col = (int)(fmod(SPOKES * dir + gturn, 2*M_PI)/(2*M_PI) * (COLORS));
          double spawn_x = 48*sin(3*gshift)*sin(11*gshift);
          double spawn_y = 12*cos(7*gshift);
          part_x = max(0, min(WIDTH *32, (int)(WIDTH *32/2 + spawn_x - part_width *part_scale/2)));
          part_y = max(0, min(HEIGHT*32, (int)(HEIGHT*32/2 + spawn_y - part_height*part_scale/2)));
          stars[i] = {
            spawn_x,
            spawn_y,
            (sin(dir) + 0.05*(7*sin(14*gshift) - 4*sin(8*gshift))) * SPEED,
            (cos(dir) - 0.05*(7/4*sin(7*gshift))) * SPEED,
            (int)hues[col][0],
            (int)hues[col][1],
            (int)hues[col][2],
            false,
          };
        }
      }
    }

    DrawFull(canvas);

    usleep(5000);
  }
}

void handleKeyboard() {
  while(1){
    switch(getchar_unlocked()){
      case '-':
        SPOKES = max(1, SPOKES - 1);
        break;
      case '+':
        SPOKES = min(10, SPOKES + 1);
        break;
      default:
        break;
    }
  }
}

void initcolors(){
  for(int col = 0; col < COLORS; col++) {
    double ahue = 13*gshift + col*2*M_PI/COLORS;
    hues[col][0] =  max(0.0, 100.0 * cos(ahue));
    hues[col][1] =  max(0.0, 100.0 * cos(ahue + 2*M_PI/3));
    hues[col][2] =  max(0.0, 100.0 * cos(ahue + 4*M_PI/3));
  }
}

int main(int argc, char *argv[]) {

  /*
   * Set up the RGBMatrix. It implements a 'Canvas' interface.
   */
  int rows = 32;    // A 32x32 display. Use 16 when this is a 16x32 display.
  int chain = WIDTH;    // Number of boards chained together.
  int parallel = HEIGHT; // Number of chains in parallel (1..3). > 1 for plus or Pi2
  RGBMatrix::Options options;
  options.rows = 32;    // A 32x32 display. Use 16 when this is a 16x32 display.
  options.chain_length = WIDTH;  // Number of boards chained together.
  options.parallel = HEIGHT;      // Number of chains in parallel (1..3).
  Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &options);

  initcolors();
  thread drawer(DrawOnCanvas, canvas);    // Using the canvas.
  thread keyboardHandler(handleKeyboard);

  keyboardHandler.join();
  
  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
