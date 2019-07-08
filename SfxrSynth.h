
/**
 * SfxrSynth
 *
 * Copyright 2010 Thomas Vian
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Thomas Vian
 */
#pragma once

#include "SfxrParams.h"

class SfxrSynth
{
public:
	SfxrSynth (float sr)
		: sampleRate (sr)
	{}

	void setSampleRate (float sr) { sampleRate = sr; }
    
    //--------------------------------------------------------------------------
    //
    //  Getters / Setters
    //
    //--------------------------------------------------------------------------
    
    /** The sound parameters */
    const SfxrParams& getParams()
    {
        return _params;
    }
    
    void setParams (SfxrParams value)
    {
        _params = value;
        _params.paramsDirty = true;
    }
    
    //--------------------------------------------------------------------------
    //
    //  Synth Methods
    //
    //--------------------------------------------------------------------------
    
    /* Length in quarter-seconds */
    float getLength()
    {
        clampTotalLength();
        
        SfxrParams& p = _params;
        float envelopeLength0 = p.getParam ("attackTime") * p.getParam ("attackTime") * 100000.0f;
        float envelopeLength1 = p.getParam ("sustainTime") * p.getParam ("sustainTime") * 100000.0f;
        float envelopeLength2 = p.getParam ("decayTime") * p.getParam ("decayTime") * 100000.0f + 10;
        return (envelopeLength0 + envelopeLength1 + envelopeLength2) * 2 / (sampleRate);

    }
    
    void clampTotalLength()
    {
        SfxrParams& p = _params;
        float totalTime = p.getParam ("attackTime") + p.getParam ("sustainTime") + p.getParam ("decayTime");
        if (totalTime < MIN_LENGTH)
        {
            float multiplier = MIN_LENGTH / totalTime;
            p.setParam ("attackTime", p.getParam ("attackTime") * multiplier);
            p.setParam ("sustainTime", p.getParam ("sustainTime") * multiplier);
            p.setParam ("decayTime", p.getParam ("decayTime") * multiplier);
        }
    }
    
