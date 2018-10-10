/*
  ==============================================================================

    StereoStereoHeadTailPartConv.cpp
    Created: 2 Dec 2016 5:25:23pm
    Author:  Tony Ducks

  ==============================================================================
*/

#include "StereoHeadTailPartConv.h"

//==============================================================================
StereoHeadTailPartConv::StereoHeadTailPartConv():pool(1){
    conv_size_block = ACCBUFFSIZE;
    host_size_block = ACCBUFFSIZE;
    size_partition = nextPowerOfTwo(conv_size_block);
    size_fft = (2*size_partition);
    ptr_global = 0;
    definirFFT(size_fft);
    String path("/Users/TonyDucks/Documents/Xcode/Part&ThreadConvClass/Source/IRs/IR1_stereo@44100.wav");
    readIR(path);
    irZeroPadded = new AudioSampleBuffer(2,1);
    remainBuffer = new AudioSampleBuffer(2,1);
    //accumulationBuffer = new AudioSampleBuffer(2,ACCBUFFSIZE);
    resetPartitions();
}

//==============================================================================
StereoHeadTailPartConv::StereoHeadTailPartConv(bool _isDummy):pool(1){
    isDummy = _isDummy;
    conv_size_block = ACCBUFFSIZE;
    host_size_block = ACCBUFFSIZE;
    size_partition = nextPowerOfTwo(conv_size_block);
    size_ir = size_partition;
    size_fft = (2*size_partition);
    ptr_global = 0;
    definirFFT(size_fft);
    irAudioBuffer = new AudioSampleBuffer(2,size_ir);   //if dummy we use an impulse as IR of size = size_block
    irAudioBuffer->clear();
    irAudioBuffer->setSample(0, 0, 1.0f);
    irZeroPadded = new AudioSampleBuffer(2,1);
    remainBuffer = new AudioSampleBuffer(2,1);
    //accumulationBuffer = new AudioSampleBuffer(2,ACCBUFFSIZE);
    resetPartitions();
}

//==============================================================================
StereoHeadTailPartConv::StereoHeadTailPartConv(const String& _path):pool(1){
    conv_size_block = ACCBUFFSIZE;
    host_size_block = ACCBUFFSIZE;
    size_partition = nextPowerOfTwo(conv_size_block);
    size_fft = (2*size_partition);
    ptr_global = 0;
    definirFFT(size_fft);
    readIR(_path);
    irZeroPadded = new AudioSampleBuffer(2,1);
    remainBuffer = new AudioSampleBuffer(2,1);
    //accumulationBuffer = new AudioSampleBuffer(2,ACCBUFFSIZE);
    resetPartitions();
}

//==============================================================================
StereoHeadTailPartConv::~StereoHeadTailPartConv(){
    clearFFT();
    pool.removeAllJobs (true, 2000);
    //deleteAndZero(audioFormatReader);
    //audioFormatManager.clearFormats();
}


//==============================================================================
void StereoHeadTailPartConv::definirFFT(int _size_fft){
    xL = ( fftw_complex*) fftw_malloc( sizeof(fftw_complex) * _size_fft);
    XL = ( fftw_complex*) fftw_malloc( sizeof(fftw_complex) * _size_fft);
    memset (xL, 0, sizeof( fftw_complex ) * _size_fft);
    TDx2XL = fftw_plan_dft_1d(_size_fft, xL, XL, FFTW_FORWARD, FFTW_ESTIMATE);
    
    xR = ( fftw_complex*) fftw_malloc( sizeof(fftw_complex) * _size_fft);
    XR = ( fftw_complex*) fftw_malloc( sizeof(fftw_complex) * _size_fft);
    memset (xR, 0, sizeof( fftw_complex ) * _size_fft);
    TDx2XR = fftw_plan_dft_1d(_size_fft, xR, XR, FFTW_FORWARD, FFTW_ESTIMATE);
}

//==============================================================================
void StereoHeadTailPartConv::clearFFT(){
    fftw_destroy_plan(TDx2XL);
    fftw_free(xL);
    fftw_free(XL);
    fftw_destroy_plan(TDx2XR);
    fftw_free(xR);
    fftw_free(XR);
}

