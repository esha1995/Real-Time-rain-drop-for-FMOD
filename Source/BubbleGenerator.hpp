#include <math.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "fmod.hpp"

class BubbleGenerator
{
public:
    BubbleGenerator(int sampleRate);
    ~BubbleGenerator();
    void generateSimulation(float r, float eps, bool barOn, int excitePoint, float L, float stiffness, float simTime, float damping);
    void resetBufferIndex();
    std::vector<float> processBuffer(int numSamples, float barAmp);
    std::vector<float> getBubble(){ return bubble; };
    std::vector<float> getMetalBar() { return metalBar; };
private:
    std::vector<float> metalBar;
    std::vector<float> bubble;
    std::vector<float> exciteMetalBar(const std::vector<float>& exciter, float L, float lengthSoundS, float stiffness, float excitePoint, float damping);
    std::vector<float> generateBubble(float r, float eps);
    int bufferIndex = 0;
    int sampleRate = 44100;
    float maxLengthS = 0.5f;
    float radius = 0.007f;
    
    // aluminium constants ->
    double density = 2.7000e-06;
    float E = 68;
};
