/*
  ==============================================================================

    Head&TailPartConv.cpp
    Created: 17 Oct 2016 1:57:01pm
    Author:  Tony Ducks

  ==============================================================================
*/

#include "HeadTailPartConv.h"

//==============================================================================
HeadTailPartConv::HeadTailPartConv():pool(1){
    conv_size_block = ACCBUFFSIZE;
    host_size_block = ACCBUFFSIZE;
    size_partition = nextPowerOfTwo(conv_size_block);
    size_fft = (2*size_partition);
    ptr_global = 0;
    definirFFT(size_fft);
    readIR();
    irZeroPadded = new AudioSampleBuffer(1,1);
    remainBuffer = new AudioSampleBuffer(1,1);
    accumulationBuffer = new AudioSampleBuffer(1,ACCBUFFSIZE);
    resetPartitions();
}

//==============================================================================
HeadTailPartConv::~HeadTailPartConv(){
    clearFFT();
    pool.removeAllJobs (true, 2000);
    deleteAndZero(audioFormatReader);
    audioFormatManager.clearFormats();
}


//==============================================================================
void HeadTailPartConv::definirFFT(int _size_fft){
    x = ( fftw_complex*) fftw_malloc( sizeof(fftw_complex) * _size_fft);
    X = ( fftw_complex*) fftw_malloc( sizeof(fftw_complex) * _size_fft);
    memset (x, 0, sizeof( fftw_complex ) * _size_fft);
    TDx2X = fftw_plan_dft_1d(_size_fft, x, X, FFTW_FORWARD, FFTW_ESTIMATE);
}

//==============================================================================
void HeadTailPartConv::clearFFT(){
    fftw_destroy_plan(TDx2X);
    fftw_free(x);
    fftw_free(X);
}

//==============================================================================
void HeadTailPartConv::resetPartitions(){
    total_parts = ceil(size_ir/(1.0f*size_partition));
    head_parts = 10;
    tail_parts = total_parts - head_parts;
    
    irZeroPadded->setSize(1, total_parts*size_partition);
    irZeroPadded->clear();
    irZeroPadded->copyFrom(0, 0, irAudioBuffer->getReadPointer(0), size_ir);
    
    remainBufferSize = size_partition*(total_parts+1);
    remainBuffer->setSize(1, remainBufferSize);
    remainBuffer->clear();
    
    accumulationBuffer->clear();
    
    //DEFINICION DE OWNEDARRAY CON OVERLAPADDCONVOLVERS
    headConvolversArray.clear(true);
    for (int i=0; i<head_parts; i++) {
        OverlapAddConvolver* headConvolver = new OverlapAddConvolver(size_partition,conv_size_block);
        headConvolver->init_h(irZeroPadded->getReadPointer(0, i*size_partition));
        headConvolver->init_remainBuffer(remainBuffer->getWritePointer(0), remainBufferSize);
        headConvolversArray.add(headConvolver);
    }
    
    tailConvolversArray.clear(true);
    for (int i=head_parts; i<total_parts; i++) {
        OverlapAddConvolver* tailConvolver = new OverlapAddConvolver(size_partition,conv_size_block);
        tailConvolver->init_h(irZeroPadded->getReadPointer(0, i*size_partition));
        tailConvolver->init_remainBuffer(remainBuffer->getWritePointer(0), remainBufferSize);
        tailConvolversArray.add(tailConvolver);
    }
}

//==============================================================================
int HeadTailPartConv::getBffSize(){
    return host_size_block;
}

//==============================================================================
void HeadTailPartConv::setBffSize(int size){
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
void HeadTailPartConv::readIR(){
    audioFormatManager.registerBasicFormats();
    File sfile("/Users/TonyDucks/Documents/Xcode/PartitionedConvClass/Source/IRs/IR1@44100.wav");
    audioFormatReader = audioFormatManager.createReaderFor(sfile);
    hsr = audioFormatReader->sampleRate;
    size_ir = audioFormatReader->lengthInSamples;
    irAudioBuffer = new AudioSampleBuffer(1,size_ir);
    irAudioBuffer->clear();
    audioFormatReader->read(irAudioBuffer, 0, size_ir, 0, true, false);
}

//==============================================================================
void HeadTailPartConv::processBlock(float* writePointer){
    
    if (accumulateInputSamplesInBiggerBuffer) {
        for (int i=0; i<host_size_block; i++){
            x[i+accIndex][0]=*(writePointer+i);
        }
        accIndex+=host_size_block;
        if(ACCBUFFSIZE - accIndex < host_size_block){                          //verificar si el accBuffer se desbordaría el siguiente frame
            for (int i=accIndex; i<ACCBUFFSIZE; i++){                          //y en ese caso... se llena con ceros hasta completar ACCBUFFSIZE
                x[i][0]= 0.0f;
            }
            accIndex = ACCBUFFSIZE;
        }
    }else{
        for (int i=0; i<conv_size_block; i++){
            x[i][0]=*(writePointer+i);
        }
    }
    
    if (accIndex == ACCBUFFSIZE || !accumulateInputSamplesInBiggerBuffer) {
        fftw_execute(TDx2X);                                            //FFT X
        
        for (int j=0; j<head_parts; j++){
            int writeOut_ptr=ptr_global + (j*conv_size_block);       //El writeOut_ptr inicia donde esta el global y cada particion lo desplaza size_block
            headConvolversArray[j]->setInputPtr(X);
            headConvolversArray[j]->setOutPtr(writeOut_ptr);
            headConvolversArray[j]->process();                  //X=fft(inputBuffer) y writeOut_ptr=en que parte del remainBuffer...
        }                                                       //se escribe resultado de conv
        
        for (int i=0; i<tail_parts; i++){
            int writeOut_ptr=ptr_global + ((i+head_parts)*conv_size_block);
            tailConvolversArray[i]->setInputPtr(X);
            tailConvolversArray[i]->setOutPtr(writeOut_ptr);
        }
        
        TailThreadPoolJob* newTailJob = new TailThreadPoolJob(tailConvolversArray);
        pool.addJob (newTailJob, true);
        
        accIndex = 0;
    }
    
    if (accumulateInputSamplesInBiggerBuffer) {
        for (int i=0; i<host_size_block; i++){
            *(writePointer+i) = remainBuffer->getSample(0, (ptr_global+i)%remainBufferSize);
            remainBuffer->setSample(0, (ptr_global+i)%remainBufferSize, 0.0f);
        }
        ptr_global+= host_size_block;
    }else{
        for (int i=0; i<conv_size_block; i++){
            *(writePointer+i) = remainBuffer->getSample(0, (ptr_global+i)%remainBufferSize);         //Se lee el RmBuffer y se envia a la salida
            remainBuffer->setSample(0, (ptr_global+i)%remainBufferSize, 0.0f);                       //Se limpia esa parte del buffer
        }
        
        ptr_global+= conv_size_block;
    }
    
    
}
