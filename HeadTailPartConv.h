/*
  ==============================================================================

    Head&TailPartConv.h
    Created: 17 Oct 2016 1:57:01pm
    Author:  Tony Ducks

  ==============================================================================
*/

#ifndef HEADTAILPARTCONV_H_INCLUDED
#define HEADTAILPARTCONV_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"
#include "fftw3.h"
#include "OverlapAddConvolver.h"
#include "TailThreadPoolJob.h"

#define ACCBUFFSIZE 4096

class HeadTailPartConv
{
public:
    
    HeadTailPartConv();
    virtual ~HeadTailPartConv();
    void definirFFT(int );
    void clearFFT();
    void init_h(const float* _h);
    int getBffSize();
    void setBffSize(int size);
    void readIR();
    void resetPartitions();
    void processBlock(float* writePointer);
    
private:
    int                                 total_parts, tail_parts, head_parts;
    OwnedArray<OverlapAddConvolver>     headConvolversArray;
    OwnedArray<OverlapAddConvolver>     tailConvolversArray;
    ThreadPool                          pool;
    ScopedPointer<AudioSampleBuffer>    remainBuffer;
    int                                 remainBufferSize;
    int                                 ptr_global, accIndex;
    
    int                                 hsr;
    int                                 size_ir, conv_size_block, size_fft, size_partition;
    int                                 host_size_block;
    
    fftw_complex                        *x, *X;
    fftw_plan                           TDx2X;
    
    AudioFormatManager                  audioFormatManager;
    AudioFormatReader*                  audioFormatReader;
    ScopedPointer<AudioSampleBuffer>    irAudioBuffer,irZeroPadded;
    
    bool                                accumulateInputSamplesInBiggerBuffer = false;
    ScopedPointer<AudioSampleBuffer>    accumulationBuffer;
};


#endif  // HEADTAILPARTCONV_H_INCLUDED
