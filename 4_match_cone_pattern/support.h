#ifndef SUPPORT
#define SUPPORT

// global variables, necessary to make the image sliders work
//hsv parameters preset
int SatLow = 80, track_cone_low;
int SatHigh = 110, track_cone_high;
int Equalize = 0;
bool equalize_y_n;

// call back functions for the sliders, only necessary when they should ajdust / display something
static void tb_c_low( int, void* ){ track_cone_low = SatLow; }
static void tb_c_high( int, void* ){ track_cone_high = SatHigh; }
static void tb_c_equal( int, void* ){ equalize_y_n = (Equalize>0); }

#endif
