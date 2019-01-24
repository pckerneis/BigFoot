/*
  ==============================================================================

    AudioDataCollector.h
    Created: 24 Jan 2019 12:07:21pm
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
template <typename SampleType>
class AudioBufferQueue
{
public:
    //==============================================================================
    static constexpr size_t order = 9;
    static constexpr size_t bufferSize = 1U << order;
    static constexpr size_t numBuffers = 5;

    //==============================================================================
    void push (const SampleType* dataToPush, size_t numSamples)
    {
        jassert (numSamples <= bufferSize);

        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (1, start1, size1, start2, size2);

        jassert (size1 <= 1);
        jassert (size2 == 0);

        if (size1 > 0)
            FloatVectorOperations::copy (buffers[(size_t) start1].data(), dataToPush, (int) jmin (bufferSize, numSamples));

        abstractFifo.finishedWrite (size1);
    }

    //==============================================================================
    void pop (SampleType* outputBuffer)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead (1, start1, size1, start2, size2);

        jassert (size1 <= 1);
        jassert (size2 == 0);

        if (size1 > 0)
            FloatVectorOperations::copy (outputBuffer, buffers[(size_t) start1].data(), (int) bufferSize);

        abstractFifo.finishedRead (size1);
    }

private:
    //==============================================================================
    AbstractFifo abstractFifo { numBuffers };
    std::array<std::array<SampleType, bufferSize>, numBuffers> buffers;
};

//==============================================================================
template <typename SampleType>
class AudioDataCollector
{
public:
    //==============================================================================
    AudioDataCollector (AudioBufferQueue<SampleType>& queueToUse)
        : audioBufferQueue (queueToUse)
    {}

    //==============================================================================
    void process (const SampleType* data, size_t numSamples)
    {
        size_t index = 0;

		while (index++ < numSamples)
                buffer[index] = *data++;

		audioBufferQueue.push(buffer.data(), buffer.size());
    }

private:
    //==============================================================================
    AudioBufferQueue<SampleType>& audioBufferQueue;
    std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
};