    /**
     * Resets the runing variables from the params
     * Used once at the start (total reset) and for the repeat effect (partial reset)
     * @param	totalReset	If the reset is total
     */
    void reset (bool totalReset)
    {
        // Shorter reference
        SfxrParams& p = _params;
        
        _period = 100.0f / (p.getParam ("startFrequency") * p.getParam ("startFrequency") + 0.001f);
        _maxPeriod = 100.0f / (p.getParam ("minFrequency") * p.getParam ("minFrequency") + 0.001f);

        _slide = 1.0f - p.getParam ("slide") * p.getParam ("slide") * p.getParam ("slide") * 0.01f;
        _deltaSlide = -p.getParam ("deltaSlide") * p.getParam ("deltaSlide") * p.getParam ("deltaSlide") * 0.000001f;
        
        if (int (p.getParam ("waveType")) == 0)
        {
            _squareDuty = 0.5f - p.getParam ("squareDuty") * 0.5f;
            _dutySweep = -p.getParam ("dutySweep") * 0.00005f;
        }
        
        _changePeriod = (((1-p.getParam ("changeRepeat")) + 0.1f) / 1.1f) * 20000 + 32;
        _changePeriodTime = 0;
        
        if (p.getParam ("changeAmount") > 0.0f)
            _changeAmount = 1.0f - p.getParam ("changeAmount") * p.getParam ("changeAmount") * 0.9f;
        else
            _changeAmount = 1.0f + p.getParam ("changeAmount") * p.getParam ("changeAmount") * 10.0f;
        
        _changeTime = 0;
        _changeReached=false;
        
        if (p.getParam ("changeSpeed") == 1.0f)
            _changeLimit = 0;
        else
            _changeLimit = int ((1.0f - p.getParam ("changeSpeed")) * (1.0f - p.getParam ("changeSpeed")) * 20000 + 32);
        
        
        if (p.getParam ("changeAmount2") > 0.0)
            _changeAmount2 = 1.0f - p.getParam ("changeAmount2") * p.getParam ("changeAmount2") * 0.9f;
        else
            _changeAmount2 = 1.0f + p.getParam ("changeAmount2") * p.getParam ("changeAmount2") * 10.0f;
        
        _changeTime2 = 0;
        _changeReached2 = false;
        
        if (p.getParam ("changeSpeed2") == 1.0f)
            _changeLimit2 = 0;
        else
            _changeLimit2 = int ((1.0f - p.getParam ("changeSpeed2")) * (1.0f - p.getParam ("changeSpeed2")) * 20000 + 32);
        
        _changeLimit  = int (_changeLimit * ((1.0f - p.getParam ("changeRepeat") + 0.1f) / 1.1f));
        _changeLimit2 = int (_changeLimit2 * ((1.0f - p.getParam ("changeRepeat") + 0.1f) / 1.1f));
        
        if (totalReset)
        {
            p.paramsDirty = false;
            
            _masterVolume = p.getParam ("masterVolume") * p.getParam ("masterVolume");
            
            _waveType = int (p.getParam ("waveType"));
            
            if (p.getParam ("sustainTime") < 0.01f)
                p.setParam ("sustainTime", 0.01f);
            
            clampTotalLength();
            
            _sustainPunch = p.getParam ("sustainPunch");
            
            _phase = 0;
            
            _minFreqency = p.getParam ("minFrequency");
            _muted = false;
            _overtones = int (p.getParam ("overtones") * 10);
            _overtoneFalloff = p.getParam ("overtoneFalloff");
                
            _bitcrush_freq = 1 - std::pow (p.getParam ("bitCrush"), 1.0f / 3.0f);
            _bitcrush_freq_sweep = - p.getParam ("bitCrushSweep") * 0.000015f;
            _bitcrush_phase = 0;
            _bitcrush_last = 0;
            
            _compression_factor = 1 / (1 + 4 * p.getParam ("compressionAmount"));
            
            _filters = p.getParam ("lpFilterCutoff") != 1.0f || p.getParam ("hpFilterCutoff") != 0.0;
            
            _lpFilterPos = 0.0f;
            _lpFilterDeltaPos = 0.0f;
            _lpFilterCutoff = p.getParam ("lpFilterCutoff") * p.getParam ("lpFilterCutoff") * p.getParam ("lpFilterCutoff") * 0.1f;
            _lpFilterDeltaCutoff = 1.0f + p.getParam ("lpFilterCutoffSweep") * 0.0001f;
            _lpFilterDamping = 5.0f / (1.0f + p.getParam ("lpFilterResonance") * p.getParam ("lpFilterResonance") * 20.0f) * (0.01f + _lpFilterCutoff);
            if (_lpFilterDamping > 0.8f) 
				_lpFilterDamping = 0.8f;
            _lpFilterDamping = 1.0f - _lpFilterDamping;
            _lpFilterOn = p.getParam ("lpFilterCutoff") != 1.0;
            
            _hpFilterPos = 0.0f;
            _hpFilterCutoff = p.getParam ("hpFilterCutoff") * p.getParam ("hpFilterCutoff") * 0.1f;
            _hpFilterDeltaCutoff = 1.0f + p.getParam ("hpFilterCutoffSweep") * 0.0003f;
            
            _vibratoPhase = 0.0;
            _vibratoSpeed = p.getParam ("vibratoSpeed") * p.getParam ("vibratoSpeed") * 0.01f;
            _vibratoAmplitude = p.getParam ("vibratoDepth") * 0.5f;
            
            _envelopeVolume = 0.0f;
            _envelopeStage = 0;
            _envelopeTime = 0.0f;
            _envelopeLength0 = p.getParam ("attackTime") * p.getParam ("attackTime") * 100000.0f;
            _envelopeLength1 = p.getParam ("sustainTime") * p.getParam ("sustainTime") * 100000.0f;
            _envelopeLength2 = p.getParam ("decayTime") * p.getParam ("decayTime") * 100000.0f + 10;
            _envelopeLength = _envelopeLength0;
            _envelopeFullLength = _envelopeLength0 + _envelopeLength1 + _envelopeLength2;
            
            _envelopeOverLength0 = 1.0f / _envelopeLength0;
            _envelopeOverLength1 = 1.0f / _envelopeLength1;
            _envelopeOverLength2 = 1.0f / _envelopeLength2;
            
            _flanger = p.getParam ("flangerOffset") != 0.0 || p.getParam ("flangerSweep") != 0.0;
            
            _flangerOffset = p.getParam ("flangerOffset") * p.getParam ("flangerOffset") * 1020.0f;
            if (p.getParam ("flangerOffset") < 0.0)
                _flangerOffset = -_flangerOffset;
            
            _flangerDeltaOffset = p.getParam ("flangerSweep") * p.getParam ("flangerSweep") * p.getParam ("flangerSweep") * 0.2f;
            _flangerPos = 0;
            
            _flangerBuffer.resize (1024);
            _noiseBuffer.resize (32);
            _pinkNoiseBuffer.resize (32);
            _loResNoiseBuffer.resize (32);
            _pinkNumber = {};
            
            for (int i = 0; i < 1024; i++)
                _flangerBuffer[i] = 0.0;

            for (int i = 0; i < 32; i++)
                _noiseBuffer[i] = float (uniformRandom()) * 2.0f - 1.0f;

            for (int i = 0; i < 32; i++)
                _pinkNoiseBuffer[i] = float (_pinkNumber.getNextValue());

            for (int i = 0; i < 32; i++)
                _loResNoiseBuffer[i] = ((i % LoResNoisePeriod) == 0) ? float (uniformRandom()) * 2.0f - 1.0f : _loResNoiseBuffer[i - 1];
        
            _repeatTime = 0;
            
            if (p.getParam ("repeatSpeed") == 0.0)
                _repeatLimit = 0;
            else
                _repeatLimit = int ((1.0f - p.getParam ("repeatSpeed")) * (1.0f - p.getParam ("repeatSpeed")) * 20000) + 32;
        }
    }
    
