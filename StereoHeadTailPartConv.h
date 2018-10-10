/*
  ==============================================================================

    StereoHeadTailPartConv.h
    Created: 2 Dec 2016 5:25:23pm
    Author:  Tony Ducks

  ==============================================================================
*/

#ifndef STEREOHEADTAILPARTCONV_H_INCLUDED
#define STEREOHEADTAILPARTCONV_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"
#include "fftw3.h"
#include "OverlapAddConvolver.h"
#include "TailThreadPoolJob.h"

#define ACCBUFFSIZE 4096

class StereoHeadTailPartConv
{
public:
    
    StereoHeadTailPartConv();
    StereoHeadTailPartConv(bool);
    StereoHeadTailPartConv(const String& _path);
    virtual ~StereoHeadTailPartConv();
    void definirFFT(int );
    void clearFFT();
    void init_h(const float* _h);
    int getBffSize();
    void setBffSize(int size);
    void readIR(const String& path);
    void loadNewIR(const String& path);
    void resetPartitions();
    void processBlock(float* writePointerL, float* writePointerR);

private:
    int                                 total_parts, tail_parts, head_parts;
    OwnedArray<OverlapAddConvolver>     headConvolversArrayL, headConvolversArrayR;
    OwnedArray<OverlapAddConvolver>     tailConvolversArray;
    ThreadPool                          pool;
    ScopedPointer<AudioSampleBuffer>    remainBuffer;
    int                                 remainBufferSize;
    int                                 ptr_global, accIndex;
    
    int                                 hsr;
    int                                 size_ir, conv_size_block, size_fft, size_partition;
    int                                 host_size_block;
    
    fftw_complex                        *xL, *XL, *xR, *XR;
    fftw_plan                           TDx2XL, TDx2XR;
    
    //AudioFormatManager                  audioFormatManager;
    //AudioFormatReader*                  audioFormatReader;
    ScopedPointer<AudioSampleBuffer>    irAudioBuffer, irZeroPadded;
    
    bool                                accumulateInputSamplesInBiggerBuffer = false;
    //ScopedPointer<AudioSampleBuffer>    accumulationBuffer;
    bool                                isDummy;
};


#endif  // STEREOHEADTAILPARTCONV_H_INCLUDED
