/*
  ==============================================================================

    TailThreadPoolJob.cpp
    Created: 23 Oct 2016 5:22:25pm
    Author:  Tony Ducks

  ==============================================================================
*/

#include "TailThreadPoolJob.h"

TailThreadPoolJob::TailThreadPoolJob(OwnedArray<OverlapAddConvolver>& t)
                            :ThreadPoolJob ("Demo Threadpool Job"),
                            tailConvolversArray(t)

{
}

ThreadPoolJob::JobStatus TailThreadPoolJob::runJob()
{
    const MessageManagerLock mml (this);

    if (mml.lockWasGained()){
        for (int i=0; i<tailConvolversArray.size(); i++){
            tailConvolversArray[i]->process();
        }
        //Logger::writeToLog("out " + String::toHexString ((int64) Thread::getCurrentThreadId()));
    }
    return jobHasFinished;
}