//==============================================================================
void StereoHeadTailPartConv::resetPartitions(){
    total_parts = ceil(size_ir/(1.0f*size_partition));
    head_parts = 1;
    tail_parts = total_parts - head_parts;
    
    irZeroPadded->setSize(2, total_parts*size_partition);
    irZeroPadded->clear();
    irZeroPadded->copyFrom(0, 0, irAudioBuffer->getReadPointer(0), size_ir);
    irZeroPadded->copyFrom(1, 0, irAudioBuffer->getReadPointer(1), size_ir);
    
    remainBufferSize = size_partition*(total_parts+1);
    remainBuffer->setSize(2, remainBufferSize);
    remainBuffer->clear();
    
    //accumulationBuffer->clear();
    
    //DEFINICION DE OWNEDARRAY CON OVERLAPADDCONVOLVERS
    headConvolversArrayL.clear(true);
    headConvolversArrayR.clear(true);
    for (int i=0; i<head_parts; i++) {
        OverlapAddConvolver* headConvolverL = new OverlapAddConvolver(size_partition,conv_size_block);
        headConvolverL->init_h(irZeroPadded->getReadPointer(0, i*size_partition));
        headConvolverL->init_remainBuffer(remainBuffer->getWritePointer(0), remainBufferSize);
        headConvolversArrayL.add(headConvolverL);
        
        OverlapAddConvolver* headConvolverR = new OverlapAddConvolver(size_partition,conv_size_block);
        headConvolverR->init_h(irZeroPadded->getReadPointer(1, i*size_partition));
        headConvolverR->init_remainBuffer(remainBuffer->getWritePointer(1), remainBufferSize);
        headConvolversArrayR.add(headConvolverR);
    }
    
    tailConvolversArray.clear(true);
    for (int i=head_parts; i<total_parts; i++) {
        OverlapAddConvolver* tailConvolverL = new OverlapAddConvolver(size_partition,conv_size_block);
        tailConvolverL->init_h(irZeroPadded->getReadPointer(0, i*size_partition));
        tailConvolverL->init_remainBuffer(remainBuffer->getWritePointer(0), remainBufferSize);
        tailConvolversArray.add(tailConvolverL);
        
        OverlapAddConvolver* tailConvolverR = new OverlapAddConvolver(size_partition,conv_size_block);
        tailConvolverR->init_h(irZeroPadded->getReadPointer(1, i*size_partition));
        tailConvolverR->init_remainBuffer(remainBuffer->getWritePointer(1), remainBufferSize);
        tailConvolversArray.add(tailConvolverR);
    }
}

//==============================================================================
int StereoHeadTailPartConv::getBffSize(){
    return host_size_block;
}

//==============================================================================
void StereoHeadTailPartConv::setBffSize(int size){
    pool.removeAllJobs (true, 2000);
    if (size < ACCBUFFSIZE){
        host_size_block = size;
        conv_size_block = ACCBUFFSIZE;
        accumulateInputSamplesInBiggerBuffer = true;
    }else{
        host_size_block = size;
        conv_size_block = size;
        accumulateInputSamplesInBiggerBuffer = false;
    }
    size_partition = nextPowerOfTwo(conv_size_block);
    size_fft = (2*size_partition);
    ptr_global = 0;
    accIndex = 0;
    definirFFT(size_fft);
    resetPartitions();
}

//==============================================================================
void StereoHeadTailPartConv::readIR(const String& path){
    AudioFormatManager audioFormatManager;
    audioFormatManager.registerBasicFormats();
    File sfile(path);
    AudioFormatReader* audioFormatReader;
    audioFormatReader = audioFormatManager.createReaderFor(sfile);
    hsr = audioFormatReader->sampleRate;
    size_ir = audioFormatReader->lengthInSamples;
    irAudioBuffer = new AudioSampleBuffer(2,size_ir);
    irAudioBuffer->clear();
    audioFormatReader->read(irAudioBuffer, 0, size_ir, 0, true, true);
    deleteAndZero(audioFormatReader);
    audioFormatManager.clearFormats();
}

