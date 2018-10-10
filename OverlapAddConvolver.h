/*
  ==============================================================================

    OverlapAddConvolver.h
    Created: 16 Oct 2016 11:49:03am
    Author:  Tony Ducks

  ==============================================================================
*/

#ifndef OVERLAPADDCONVOLVER_H_INCLUDED
#define OVERLAPADDCONVOLVER_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"
#include "fftw3.h"


class OverlapAddConvolver{
    
public:
    OverlapAddConvolver(int _size_partition,int _size_block);
    ~OverlapAddConvolver();
    void initFFTW();
    void init_h(const float* _h);
    void setInputPtr (fftw_complex* _X);
    void setOutPtr (int _ptrToRemainBuff);
    void init_remainBuffer(float* _remainBuffer, int _size_remainBuffer);
    void process();
    void complex_mult(fftw_complex *x, fftw_complex *h, fftw_complex *y);
    
private:
    fftw_complex*   X;
    fftw_complex    *h, *H;
    fftw_complex    *Y, *y;
    fftw_plan       plan_forward_h, plan_backward;
    int             size_block,size_partition, size_fft;
    float*          remainBuffer;
    int             size_remainBuffer;
    int             ptrToRemainBuff;
};

#endif  // OVERLAPADDCONVOLVER_H_INCLUDED
