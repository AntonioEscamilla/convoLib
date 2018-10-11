# convoLib
Partitioned And Threaded Audio Convolution Class

## Presentation
**convoLib** is a set of functionalities that compose an open source library that allows to perform the convolution between impulse responses of acoustic enclosures and audio signals in real time. The library is presented as a primary set of source code files (.h and .cpp) programmed in C ++ language with the aim of facilitating its use in multiplatform audio applications.

convoLib is developed under the context of JUCE, an open source framework for the development of applications for mobile and desktop platforms, known particularly for its libraries for audio management and user interfaces (GUI). JUCE focuses on providing application developers (who usually need several third-party software libraries), a complete software development framework that allows to speed up the development of audio computing applications.

## Dependencies
For the calculations of the Discrete Fourier Transform needed in the development of the convolution algorithm in the frequency domain, convoLib uses the "Fastest Fourier Transform in the West" library. FFTW is a software library with public GNU license to calculate discrete Fourier transforms, developed by Matteo Frigo and Steven G. Johnson at the Massachusetts Institute of Technology. For instructions on how to download, compile and use FFTW, the user should refer to the official website http://www.fftw.org/

To develop an audio application using convoLib, the user must be familiar with the use of the JUCE framework and the Projucer application to generate a project using external libraries such as FFTW and convoLib. JUCE is an open source software under public license GPL, for the development of applications for mobile platforms (iOS and Android) and desktop (OS X, Win and Linux). For instructions on how to download and use JUCE, the user should refer to the official page https://www.juce.com/

## Description
**convoLib** was designed as a set of classes with a certain dependency and hierarchy, to perform a single task: Convoluting an impulse response with an audio signal in real time. Next, the classes that make up the library are described:

### 1. HeadTailPartConv Class 

This is the class with the highest hierarchy and therefore contains the most general functionality in the task of convolving two audio signals. It is responsible for instantiating necessary objects of the other classes that make up the library and processing the audio signal in segments (frames) as required by the application (called client or Host in this context). The most relevant methods of this class are:

1. `void processBlock (float * wrtPointerL, float * wrtPointerR);`

   This method processes an audio segment coming from the Host and, therefore, must write in the host-buffer, the convolved    signal. This method also manages the use of two sample storage buffers: an accumulation buffer of audio samples for when    the size of the host-buffer is less than 4096 and a retentive buffer, where surplus samples of convolution calculations      are stored for previous audio frames.

2. `void resetPartitions ();`

   Method responsible for creating partitions for calculating the convolution, according to the size of the impulse response and the size of the host-buffer. It manages the creation of objects of the class OverlapAddConvolver, associating the partition of the response to the corresponding impulse and a pointer to the remaining buffer where they will write each convolution. In the creation of partitions, it is defined that the objects of the OverlapAddConvolver type, to which the first partitions of the impulse response are assigned, will process in the audio-thread the corresponding convolution (head convolvers) and that all other objects of the same type, process the convolution in a thread-pool to meet the real-time requirement (tail convolvers).

3. `void readIR (const String & path);`

   Read .wav files with the response to the impulse used for the convolution.

## About this Software
convoLib was developed by **Antonio Escamilla Pinilla** and **Daniel Upegui FLorez** working for the Universidad Pontificia Bolivariana, in the context of a research project entitled **Software Development for Measurement, Processing and Analysis of Acoustic Impulse Responses**. Project funded by the Research Center for Development and Innovation CIDI-UPB with number 771B-06/17-23.
