/*
  ==============================================================================

    TailThreadPoolJob.h
    Created: 23 Oct 2016 5:22:25pm
    Author:  Tony Ducks

  ==============================================================================
*/

#ifndef TAILTHREADPOOLJOB_H_INCLUDED
#define TAILTHREADPOOLJOB_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "OverlapAddConvolver.h"

//==============================================================================
class TailThreadPoolJob  : public ThreadPoolJob{
public:
    TailThreadPoolJob(OwnedArray<OverlapAddConvolver>& t);
    JobStatus runJob() override;

private:
    OwnedArray<OverlapAddConvolver>&    tailConvolversArray;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TailThreadPoolJob)
};


#endif  // TAILTHREADPOOLJOB_H_INCLUDED
