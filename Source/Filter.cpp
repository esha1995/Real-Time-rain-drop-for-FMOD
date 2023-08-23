//
//  DelayUnit.cpp
//  DelayPlugin
//
//  Created by James Kelly on 14/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//
//

#include <math.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "BubbleGenerator.hpp"
#include "fmod.hpp"
#include <iostream>
#include <random>



extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

// ==================== //
// CALLBACK DEFINITIONS //
// ==================== //
FMOD_RESULT Create_Callback                     (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Release_Callback                    (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Reset_Callback                      (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Read_Callback                       (FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
FMOD_RESULT Process_Callback                    (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT SetPosition_Callback                (FMOD_DSP_STATE *dsp_state, unsigned int pos);
FMOD_RESULT ShouldIProcess_Callback             (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);

FMOD_RESULT SetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float value);
FMOD_RESULT SetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int value);
FMOD_RESULT SetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT SetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int length);
FMOD_RESULT GetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr);
FMOD_RESULT GetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int *value, char *valuestr);
FMOD_RESULT GetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr);
FMOD_RESULT GetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void **data, unsigned int *length, char *valuestr);

FMOD_RESULT SystemRegister_Callback             (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT SystemDeregister_Callback           (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT SystemMix_Callback                  (FMOD_DSP_STATE *dsp_state, int stage);

// ==================== //
//      PARAMETERS      //
// ==================== //

// set all parameters in ENUM + a NUM_PARAMS which will know how many parameters the program has
enum
{
    PARAM_RADIUS,
    PARAM_XI,
    PARAM_MAXLENGTH,
    PARAM_AMP,
    PARAM_EXCITEPOINT,
    PARAM_STIFFNESS,
    PARAM_LENGTHBAR,
    PARAM_SIMTIME,
    PARAM_DAMPING,
    PARAM_BARAMP,
    PARAM_BARON,
    NUM_PARAMS
};

// create parameters as FMOD_DSP_PARAMETER DESC
static FMOD_DSP_PARAMETER_DESC radius, xi, maxLength, amplitude, excitePoint, lengthBar, stiffness, simTime, damping, barAmp, barOn;

// create a list with NUM_PARAM elements to get parameters
FMOD_DSP_PARAMETER_DESC* PluginsParameters[NUM_PARAMS] =
{
    &radius,
    &xi,
    &maxLength,
    &amplitude,
    &excitePoint,
    &stiffness,
    &lengthBar,
    &simTime,
    &damping,
    &barAmp,
    &barOn
};


// ==================== //
//     SET CALLBACKS    //
// ==================== //

FMOD_DSP_DESCRIPTION PluginCallbacks =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Bubble Generator",     // name
    0x00010000,                 // plugin version
    1,                          // no. input buffers
    1,                          // no. output buffers
    Create_Callback,            // create
    Release_Callback,           // release
    Reset_Callback,             // reset
    Read_Callback,              // read
    Process_Callback,           // process
    SetPosition_Callback,       // setposition
    NUM_PARAMS,                          // no. parameter
    PluginsParameters,                          // pointer to parameter descriptions
    SetFloat_Callback,          // Set float
    SetInt_Callback,            // Set int
    SetBool_Callback,           // Set bool
    SetData_Callback,           // Set data
    GetFloat_Callback,          // Get float
    GetInt_Callback,            // Get int
    GetBool_Callback,           // Get bool
    GetData_Callback,           // Get data
    ShouldIProcess_Callback,    // Check states before processing
    0,                          // User data
    SystemRegister_Callback,    // System register
    SystemDeregister_Callback,  // System deregister
    SystemMix_Callback          // Mixer thread exucute / after execute
};
// excitePoint, lengthBar, stiffness, simTime, damping,barOn;

// assign the parameters with a name and value, to be controlled from FMOD
extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription ()
    {
        FMOD_DSP_INIT_PARAMDESC_FLOAT(radius, "Bubble Radius", "", "", 0.1f, 2.0f, 0.7f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(xi, "Xi", "", "", 0.0f, 0.1f, 0.03f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(maxLength, "Max Length", "", "", 0.1f, 2.0f, 1.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(amplitude, "Amplitude", "", "", 0.01f, 1.0f, 0.8f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(excitePoint, "Excite Point", "", "", 0.0f, 10.0f, 5.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(stiffness, "Stiffness", "", "", 0.1f, 10.0f, 1.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(lengthBar, "Length Bar", "", "", 0.1f, 10.0f, 2.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(simTime, "SimTime", "", "", 0.2f, 3.0f, 1.5f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(damping, "Damping", "", "", 1.0f, 20.0f, 10.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(barAmp, "BarAmp", "", "", 0.0f, 0.01f, 0.005f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(barOn, "Bar On", "", "", 0.0f, 1.0f, 1.0f);
        return &PluginCallbacks;
    }
}

// ==================== //
//     PLUGIN CLASS     //
// ==================== //

// in the plugin class you do all of your DSP stuff
class Plugin
{
public:
    Plugin();
    void Prepare (FMOD_DSP_STATE *dsp_state);
    void Release (FMOD_DSP_STATE *dsp_state);
    void Reset (FMOD_DSP_STATE *dsp_state);
    void Process (float* inbuffer, float* outbuffer, unsigned int length, int channels, FMOD_DSP_STATE *dsp_state);
    void setParameterFloat(int index, float value);
    void getParameterFloat(int index, float* value);
private:
    bool approximatelyEqual(float a, float b, float epsilon);
    float epsilon = 1e-5;
    int sampleRate = 44100;
    int numChannels = 0;
    int numSamples = 0;
    
    // bubble params ->
    float bubbleRadius = 0.007f;
    float maxLength = 1;
    float amplitude = 0.8f;
    float xi = 0.3f;
    
    // metal bar params
    int excitePoint = 5;
    float lengthBar = 2.0f;
    float simTime = 1.5f;
    float damping = 10;
    float stiffness = 1.0f;
    float barAmp = 0.5f;
    bool barOn = true;
    bool valueChanged = false;
    
    // create pointer to a bubble generator object
    BubbleGenerator* bubbleGenerator;
};

void Plugin::Prepare(FMOD_DSP_STATE *dsp_state)
{
    // get samplerate from FMOD and create bubblegenerator with sample rate at pointer position
    FMOD_DSP_GETSAMPLERATE(dsp_state, &sampleRate);
    bubbleGenerator = new BubbleGenerator(this->sampleRate);
    
    // generate the bubble
    bubbleGenerator->generateSimulation(this->bubbleRadius, this->xi, this->barOn, this->excitePoint, this->lengthBar, this->stiffness, this->simTime, this->damping);
}

void Plugin::Process(float *inbuffer, float *outbuffer, unsigned int numSamples, int numChannels, FMOD_DSP_STATE *dsp_state)
{
    if(numSamples != this->numSamples)
    {
        this->numSamples = numSamples;
        this->numChannels = numChannels;
    }
    
    // if a parameter has changed then update the bubble ->
    if(valueChanged)
    {
        bubbleGenerator->resetBufferIndex(); // make the bubble play from the beggining
        bubbleGenerator->generateSimulation(this->bubbleRadius, this->xi, this->barOn, this->excitePoint, this->lengthBar, this->stiffness, this->simTime, this->damping);
        valueChanged = false;
    }
    
    // get the current buffer vector from bubble
    std::vector<float> buffer = bubbleGenerator->processBuffer(numSamples, this->barAmp);
            
    for (unsigned int s = 0; s < numSamples; s++)
    {
        // Loop through all channels within the sample (audio is interleaved) i
        // increase pointer to index in buffer array each time to look at new channel
        for (unsigned int ch = 0; ch < numChannels; ch++, *outbuffer++, *inbuffer++)
        {
            if(ch==0)
            {
                *outbuffer = buffer[s];
            }
            else if(ch==1)
            {
                *outbuffer = buffer[s];
            }
            else
            {
            }
        }
    }
}

bool Plugin::approximatelyEqual(float a, float b, float epsilon) {
    return std::abs(a - b) <= epsilon;
}

void Plugin::Release(FMOD_DSP_STATE *dsp_state)
{
    // do release stuff
}


void Plugin::setParameterFloat(int index, float value) {
    if(index == 0) // Center frequency
    {
        if(!approximatelyEqual(value, this->bubbleRadius, epsilon))
        {
            valueChanged = true;
            this->bubbleRadius = (value*0.01f);
        }
    }
    if(index == 1) // Center frequency
    {
        if(!approximatelyEqual(value, this->xi, epsilon))
        {
            valueChanged = true;
            this->xi = value;
        }
    }
    if(index == 2) // Center frequency
    {
        if(!approximatelyEqual(value, this->maxLength, epsilon))
        {
            valueChanged = true;
            this->maxLength = value;
        }
    }
    if(index == 3) // Center frequency
    {
        if(!approximatelyEqual(value, this->amplitude, epsilon))
        {
            valueChanged = true;
            this->amplitude = value;
        }
    }
    
    if(index == 4) // Center frequency
    {
        if(!approximatelyEqual(value, static_cast<float>(excitePoint), epsilon))
        {
            valueChanged = true;
            this->excitePoint = static_cast<int>(std::floor(value));
        }
    }
    

    if(index == 5) // Center frequency
    {
        if(!approximatelyEqual(value, this->stiffness, epsilon))
        {
            valueChanged = true;
            this->stiffness = value;
        }
    }
    
    if(index == 6) // Center frequency
    {
        if(!approximatelyEqual(value, this->lengthBar, epsilon))
        {
            valueChanged = true;
            this->lengthBar = value;
        }
    }
    
    if(index == 7) // Center frequency
    {
        if(!approximatelyEqual(value, this->simTime, epsilon))
        {
            valueChanged = true;
            this->simTime = value;
        }
    }
    if(index == 8) // Center frequency
    {
        if(!approximatelyEqual(value, this->damping, epsilon))
        {
            valueChanged = true;
            this->damping = value;
        }
    }
    if(index == 9) // Center frequency
    {
        if(!approximatelyEqual(value, this->barAmp, epsilon))
        {
            this->barAmp = value;
        }
    }
    if(index == 10) // Center frequency
    {
        if(value > 0.5f && !barOn)
        {
            valueChanged = true;
            this->barOn = true;
        }
        if(value < 0.5f && barOn)
        {
            valueChanged = true;
            this->barOn = false;
        }
    }
}

void Plugin::getParameterFloat(int index, float *value)
{
}

void Plugin::Reset(FMOD_DSP_STATE *dsp_state) {
    bubbleGenerator->resetBufferIndex();
}

// set/get bool, int etc. can also be gennerated
    
// ======================= //
// CALLBACK IMPLEMENTATION //
// ======================= //

FMOD_RESULT Create_Callback                     (FMOD_DSP_STATE *dsp_state)
{
    // create our plugin class and attach to fmod
    Plugin* state = (Plugin* )FMOD_DSP_ALLOC(dsp_state, sizeof(Plugin));
    dsp_state->plugindata = state;
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    state->Prepare(dsp_state);
    return FMOD_OK;
}

FMOD_RESULT Release_Callback                    (FMOD_DSP_STATE *dsp_state)
{
    // release our plugin class
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->Release(dsp_state);
    FMOD_DSP_FREE(dsp_state, state);
    return FMOD_OK;
}

FMOD_RESULT Reset_Callback                      (FMOD_DSP_STATE *dsp_state)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->Reset(dsp_state);
    return FMOD_OK;
}

FMOD_RESULT Read_Callback                       (FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
    return FMOD_OK;
}

FMOD_RESULT Process_Callback                    (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    
    switch (op) {
        case FMOD_DSP_PROCESS_QUERY:
            if (outbufferarray && inbufferarray)
            {
            outbufferarray[0].bufferchannelmask[0] = inbufferarray[0].bufferchannelmask[0];
            outbufferarray[0].buffernumchannels[0] = inbufferarray[0].buffernumchannels[0];
            outbufferarray[0].speakermode       = inbufferarray[0].speakermode;
            }
            
            
            
            if (inputsidle)
            {
                return FMOD_ERR_DSP_DONTPROCESS;
            }

            break;
            
        case FMOD_DSP_PROCESS_PERFORM:
            
            if (inputsidle)
            {
                return FMOD_ERR_DSP_DONTPROCESS;
            }
            
            // actually process
            state->Process(inbufferarray[0].buffers[0], outbufferarray[0].buffers[0], length, outbufferarray[0].buffernumchannels[0], dsp_state);
            
            return FMOD_OK;
            break;
    }
    
    return FMOD_OK;
}

FMOD_RESULT SetPosition_Callback                (FMOD_DSP_STATE *dsp_state, unsigned int pos)
{
    return FMOD_OK;
}

FMOD_RESULT ShouldIProcess_Callback             (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode)
{
    if (inputsidle)
    {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    return FMOD_OK;
}

FMOD_RESULT SetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float value)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->setParameterFloat(index, value);
    return FMOD_OK;
}

FMOD_RESULT SetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int value)
{
    return FMOD_OK;
}

FMOD_RESULT SetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value)
{
    return FMOD_OK;
}

FMOD_RESULT SetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int length)
{
    return FMOD_OK;
}

FMOD_RESULT GetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->getParameterFloat(index, value);
    return FMOD_OK;
}

FMOD_RESULT GetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int *value, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void **data, unsigned int *length, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT SystemRegister_Callback             (FMOD_DSP_STATE *dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT SystemDeregister_Callback           (FMOD_DSP_STATE *dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT SystemMix_Callback                  (FMOD_DSP_STATE *dsp_state, int stage)
{
    return FMOD_OK;
}
