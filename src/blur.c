/****************************************
 blur.c
 ****************************************/

#include <blur.h>

IMG* blur(IMG *img, IMG* psf)
{
  Complex psfIn[FFT_SIZE][FFT_SIZE] = { 0.0 };
  IMG* dst = createImage( img->height, img->width);

  for(int h = 0; h < psf->height; ++h){
    for( int w = 0 ; w < psf->width; ++w){
      psfIn[h][w].Re = (double)IMG_ELEM(psf, h, w);
    }
  }
  
  normalize( psfIn );
  
  for(int h = 0; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){
      
      double val = 0.0;

      for(int y = 0; y < psf->height; ++y){
	for(int x = 0; x < psf->width; ++x){
	  
	  if( h+y >= img->height || w+x >= img->width) continue;

	  val += (double)IMG_ELEM(img, h+y, w+x) * psfIn[y][x].Re;
	}
      }
      IMG_ELEM(dst, h, w) = (uchar)val;
    }

  }

  return dst;

}


IMG* blurFilter( IMG *img, IMG *psf)
{
  double imgIn[FFT_SIZE][FFT_SIZE] = {0.0};
  double psfIn[FFT_SIZE][FFT_SIZE] = {0.0};
  double dstIn[FFT_SIZE][FFT_SIZE];

  Complex imgFreq[FFT_SIZE][FFT_SIZE];
  Complex psfFreq[FFT_SIZE][FFT_SIZE];
  Complex dstFreq[FFT_SIZE][FFT_SIZE];

  //copy
  for(int h = 0 ; h < FFT_SIZE; ++h){
    for( int w = 0 ; w < FFT_SIZE; ++w){
      imgIn[h][w] = (double)IMG_ELEM(img , h, w);
    }
  }

  //copy psf
  for( int h = 0 ; h < psf->height; ++h){
    for( int w = 0; w < psf->width; ++w){
      int y = psf->height - h;
      int x = psf->width - w;
      psfIn[h][w] = (double)IMG_ELEM(psf, y, x);
    }
  }

  fourier( imgFreq, imgIn);
  fourier( psfFreq, psfIn);

  for(int h = 0; h < FFT_SIZE; ++h){
    for( int w = 0 ; w < FFT_SIZE; ++w){
      double a = imgFreq[h][w].Re;
      double b = imgFreq[h][w].Im;
      double c = psfFreq[h][w].Re;
      double d = psfFreq[h][w].Im;

      dstFreq[h][w].Re = a*c - b*d;
      dstFreq[h][w].Im = b*c + d*a;

    }
  }
  
  inverseFourier( dstIn, dstFreq);


  IMG* dst = createImage( FFT_SIZE, FFT_SIZE);
  for(int h = 0 ; h < FFT_SIZE; ++h){
    for( int w = 0; w < FFT_SIZE; ++w){
      IMG_ELEM(dst, h, w) = dstIn[h][w];
    }
  }

  return dst;
  

}


IMG* blurWithPSFMap( IMG* img, Mat psf[], IMG* psfMap)
{
  IMG* dst = createImage( img->height, img->width);
  
  for( int h = 0; h < dst->height; ++h){
    for( int w = 0 ; w < dst->width; ++w){

      double sum = 0.0;
      int idx = (int)IMG_ELEM( psfMap, h, w);



      for( int y = 0; y < psf[idx].row; ++y){
	for( int x = 0 ; x < psf[idx].clm; ++x){
	  int py = h + y - psf[idx].row / 2;
	  int px = w + x - psf[idx].clm / 2;

	  if( py < 0  || py >= img->height ||
	      px < 0  || px >= img->width){
	    continue;
	  }else{
	    sum += (double)IMG_ELEM( img, py, px) * ELEM0( psf[idx], y, x);
	  }
	}
      }

      if(idx == MAX_DISPARITY) sum = 255.0;
      IMG_ELEM( dst, h, w) = sum;

    }
  }

  return dst;

}