    /**
     * Writes the wave to the supplied buffer ByteArray
     * @param	buffer		A ByteArray to write the wave to
     * @return				If the wave is finished
     */
    bool synthWave (float* buffer, int start, int length)
    {
        _finished = false;
        
        _sampleCount = 0;
        _bufferSample = 0.0;
        
        for (int i = 0; i < length; i++)
        {
            if (_finished)
                return true;
            
            // Repeats every _repeatLimit times, partially resetting the sound parameters
            if (_repeatLimit != 0)
            {
                if (++_repeatTime >= _repeatLimit)
                {
                    _repeatTime = 0;
                    reset (false);
                }
            }
            
            _changePeriodTime++;
            if (_changePeriodTime >= _changePeriod)
            {
                _changeTime = 0;
                _changeTime2 = 0;
                _changePeriodTime = 0;
                if (_changeReached)
                {
                    _period /= _changeAmount;
                    _changeReached = false;
                }
                if (_changeReached2)
                {
                    _period /= _changeAmount2;
                    _changeReached2 = false;
                }
            }
            
            // If _changeLimit is reached, shifts the pitch
            if (!_changeReached)
            {
                if (++_changeTime >= _changeLimit)
                {
                    _changeReached = true;
                    _period *= _changeAmount;
                }
            }
            
            // If _changeLimit is reached, shifts the pitch
            if (!_changeReached2)
            {
                if (++_changeTime2 >= _changeLimit2)
                {
                    _period *= _changeAmount2;
                    _changeReached2 = true;
                }
            }
            
            // Acccelerate and apply slide
            _slide += _deltaSlide;
            _period *= _slide;
            
            // Checks for frequency getting too low, and stops the sound if a minFrequency was set
            if (_period > _maxPeriod)
            {
                _period = _maxPeriod;
                if (_minFreqency > 0.0)
                    _muted = true;
            }
            
            _periodTemp = _period;
            
            // Applies the vibrato effect
            if (_vibratoAmplitude > 0.0)
            {
                _vibratoPhase += _vibratoSpeed;
                _periodTemp = _period * (1.0f + std::sin (_vibratoPhase) * _vibratoAmplitude);
            }
            
            _periodTemp = std::floor (_periodTemp);
            if (_periodTemp < 8)
                _periodTemp = 8;
            
            // Sweeps the square duty
            if (_waveType == 0)
            {
                _squareDuty += _dutySweep;
                 if (_squareDuty < 0.0)
                     _squareDuty = 0.0;
                else if (_squareDuty > 0.5)
                    _squareDuty = 0.5;
            }
            
            // Moves through the different stages of the volume envelope
            if (++_envelopeTime > _envelopeLength)
            {
                _envelopeTime = 0;
                
                switch (++_envelopeStage)
                {
                    case 1: _envelopeLength = _envelopeLength1; break;
                    case 2: _envelopeLength = _envelopeLength2; break;
                }
            }
            
            // Sets the volume based on the position in the envelope
            switch (_envelopeStage)
            {
                case 0: _envelopeVolume = _envelopeTime * _envelopeOverLength0; 					   				   break;
                case 1: _envelopeVolume = 1.0f + (1.0f - _envelopeTime * _envelopeOverLength1) * 2.0f * _sustainPunch; break;
                case 2: _envelopeVolume = 1.0f - _envelopeTime * _envelopeOverLength2; 								   break;
                case 3: _envelopeVolume = 0.0; _finished = true; 													   break;
            }
            
            // Moves the flanger offset
            if (_flanger)
            {
                _flangerOffset += _flangerDeltaOffset;
                _flangerInt = int (_flangerOffset);
                
                if (_flangerInt < 0)
                    _flangerInt = -_flangerInt;
                else if (_flangerInt > 1023)
                    _flangerInt = 1023;
            }
            
            // Moves the high-pass filter cutoff
            if (_filters && _hpFilterDeltaCutoff != 0.0)
            {
                _hpFilterCutoff *= _hpFilterDeltaCutoff;
                
                if (_hpFilterCutoff < 0.00001f)
                    _hpFilterCutoff = 0.00001f;
                else if (_hpFilterCutoff > 0.1f)
                    _hpFilterCutoff = 0.1f;
            }
            
            _superSample = 0.0;
            for (int j = 0; j < 8; j++)
            {
                // Cycles through the period
                _phase++;
                if (_phase >= _periodTemp)
                {
                    _phase = int (_phase - _periodTemp);
                    
                    // Generates new random noise for this period
                    if (_waveType == 3)
                    {
                        for (int n = 0; n < 32; n++)
                            _noiseBuffer[n] = float (uniformRandom()) * 2.0f - 1.0f;
                    }
                    else if (_waveType == 5)
                    {
                        for (int n = 0; n < 32; n++)
                            _pinkNoiseBuffer[n] = float (_pinkNumber.getNextValue());
                    }
                    else if (_waveType == 6)
                    {
                        for (int n = 0; n < 32; n++)
                            _loResNoiseBuffer[n] = ((n % LoResNoisePeriod) == 0) ? float (uniformRandom()) * 2.0f - 1.0f : _loResNoiseBuffer[n - 1];
                    }
                }
                
                _sample = 0;
                float overtonestrength = 1;
                for (int k = 0; k <= _overtones; k++)
                {
                    float tempphase = (float) std::fmod ((_phase * (k + 1)), _periodTemp);
                    // Gets the sample from the oscillator
                    switch (_waveType)
                    {
                        case 0: // Square wave
                        {
                            _sample += overtonestrength * (((tempphase / _periodTemp) < _squareDuty) ? 0.5f : -0.5f);
                            break;
                        }
                        case 1: // Saw wave
                        {
                            _sample += overtonestrength * (1.0f - (tempphase / _periodTemp) * 2.0f);
                            break;
                        }
                        case 2: // Sine wave (fast and accurate approx)
                        {
                             _pos = tempphase / _periodTemp;
                             _pos = _pos > 0.5f ? (_pos - 1.0f) * 6.28318531f : _pos * 6.28318531f;
                            float _tempsample = _pos < 0 ? 1.27323954f * _pos + 0.405284735f * _pos * _pos : 1.27323954f * _pos - 0.405284735f * _pos * _pos;
                            _sample += overtonestrength * (_tempsample < 0 ? 0.225f * (_tempsample * -_tempsample - _tempsample) + _tempsample : 0.225f * (_tempsample * _tempsample - _tempsample) + _tempsample);
                            break;
                        }
                        case 3: // Noise
                        {
                            _sample += overtonestrength * (_noiseBuffer[(unsigned int)(tempphase * 32 / int (_periodTemp)) % 32]);
                            break;
                        }
                        case 4: // Triangle Wave
                        {
                            _sample += overtonestrength * (std::abs (1 - (tempphase / _periodTemp) * 2) - 1);
                            break;
                        }
                        case 5: // Pink Noise
                        {
                            _sample += overtonestrength * (_pinkNoiseBuffer [int (tempphase * 32 / int (_periodTemp)) % 32]);
                            break;
                        }
                        case 6: // tan
                        {
                            //detuned
                            _sample += std::tan (float (pi) * tempphase / _periodTemp) * overtonestrength;
                            break;
                        }
                        case 7: // Whistle
                        {
                            // Sin wave code
                            _pos = tempphase / _periodTemp;
                            _pos = _pos > 0.5f ? (_pos - 1.0f) * 6.28318531f : _pos * 6.28318531f;
                            float _tempsample = _pos < 0 ? 1.27323954f * _pos + 0.405284735f * _pos * _pos : 1.27323954f * _pos - 0.405284735f * _pos * _pos;
                            float value = 0.75f * (_tempsample < 0 ? 0.225f * (_tempsample * -_tempsample - _tempsample) + _tempsample : 0.225f * (_tempsample * _tempsample - _tempsample) + _tempsample);
                            //then whistle (essentially an overtone with frequencyx20 and amplitude0.25
                            
                            _pos = std::fmod ((tempphase * 20), _periodTemp) / _periodTemp;
                            _pos = _pos > 0.5f ? (_pos - 1.0f) * 6.28318531f : _pos * 6.28318531f;
                            _tempsample = _pos < 0 ? 1.27323954f * _pos + 0.405284735f * _pos * _pos : 1.27323954f * _pos - 0.405284735f * _pos * _pos;
                            value += 0.25f * (_tempsample < 0 ? 0.225f * (_tempsample * -_tempsample - _tempsample) + _tempsample : 0.225f * (_tempsample * _tempsample - _tempsample) + _tempsample);
                            
                            _sample += overtonestrength * value;//main wave
                            
                            break;
                        }
                        case 8: // Breaker
                        {
                            float amp = tempphase / _periodTemp;
                            _sample += overtonestrength * (std::abs (1 - amp * amp * 2) - 1);
                            break;
                        }
                    }
                    overtonestrength *= (1 - _overtoneFalloff);
                    
                }
                
                // Applies the low and high pass filters
                if (_filters)
                {
                    _lpFilterOldPos = _lpFilterPos;
                    _lpFilterCutoff *= _lpFilterDeltaCutoff;
                    
                     if (_lpFilterCutoff < 0.0f)
                         _lpFilterCutoff = 0.0f;
                    else if (_lpFilterCutoff > 0.1f)
                        _lpFilterCutoff = 0.1f;
                    
                    if (_lpFilterOn)
                    {
                        _lpFilterDeltaPos += (_sample - _lpFilterPos) * _lpFilterCutoff;
                        _lpFilterDeltaPos *= _lpFilterDamping;
                    }
                    else
                    {
                        _lpFilterPos = _sample;
                        _lpFilterDeltaPos = 0.0f;
                    }
                    
                    _lpFilterPos += _lpFilterDeltaPos;
                    
                    _hpFilterPos += _lpFilterPos - _lpFilterOldPos;
                    _hpFilterPos *= 1.0f - _hpFilterCutoff;
                    _sample = _hpFilterPos;
                }
                
                // Applies the flanger effect
                if (_flanger)
                {
                    _flangerBuffer[_flangerPos&1023] = _sample;
                    _sample += _flangerBuffer[(_flangerPos - _flangerInt + 1024) & 1023];
                    _flangerPos = (_flangerPos + 1) & 1023;
                }
                
                _superSample += _sample;
            }
            
            // Clipping if too loud
            if (_superSample > 8.0f)
                _superSample = 8.0f;
            else if (_superSample < -8.0f)
                _superSample = -8.0f;
            
            // Averages out the super samples and applies volumes
            _superSample = _masterVolume * _envelopeVolume * _superSample * 0.125f;
            
            //BIT CRUSH
            _bitcrush_phase += _bitcrush_freq;
            if (_bitcrush_phase > 1)
            {
                _bitcrush_phase = 0;
                _bitcrush_last = _superSample;
            }
            
            _bitcrush_freq = std::max (std::min (_bitcrush_freq + _bitcrush_freq_sweep, 1.0f), 0.0f);
            
            _superSample = _bitcrush_last;
        
             //compressor
             if (_superSample > 0)
                 _superSample = std::pow (_superSample, _compression_factor);
             else
                 _superSample = -std::pow (-_superSample, _compression_factor);
            
             if (_muted)
                 _superSample = 0;
            
            buffer[start + i] += _superSample;
        }
        
        return false;
    }
    
private:
    //--------------------------------------------------------------------------
    //
    //  Sound Parameters
    //
    //--------------------------------------------------------------------------
    
