// $B2hA|<h$j9~$_!?J]B8%W%m%0%i%`!#%F%9%HMQ(B

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fourier.h>
#include <ppm.h>

unsigned char image[FFT_SIZE][FFT_SIZE];
Complex fimage[FFT_SIZE][FFT_SIZE];

#define RAD 20

main (int argc, char *argv[]) {
  int width, height, x, y, off_x, off_y;
  unsigned char *data;
  double maxPower;
  char fname[1000];

  if(argc != 2) {
    fprintf(stderr, "filter : test fourier transform library.\n");
    fprintf(stderr, "Usage : fourier <file name prefix>\n"); 
    fprintf(stderr, "file : PGM monochrome\n"); 
    exit(0);
  }

  // $BFI$_=P$7(B ( $B2hAGCM7?(B unsigned char )
  data = readPGM(argv[1], &width, &height);
  printf("size = %d %d\n", width, height);

  fourier(fimage, (unsigned char (*)[FFT_SIZE])data);

  fourierSpectrumImage(image, fimage);

  strcpy(fname, argv[1]);
  strcat(fname, ".fft.pbm");
  writePGM(fname, (unsigned char *)image, FFT_SIZE, FFT_SIZE);

#if 1
  for( x = 0; x < FFT_SIZE; x++) {
    for( y = 0; y < FFT_SIZE; y++) {
      if((x > RAD && x < FFT_SIZE - RAD) || (y > RAD && y < FFT_SIZE - RAD)) {
	fimage[x][y].Re = fimage[x][y].Im = 0;
      }
    }
  }
#endif
  inverseFourier(image, fimage);

  strcpy(fname, argv[1]);
  strcat(fname, ".filt.pbm");
  writePGM(fname, (unsigned char *)image, FFT_SIZE, FFT_SIZE);

  free(data);
}