//==============================================================================
void StereoHeadTailPartConv::loadNewIR(const String& _path){
    pool.removeAllJobs (true, 2000);
    ptr_global = 0;
    accIndex = 0;
    definirFFT(size_fft);
    readIR(_path);
    resetPartitions();
}

//==============================================================================
void StereoHeadTailPartConv::processBlock(float* writePointerL, float* writePointerR){
    
    if (accumulateInputSamplesInBiggerBuffer) {
        for (int i=0; i<host_size_block; i++){
            xL[i+accIndex][0]=*(writePointerL+i);
            xR[i+accIndex][0]=*(writePointerR+i);
        }
        accIndex+=host_size_block;
        if(ACCBUFFSIZE - accIndex < host_size_block){                          //verificar si el accBuffer se desbordaría el siguiente frame
            for (int i=accIndex; i<ACCBUFFSIZE; i++){                          //y en ese caso... se llena con ceros hasta completar ACCBUFFSIZE
                xL[i][0]= 0.0f;
                xR[i][0]= 0.0f;
            }
            accIndex = ACCBUFFSIZE;
        }
    }else{
        for (int i=0; i<conv_size_block; i++){
            xL[i][0]=*(writePointerL+i);
            xR[i][0]=*(writePointerR+i);
        }
    }
    
    if (accIndex == ACCBUFFSIZE || !accumulateInputSamplesInBiggerBuffer) {
        fftw_execute(TDx2XL);                                            //FFT X
        fftw_execute(TDx2XR);
        
        for (int j=0; j<head_parts; j++){
            int writeOut_ptr=ptr_global + (j*conv_size_block);       //El writeOut_ptr inicia donde esta el global y cada particion lo desplaza size_block
            headConvolversArrayL[j]->setInputPtr(XL);
            headConvolversArrayL[j]->setOutPtr(writeOut_ptr);
            headConvolversArrayL[j]->process();                     //X=fft(inputBuffer) y writeOut_ptr=en que parte del remainBuffer...
                                                                    //se escribe resultado de conv
            headConvolversArrayR[j]->setInputPtr(XR);
            headConvolversArrayR[j]->setOutPtr(writeOut_ptr);
            headConvolversArrayR[j]->process();
        }
        
        for (int i=0; i<tail_parts; i++){
            int writeOut_ptr=ptr_global + ((i+head_parts)*conv_size_block);
            tailConvolversArray[2*i]->setInputPtr(XL);
            tailConvolversArray[2*i]->setOutPtr(writeOut_ptr);
            
            tailConvolversArray[2*i+1]->setInputPtr(XR);
            tailConvolversArray[2*i+1]->setOutPtr(writeOut_ptr);
        }
        
        TailThreadPoolJob* newTailJob = new TailThreadPoolJob(tailConvolversArray);
        pool.addJob (newTailJob, true);
        
        accIndex = 0;
    }
    
    if (accumulateInputSamplesInBiggerBuffer) {
        for (int i=0; i<host_size_block; i++){
            *(writePointerL+i) = remainBuffer->getSample(0, (ptr_global+i)%remainBufferSize);
            remainBuffer->setSample(0, (ptr_global+i)%remainBufferSize, 0.0f);
            
            *(writePointerR+i) = remainBuffer->getSample(1, (ptr_global+i)%remainBufferSize);
            remainBuffer->setSample(1, (ptr_global+i)%remainBufferSize, 0.0f);
        }
        ptr_global+= host_size_block;
    }else{
        for (int i=0; i<conv_size_block; i++){
            *(writePointerL+i) = remainBuffer->getSample(0, (ptr_global+i)%remainBufferSize);         //Se lee el RmBuffer y se envia a la salida
            remainBuffer->setSample(0, (ptr_global+i)%remainBufferSize, 0.0f);                       //Se limpia esa parte del buffer
            
            *(writePointerR+i) = remainBuffer->getSample(1, (ptr_global+i)%remainBufferSize);         
            remainBuffer->setSample(1, (ptr_global+i)%remainBufferSize, 0.0f);
        }
        ptr_global+= conv_size_block;
    }
    
    
}