    const float MIN_LENGTH = 0.18f;
    const int version = 104;
    //should be <32
    const int LoResNoisePeriod = 8;
    
	float sampleRate = 44100.0f;
    SfxrParams _params;                      // Params instance
    
    //--------------------------------------------------------------------------
    //
    //  Synth Variables
    //
    //--------------------------------------------------------------------------
    
    bool _finished;                           // If the sound has finished
    
    float _masterVolume;                      // masterVolume * masterVolume (for quick calculations)
    
    unsigned int _waveType;                   // The type of wave to generate
    
    float _envelopeVolume;                    // Current volume of the envelope
    int _envelopeStage;                       // Current stage of the envelope (attack, sustain, decay, end)
    float _envelopeTime;                      // Current time through current enelope stage
    float _envelopeLength;                    // Length of the current envelope stage
    float _envelopeLength0;                   // Length of the attack stage
    float _envelopeLength1;                   // Length of the sustain stage
    float _envelopeLength2;                   // Length of the decay stage
    float _envelopeOverLength0;               // 1 / _envelopeLength0 (for quick calculations)
    float _envelopeOverLength1;               // 1 / _envelopeLength1 (for quick calculations)
    float _envelopeOverLength2;               // 1 / _envelopeLength2 (for quick calculations)
    float _envelopeFullLength;                // Full length of the volume envelop (and therefore sound)
    