IMG* blurMat2IMG( IMG *src, Mat psf )
{
  Mat blurred = blurMat2Mat( src, psf);
  IMG* ret = createImage( blurred.row, blurred.clm);
  convertMat2IMG( &blurred, ret );
  matrixFree( blurred );
  return ret;
}


Mat blurMat2Mat( IMG *src, Mat psf )
{
  Mat dst = matrixAlloc( src->height, src->width );
  for( int h = 0 ; h < dst.row; ++h){
    for( int w = 0 ; w < dst.clm; ++w){
      ELEM0( dst, h, w ) = 0.0;
      for( int y = 0; y < psf.row ; ++y){
	for( int x = 0 ; x < psf.clm ; ++x){
	  int py = h + y - psf.row / 2;
	  int px = w + x - psf.clm /2 ;

	  if( py < 0 || py >= dst.row || px < 0 || px >= dst.clm) continue;
	  else
	    ELEM0( dst, h, w) += (double)IMG_ELEM( src, py, px ) * ELEM0( psf, y, x );
	}
      }
    }
  }

  return dst;

}

IMG* blurFreqWithMap( IMG* img, freq* psf[], IMG* psfMap)
{
  int height = img->height;
  int width = img->width;
  int h, w;
  size_t memSize = sizeof(freq) * height * width;
  freq* src = (freq*)fftw_malloc( memSize );
  fftw_plan planSrc = fftw_plan_dft_2d( height, width, src, src,
					FFTW_FORWARD, FFTW_ESTIMATE);

  // copy sourece and FFTW
  for( h = 0 ; h < height; ++h){
    for( w = 0 ; w < width ; ++w){
      src[ h*width + w ][0] = IMG_ELEM(img, h, w);
      src[ h*width + w ][1] = 0;
    }
  }
  fftw_execute( planSrc );
  fftw_destroy_plan (planSrc);

  // convolution in frequency region
  freq* blu[MAX_DISPARITY];
  for( int disp = 0; disp < MAX_DISPARITY; ++disp){
    blu[disp] = (freq*)fftw_malloc( memSize );
    for( int i = 0; i < height*width; ++i){
      double a = src[i][0];
      double b = src[i][1];
      double c = psf[disp][i][0];
      double d = psf[disp][i][1];
      blu[disp][i][0] = a*c - b*d;
      blu[disp][i][1] = a*d + b*c;
    }

    fftw_plan planBlu = fftw_plan_dft_2d( height, width, blu[disp], blu[disp],
					  FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute( planBlu );
    fftw_destroy_plan( planBlu );
  }

  double scale = height * width ;
  for(int d = 0; d < MAX_DISPARITY; ++d){
    for( int i = 0; i < height*width; ++i){
      blu[d][i][0] /= scale;
      blu[d][i][1] /= scale;
    }
  }

  // integrate to blurred image
  IMG* dst = createImage( height, width );
  for( h = 0 ; h < height; ++h){
    for( w = 0 ; w < width; ++w){
      int d = IMG_ELEM( psfMap, h, w);
      int idx = h * width + w;
      double val = sqrt(SQSUM( blu[d][idx][0], blu[d][idx][1] ));
      if(val >255) val = 255;
      IMG_ELEM( dst, h, w ) = val;
    }
  }

  // clean up
  for(int d = 0; d < MAX_DISPARITY; ++d)
    fftw_free( blu[d] );
  fftw_free( src );

return dst;
}


void normalize( Complex arr[FFT_SIZE][FFT_SIZE] )
{
  double norm = 0.0;

  //compute norm
  for( int h = 0; h < FFT_SIZE; ++h){
    for(int w = 0; w < FFT_SIZE; ++w){
      double re = arr[h][w].Re;
      double im = arr[h][w].Im;
      norm += sqrt( re*re + im*im );
    }
  }

  printf("norm = %lf\n", norm);

  // normalize 
  for( int h = 0 ; h < FFT_SIZE; ++h){
    for( int w = 0; w < FFT_SIZE; ++w){
      arr[h][w].Re /= norm;
      arr[h][w].Im /= norm;
    }
  }

  return;

}
