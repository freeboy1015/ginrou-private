#ifndef __FOURIER__
#define __FOURIER__

#include "include.h"


void fourier(Complex out[][FFT_SIZE],  double in[][FFT_SIZE] );
void inverseFourier( double out[][FFT_SIZE], Complex in[][FFT_SIZE]);
void fourierSpectrumImage(double out[][FFT_SIZE], Complex in[][FFT_SIZE]);

void fourier1D(Complex out[], double in[]);
void inverseFourier1D(double out[], Complex in[]);

#endif