    float _sustainPunch;                      // The punch factor (louder at begining of sustain)
    
    int _phase;                               // Phase through the wave
    float _pos;                               // Phase expresed as a Number from 0-1, used for fast sin approx
    float _period;                            // Period of the wave
    float _periodTemp;                        // Period modified by vibrato
    float _maxPeriod;                         // Maximum period before sound stops (from minFrequency)
    
    float _slide;                             // Note slide
    float _deltaSlide;                        // Change in slide
    float _minFreqency;                       // Minimum frequency before stopping
    bool _muted;                              // Whether or not min frequency has been attained
    
    int _overtones;                           // Minimum frequency before stopping
    float _overtoneFalloff;                   // Minimum frequency before stopping
    
    float _vibratoPhase;                      // Phase through the vibrato sine wave
    float _vibratoSpeed;                      // Speed at which the vibrato phase moves
    float _vibratoAmplitude;                  // Amount to change the period of the wave by at the peak of the vibrato wave
    
    float _changePeriod;
    int _changePeriodTime;
    
    float _changeAmount;                      // Amount to change the note by
    int _changeTime;                          // Counter for the note change
    int _changeLimit;                         // Once the time reaches this limit, the note changes
    bool _changeReached;
    
    float _changeAmount2;                     // Amount to change the note by
    int _changeTime2;                         // Counter for the note change
    int _changeLimit2;                        // Once the time reaches this limit, the note changes
    bool _changeReached2;
        
