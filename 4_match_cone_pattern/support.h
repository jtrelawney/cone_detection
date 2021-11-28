#ifndef SUPPORT
#define SUPPORT

// global variables, necessary to make the image sliders work
//hsv parameters preset
int SatLow = 80, track_cone_low;
int SatHigh = 110, track_cone_high;
int Equalize = 0;
bool equalize_y_n;
int template_select = 0;
bool template_onoff;
int binarize_select = 0;
bool binarize_onoff;


// call back functions for the sliders, only necessary when they should ajdust / display something
static void tb_c_low( int, void* ){ track_cone_low = SatLow; }
static void tb_c_high( int, void* ){ track_cone_high = SatHigh; }
static void tb_c_equal( int, void* ){ equalize_y_n = (Equalize>0); }
static void button_template_onoff( int, void* ){ template_onoff = (template_select>0); }
static void button_binarize_onoff( int, void* ){ binarize_onoff = (binarize_select>0); }

#endif
