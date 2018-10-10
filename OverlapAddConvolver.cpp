/*
  ==============================================================================

    OverlapAddConvolver.cpp
    Created: 16 Oct 2016 11:49:03am
    Author:  Tony Ducks

  ==============================================================================
*/

#include "OverlapAddConvolver.h"

//==============================================================================
OverlapAddConvolver::OverlapAddConvolver(int _size_partition, int _size_block){
    size_partition = _size_partition;
    size_block = _size_block;
    size_fft = (2*size_partition);
    initFFTW();
}

//==============================================================================
OverlapAddConvolver::~OverlapAddConvolver(){
    fftw_destroy_plan( plan_forward_h );
    fftw_destroy_plan( plan_backward );
    
    fftw_free( h );
    fftw_free( H );
    fftw_free( Y );
    fftw_free( y );
}


//==============================================================================
void OverlapAddConvolver::initFFTW(){
    h  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * size_fft );
    H  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * size_fft );
    Y   = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * size_fft );
    y = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * size_fft );
    
    memset (h, 0, sizeof( fftw_complex ) * size_fft);
    memset (Y, 0, sizeof( fftw_complex ) * size_fft);
    
    plan_forward_h  = fftw_plan_dft_1d( size_fft, h, H, FFTW_FORWARD, FFTW_ESTIMATE );
    plan_backward = fftw_plan_dft_1d( size_fft, Y, y, FFTW_BACKWARD, FFTW_ESTIMATE );
}

//==============================================================================
void OverlapAddConvolver::init_h(const float* _h){
    for (int i = 0; i<size_partition; i++) {
        h[i][0] = *_h++;
    }
    
    fftw_execute( plan_forward_h );
}

//==============================================================================
void OverlapAddConvolver::init_remainBuffer(float* _remainBuffer, int _size_remainBuffer){
    remainBuffer = _remainBuffer;
    size_remainBuffer = _size_remainBuffer;
}

//==============================================================================
void OverlapAddConvolver::setInputPtr(fftw_complex* _X){
    X = _X;
}

//==============================================================================
void OverlapAddConvolver::setOutPtr(int _ptrToRemainBuff){
    ptrToRemainBuff = _ptrToRemainBuff;
}

//==============================================================================
void OverlapAddConvolver::process(){    
    complex_mult(X, H, Y);
    fftw_execute( plan_backward );
    
    for (int i = 0; i<size_fft; i++) {                         //Se escribe "y" en el RmBuffer de acuerdo a writeOut_ptr
        remainBuffer[(ptrToRemainBuff+i)%size_remainBuffer] += (1.0f/size_fft) * y[i][0];
    }
}

//==============================================================================
void OverlapAddConvolver::complex_mult(fftw_complex *x, fftw_complex *h, fftw_complex *y){
    for (int i = 0 ; i < size_fft ; i++ ){
        y[i][0]=x[i][0]*h[i][0] - x[i][1]*h[i][1];
        y[i][1]=x[i][0]*h[i][1] + x[i][1]*h[i][0];
    }
}