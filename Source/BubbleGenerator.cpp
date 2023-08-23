#include "BubbleGenerator.hpp"
#include <iostream>
#include <random>

BubbleGenerator::BubbleGenerator(int sampleRate){
    this->sampleRate = sampleRate;
}

BubbleGenerator::~BubbleGenerator() {}

std::vector<float> BubbleGenerator::generateBubble(float r, float eps) {
    float T = 1.0f / static_cast<float>(sampleRate);
    int numSamples = static_cast<int>(std::round(maxLengthS * static_cast<float>(sampleRate)));
    std::vector<float> x(numSamples, 0.0f);

    for (int i = 0; i < numSamples; ++i) {
        float t = i * T;
        float f0 = 3.0f / r;
        float d = 0.13f / r + 0.0072f * std::pow(r, -1.5f);
        float sigma = eps * d;
        float ft = f0 * (1.0f + sigma * t);
        x[i] = 0.8f * std::sin(2.0f * M_PI * ft * t) * std::exp(-d * t);
    }
    return x;
}

std::vector<float> BubbleGenerator::exciteMetalBar(const std::vector<float>& exciter, float L, float lengthSoundS, float stiffness, float excitePoint, float damping) {
    float k = 1 / static_cast<float>(this->sampleRate);
    float c = std::sqrt(this->E / this->density);
    float h = c * k;
    int N = static_cast<int>(std::floor(L / h));
    h = std::sqrt(2 * stiffness * k);
    int lengthSound = static_cast<int>(this->sampleRate * lengthSoundS);

    excitePoint = (excitePoint > 10 || excitePoint < 0) ? ((excitePoint > 10) ? 10 : 0) : excitePoint;
    excitePoint = std::floor((N / 10) * std::floor(excitePoint));

    std::vector<float> uNext(N + 1, 0.0);
    std::vector<float> u(N + 1, 0.0);
    std::vector<float> uPrev(N + 1, 0.0);
    std::vector<float> y(lengthSound, 0.0);

    for (int n = 0; n < lengthSound; ++n) {
        if (n < static_cast<int>(exciter.size())) {
            u[excitePoint] = u[excitePoint] + exciter[n];
        }

        for (int l = 3; l < N - 2; ++l) {
            uNext[l] = (2 * u[l] - uPrev[l] - ((stiffness * stiffness * k * k) / (h * h * h * h)) * (u[l + 2] - 4 * u[l + 1] + 6 * u[l] - 4 * u[l - 1] + u[l - 2]) + damping * k * uPrev[l]) / (1 + damping * k);
        }

        uPrev = u;
        u = uNext;
        y[n] = u[excitePoint];
    }

    return y;
}

std::vector<float> BubbleGenerator::processBuffer(int numSamples, float barAmp) {
    std::vector<float> buffer(numSamples, 0.0f);
    for (int i = 0; i < numSamples; ++i, ++bufferIndex) {
        if(bufferIndex < bubble.size()) {
            buffer[i] += bubble[bufferIndex];
        }
        if(bufferIndex < metalBar.size())
        {
            buffer[i] += (barAmp * metalBar[bufferIndex]);
        }
        if(bufferIndex >= bubble.size() && bufferIndex >= metalBar.size())
        {
            break;
        }
    }
    return buffer;
}

void BubbleGenerator::resetBufferIndex() { 
    this->bufferIndex = 0;
}

void BubbleGenerator::generateSimulation(float r, float eps, bool barOn,int excitePoint, float L, float stiffness, float simTime, float damping)
{
    bubble = generateBubble(r, eps);
    if(barOn) metalBar = exciteMetalBar(bubble, L, simTime, stiffness, excitePoint, damping);
    else metalBar = std::vector<float>(bubble.size(), 0.0f);
}




