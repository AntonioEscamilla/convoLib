# convoLib
Partitioned And Threaded Audio Convolution Class

## Presentation
The software is a set of functionalities that compose an open source library that allows to perform the convolution between impulse responses of acoustic enclosures and audio signals in real time. The library is presented as a primary set of source code files (.h and .cpp) programmed in C ++ language with the aim of facilitating its use in multiplatform audio applications.

convoLib is developed under the context of JUCE, an open source framework for the development of applications for mobile and desktop platforms, known particularly for its libraries for audio management and user interfaces (GUI). JUCE focuses on providing application developers (who usually need several third-party software libraries), a complete software development framework that allows to speed up the development of audio computing applications.

## Dependencies
For the calculations of the Discrete Fourier Transform needed in the development of the convolution algorithm in the frequency domain, convoLib uses the "Fastest Fourier Transform in the West" library. FFTW is a software library with public GNU license to calculate discrete Fourier transforms, developed by Matteo Frigo and Steven G. Johnson at the Massachusetts Institute of Technology. For instructions on how to download, compile and use FFTW, the user should refer to the official website http://www.fftw.org/

To develop an audio application using convoLib, the user must be familiar with the use of the JUCE framework and the Projucer application to generate a project using external libraries such as FFTW and convoLib. JUCE is an open source software under public license GPL, for the development of applications for mobile platforms (iOS and Android) and desktop (OS X, Win and Linux). For instructions on how to download and use JUCE, the user should refer to the official page https://www.juce.com/

## About this Software
convoLib was developed by **Antonio Escamilla Pinilla** and **Daniel Upegui FLorez** working for the Universidad Pontificia Bolivariana, in the context of a research project entitled **Software Development for Measurement, Processing and Analysis of Acoustic Impulse Responses**. Project funded by the Research Center for Development and Innovation CIDI-UPB with number 771B-06/17-23.
