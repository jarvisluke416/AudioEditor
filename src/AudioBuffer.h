#pragma once
#include <vector>
#include <cstddef>
#include <algorithm>

class EditorAudioBuffer
{
public:
    EditorAudioBuffer(
        std::size_t numSamples,
        int sampleRate = 48000
    )
        : sampleRate_(sampleRate),
          left_(numSamples, 0.0f),
          right_(numSamples, 0.0f)
    {
    }

    int sampleRate() const
    {
        return sampleRate_;
    }

    std::size_t size() const
    {
        return left_.size();
    }

    float* left()
    {
        return left_.data();
    }

    float* right()
    {
        return right_.data();
    }

    const float* left() const
    {
        return left_.data();
    }

    const float* right() const
    {
        return right_.data();
    }

    void clear()
    {
        std::fill(left_.begin(), left_.end(), 0.0f);
        std::fill(right_.begin(), right_.end(), 0.0f);
    }

    void add(
        std::size_t sample,
        float leftSample,
        float rightSample
    )
    {
        if (sample >= size())
            return;

        left_[sample] += leftSample;
        right_[sample] += rightSample;
    }

private:
    int sampleRate_;

    std::vector<float> left_;
    std::vector<float> right_;
};