    float _squareDuty;                        // Offset of center switching point in the square wave
    float _dutySweep;                         // Amount to change the duty by
    
    int _repeatTime;                          // Counter for the repeats
    int _repeatLimit;                         // Once the time reaches this limit, some of the variables are reset
    
    bool _flanger;                            // If the flanger is active
    float _flangerOffset;                     // Phase offset for flanger effect
    float _flangerDeltaOffset;                // Change in phase offset
    int _flangerInt;                          // Integer flanger offset, for bit maths
    int _flangerPos;                          // Position through the flanger buffer
    std::vector<float> _flangerBuffer;        // Buffer of wave values used to create the out of phase second wave
    
    bool _filters;                            // If the filters are active
    float _lpFilterPos;                       // Adjusted wave position after low-pass filter
    float _lpFilterOldPos;                    // Previous low-pass wave position
    float _lpFilterDeltaPos;                  // Change in low-pass wave position, as allowed by the cutoff and damping
    float _lpFilterCutoff;                    // Cutoff multiplier which adjusts the amount the wave position can move
    float _lpFilterDeltaCutoff;               // Speed of the low-pass cutoff multiplier
    float _lpFilterDamping;                   // Damping muliplier which restricts how fast the wave position can move
    bool _lpFilterOn;                         // If the low pass filter is active
    
    float _hpFilterPos;                       // Adjusted wave position after high-pass filter
    float _hpFilterCutoff;                    // Cutoff multiplier which adjusts the amount the wave position can move
    float _hpFilterDeltaCutoff;               // Speed of the high-pass cutoff multiplier
    
    std::vector<float> _noiseBuffer;          // Buffer of random values used to generate noise
    std::vector<float> _pinkNoiseBuffer;      // Buffer of random values used to generate noise
    std::vector<float> _loResNoiseBuffer;     // Buffer of random values used to generate noise
    
    PinkNumber _pinkNumber;
    
    float _superSample;                       // Actual sample writen to the wave
    float _sample;                            // Sub-sample calculated 8 times per actual sample, averaged out to get the super sample
    unsigned int _sampleCount;                // Number of samples added to the buffer sample
    float _bufferSample;                      // Another supersample used to create a 22050Hz wave
    
    float _bitcrush_freq;                     // inversely proportional to the number of samples to skip
    float _bitcrush_freq_sweep;               // change of the above
    float _bitcrush_phase;                    // samples when this > 1
    float _bitcrush_last;                     // last sample value
    
    float _compression_factor;
};
