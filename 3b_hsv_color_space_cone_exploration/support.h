#ifndef SUPPORT
#define SUPPORT

#include "opencv2/opencv.hpp"

// global variables, necessary to make the image sliders work
//hsv parameters preset
int hue_low=10, hue_high=40;
int sat_low=120, sat_high=255;
int val_low=90, val_high=255;

int color_space = 0;

// call back functions for the sliders, only necessary when they should ajdust / display something
static void tb_hue_low( int, void* ){}
static void tb_hue_high( int, void* ){}
static void tb_sat_low( int, void* ){}
static void tb_sat_high( int, void* ){}
static void tb_val_low( int, void* ){}
static void tb_val_high( int, void* ){}

static void tb_col_space_select( int, void* ){}

#endif
