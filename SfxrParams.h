/**
 * SfxrParams
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

#include <map>

#include "Util.h"

class Param
{
public:
    Param() = default;

    Param (std::string name_,
           std::string description_,
           int grouping_,
           std::string uid_,
           float defaultValue_,
           float minValue_,
           float maxValue_)
      : name (name_),
        description (description_),
        grouping (grouping_),
        uid (uid_),
        defaultValue (defaultValue_),
        minValue (minValue_),
        maxValue (maxValue_),
        currentValue (defaultValue)
    {
    }

    std::string name;
    std::string description;
    int grouping = 0;
    std::string uid;
    float defaultValue = 0.0f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float currentValue = 0.0f;
};

class SfxrParams
{
public:
    //--------------------------------------------------------------------------
    //
    //  Getters / Setters
    //
    //--------------------------------------------------------------------------
    
    SfxrParams()
    {
        resetParams();
    }

    std::vector<std::string> getParams()
    {
        std::vector<std::string> uids;
        for (auto& p : params)
            uids.push_back (p.uid);

        return uids;
    }

    std::string getName (std::string param)
    {
        for (auto p : params)
            if (p.uid == param)
                return p.name;

        return {};
    }

    std::string getDescription (std::string param)
    {
        for (auto p : params)
            if (p.uid == param)
                return p.description;

        return {};
    }

    float getDefault (std::string param)
    {
        return getProperty (param, 4);
    }
    
    float getMin (std::string param)
    {
        return getProperty (param, 5);
    }
    
    float getMax (std::string param)
    {
        return getProperty (param, 6);
    }
    
    float getProperty (std::string param, int index)
    {
        for (auto p : params)
        {
            if (p.uid == param)
            {
                switch (index)
                {
                    case 4: return p.defaultValue;
                    case 5: return p.minValue;
                    case 6: return p.maxValue;
                    case 7: return p.currentValue;
                    default: return 0;
                }
            }
        }
        return 0;
    }
    
    float getParam (std::string param)
    {
        return getProperty (param, 7);
    }
    
    void setParam (std::string param, float value)
    {
        for (auto& p : params)
            if (p.uid == param)
                p.currentValue = clamp (value, getMin (param), getMax (param));
        
        paramsDirty = true;
    }
    
    /** Returns true if this parameter is locked */
    bool lockedParam (std::string param)
    {
        return std::find (lockedParams.begin(), lockedParams.end(), param) != lockedParams.end();
    }
    
    void setAllLocked (bool locked)
    {
        lockedParams.clear();
        
        if (locked)
            for (auto p : params)
                lockedParams.push_back (p.uid);

        paramsDirty = true;
    }
    
    void setParamLocked (std::string param, bool locked)
    {
        auto itr = std::find (lockedParams.begin(), lockedParams.end(), param);
        
        if (locked)
        {
            if (itr == lockedParams.end())
            {
                lockedParams.push_back (param);
                paramsDirty = true;
            }
        }
        else
        {
            if (itr != lockedParams.end())
            {
                lockedParams.erase (itr);
                paramsDirty = true;
            }
        }
    }
    
    //--------------------------------------------------------------------------
    //
    //  Generator Methods
    //
    //--------------------------------------------------------------------------
    
    /**
     * Sets the parameters to generate a pickup/coin sound
     */
    void generatePickupCoin()
    {
        resetParams();
        
        setParam ("startFrequency",0.4+uniformRandom()*0.5);
        
        setParam ("sustainTime", uniformRandom() * 0.1);
        setParam ("decayTime", 0.1 + uniformRandom() * 0.4);
        setParam ("sustainPunch", 0.3 + uniformRandom() * 0.3);
        
        if (uniformRandom() < 0.5)
        {
            setParam ("changeSpeed", 0.5 + uniformRandom() * 0.2);
            int cnum = int(uniformRandom()*7)+1;
            int cden = cnum+int(uniformRandom()*7)+2;
            
            setParam ("changeAmount", float(cnum)/float(cden));
        }
    }
    
    /**
     * Sets the parameters to generate a laser/shoot sound
     */
    void generateLaserShoot()
    {
        resetParams();
        
        setParam ("waveType", int (uniformRandom() * 3));
        if (int (getParam ("waveType")) == 2 && uniformRandom() < 0.5)
            setParam ("waveType", int (uniformRandom() * 2));
        
        setParam ("startFrequency", 0.5 + uniformRandom() * 0.5);
        setParam ("minFrequency", getParam ("startFrequency") - 0.2 - uniformRandom() * 0.6);
        
        if (getParam ("minFrequency") < 0.2)
            setParam ("minFrequency", 0.2);
        
        setParam ("slide", -0.15 - uniformRandom() * 0.2);
         
        if (uniformRandom() < 0.33)
        {
            setParam ("startFrequency", uniformRandom() * 0.6);
            setParam ("minFrequency", uniformRandom() * 0.1);
            setParam ("slide", -0.35 - uniformRandom() * 0.3);
        }
        
        if (uniformRandom() < 0.5)
        {
            setParam ("squareDuty", uniformRandom() * 0.5);
            setParam ("dutySweep", uniformRandom() * 0.2);
        }
        else
        {
            setParam ("squareDuty", 0.4 + uniformRandom() * 0.5);
            setParam ("dutySweep",- uniformRandom() * 0.7);
        }
        
        setParam ("sustainTime", 0.1 + uniformRandom() * 0.2);
        setParam ("decayTime", uniformRandom() * 0.4);
        if (uniformRandom() < 0.5) setParam ("sustainPunch", uniformRandom() * 0.3);
        
        if (uniformRandom() < 0.33)
        {
            setParam ("flangerOffset", uniformRandom() * 0.2);
            setParam ("flangerSweep", -uniformRandom() * 0.2);
        }
        
        if (uniformRandom() < 0.5)
            setParam ("hpFilterCutoff", uniformRandom() * 0.3);
    }
    
    /**
     * Sets the parameters to generate an explosion sound
     */
    void generateExplosion()
    {
        resetParams();
        setParam ("waveType", 3);
        
        if (uniformRandom() < 0.5)
        {
            setParam ("startFrequency", 0.1 + uniformRandom() * 0.4);
            setParam ("slide", -0.1 + uniformRandom() * 0.4);
        }
        else
        {
            setParam ("startFrequency", 0.2 + uniformRandom() * 0.7);
            setParam ("slide", -0.2 - uniformRandom() * 0.2);
        }
        
        setParam ("startFrequency", getParam ("startFrequency") * getParam ("startFrequency"));
        
        if (uniformRandom() < 0.2)
            setParam ("slide", 0.0);
        
        if (uniformRandom() < 0.33)
            setParam ("repeatSpeed", 0.3 + uniformRandom() * 0.5);
        
        setParam ("sustainTime", 0.1 + uniformRandom() * 0.3);
        setParam ("decayTime", uniformRandom() * 0.5);
        setParam ("sustainPunch", 0.2 + uniformRandom() * 0.6);
        
        if (uniformRandom() < 0.5)
        {
            setParam ("flangerOffset", -0.3 + uniformRandom() * 0.9);
            setParam ("flangerSweep", -uniformRandom() * 0.3);
        }
        
        if (uniformRandom() < 0.33)
        {
            setParam ("changeSpeed", 0.6 + uniformRandom() * 0.3);
            setParam ("changeAmount", 0.8 - uniformRandom() * 1.6);
        }
    }
    
    /**
     * Sets the parameters to generate a powerup sound
     */
    void generatePowerup()
    {
        resetParams();
        
        if (uniformRandom() < 0.5)
            setParam ("waveType", 1);
        else
            setParam ("squareDuty", uniformRandom() * 0.6);
        
        if (uniformRandom() < 0.5)
        {
            setParam ("startFrequency", 0.2 + uniformRandom() * 0.3);
            setParam ("slide", 0.1 + uniformRandom() * 0.4);
            setParam ("repeatSpeed", 0.4 + uniformRandom() * 0.4);
        }
        else
        {
            setParam ("startFrequency", 0.2 + uniformRandom() * 0.3);
            setParam ("slide", 0.05 + uniformRandom() * 0.2);
            
            if (uniformRandom() < 0.5)
            {
                setParam ("vibratoDepth", uniformRandom() * 0.7);
                setParam ("vibratoSpeed", uniformRandom() * 0.6);
            }
        }
        
        setParam ("sustainTime", uniformRandom() * 0.4);
        setParam ("decayTime", 0.1 + uniformRandom() * 0.4);
    }
    
    /**
     * Sets the parameters to generate a hit/hurt sound
     */
    void generateHitHurt()
    {
        resetParams();
        
        setParam ("waveType", int (uniformRandom() * 3));
        if (int (getParam ("waveType")) == 2)
            setParam ("waveType", 3);
        else if (int (getParam ("waveType")) == 0)
            setParam ("squareDuty", uniformRandom() * 0.6);
        
        setParam ("startFrequency", 0.2 + uniformRandom() * 0.6);
        setParam ("slide", -0.3 - uniformRandom() * 0.4);
        
        setParam ("sustainTime", uniformRandom() * 0.1);
        setParam ("decayTime", 0.1 + uniformRandom() * 0.2);
        
        if (uniformRandom() < 0.5)
            setParam ("hpFilterCutoff", uniformRandom() * 0.3);
    }
    
    /**
     * Sets the parameters to generate a jump sound
     */
    void generateJump()
    {
        resetParams();
        
        setParam ("waveType", 0);
        setParam ("squareDuty", uniformRandom() * 0.6);
        setParam ("startFrequency", 0.3 + uniformRandom() * 0.3);
        setParam ("slide", 0.1 + uniformRandom() * 0.2);
        
        setParam ("sustainTime", 0.1 + uniformRandom() * 0.3);
        setParam ("decayTime", 0.1 + uniformRandom() * 0.2);
        
        if (uniformRandom() < 0.5) setParam ("hpFilterCutoff", uniformRandom() * 0.3);
        if (uniformRandom() < 0.5) setParam ("lpFilterCutoff", 1.0 - uniformRandom() * 0.6);
    }
    
    /**
     * Sets the parameters to generate a blip/select sound
     */
    void generateBlipSelect()
    {
        resetParams();
        
        setParam ("waveType", int (uniformRandom() * 2));
        if (int (getParam ("waveType")) == 0)
            setParam ("squareDuty", uniformRandom() * 0.6);
        
        setParam ("startFrequency", 0.2 + uniformRandom() * 0.4);
        
        setParam ("sustainTime", 0.1 + uniformRandom() * 0.1);
        setParam ("decayTime", uniformRandom() * 0.2);
        setParam ("hpFilterCutoff", 0.1);
    }
    
    /**
     * Resets the parameters, used at the start of each generate function
     */
    void resetParams()
    {
        paramsDirty = true;
        
        for (auto& p : params)
            p.currentValue = p.defaultValue;
    
        lockedParams.clear();
        lockedParams.push_back ("masterVolume");
    }
    
    //--------------------------------------------------------------------------
    //
    //  Randomize Methods
    //
    //--------------------------------------------------------------------------
    
    /**
     * Randomly adjusts the parameters ever so slightly
     */
    void mutate (float mutation = 0.05)
    {
        for (auto& p : params)
        {
            if (! lockedParam(p.uid))
            {
                if (uniformRandom() < 0.5)
                {
                    setParam (p.uid, p.currentValue + uniformRandom() * mutation * 2 - mutation);
                }
            }
        }
    }
    
    //some constants used for weighting random values
    
    std::map<std::string, float> randomizationPower
    {
        { "attackTime", 4},
        { "sustainTime", 2},
        { "sustainPunch", 2},
        { "overtones", 3},
        { "overtoneFalloff", 0.25},
        { "vibratoDepth", 3},
        { "dutySweep", 3},
        { "flangerOffset", 3},
        { "flangerSweep", 3},
        { "lpFilterCutoff", 0.3},
        { "lpFilterSweep", 3},
        { "hpFilterCutoff", 5},
        { "hpFilterSweep", 5},
        { "bitCrush", 4},
        { "bitCrushSweep", 5},
    };
    
    std::vector<int> waveTypeWeights =
    {
        1,//0:square
        1,//1:saw
        1,//2:sin
        1,//3:noise
        1,//4:triangle
        1,//5:pink
        1,//6:tan
        1,//7:whistle
        1,//8:breaker
    };
    
    /**
     * Sets all parameters to random values
     * If passed null, no fields constrained
     */
    void randomize()
    {
        for (auto& p : params)
        {
            if (! lockedParam (p.uid))
            {
                auto min = getMin (p.uid);
                auto max = getMax (p.uid);
                
                auto r = uniformRandom();
                
                auto itr = randomizationPower.find (p.uid);
                if (itr != randomizationPower.end())
                    r = std::pow (r, itr->second);
                
                p.currentValue = min + ( max - min) * r;
            }
        }
        
        paramsDirty = true;
        
        if (! lockedParam ("waveType"))
        {
            int count = 0;
            for (auto weight : waveTypeWeights)
                count += weight;
            
            float r = uniformRandom() * count;
            for (int i = 0; i < waveTypeWeights.size(); i++)
            {
                r -= waveTypeWeights[i];
                if (r <= 0)
                {
                    setParam("waveType",i);
                    break;
                }
            }
            
        }
        
        if (! lockedParam ("repeatSpeed"))
        {
            if (uniformRandom() < 0.5)
                setParam ("repeatSpeed",0);
        }
        
        if (! lockedParam ("slide"))
        {
            float r = uniformRandom() * 2 - 1;
            r = std::pow (r, 5);
            setParam ("slide", r);
        }
        if (! lockedParam ("deltaSlide"))
        {
            float r=uniformRandom() * 2 - 1;
            r=std::pow (r, 3);
            setParam ("deltaSlide", r);
        }
        
        if (! lockedParam ("minFrequency"))
            setParam ("minFrequency", 0);
        
        if (! lockedParam ("startFrequency"))
            setParam("startFrequency", (uniformRandom() < 0.5) ? std::pow (uniformRandom() * 2 - 1, 2) : (std::pow (uniformRandom() * 0.5, 3) + 0.5));
        
        if ((! lockedParam("sustainTime")) && (! lockedParam("decayTime")))
        {
            if (getParam ("attackTime") + getParam ("sustainTime") + getParam ("decayTime") < 0.2)
            {
                setParam ("sustainTime", 0.2 + uniformRandom() * 0.3);
                setParam ("decayTime", 0.2 + uniformRandom() * 0.3);
            }
        }
        
        if (! lockedParam ("slide"))
        {
            if ((getParam ("startFrequency") > 0.7 && getParam ("slide") > 0.2) || (getParam ("startFrequency") < 0.2 && getParam ("slide") < -0.05))
            {
                setParam ("slide", -getParam("slide"));
            }
        }
        
        if (! lockedParam ("lpFilterCutoffSweep"))
        {
            if (getParam ("lpFilterCutoff") < 0.1 && getParam ("lpFilterCutoffSweep") < -0.05)
            {
                setParam ("lpFilterCutoffSweep", -getParam ("lpFilterCutoffSweep"));
            }
        }
    }
    
    //--------------------------------------------------------------------------
    //
    //  Util Methods
    //
    //--------------------------------------------------------------------------
    
    /**
     * Clams a value to betwen 0 and 1
     * @param	value	Input value
     * @return			The value clamped between 0 and 1
     */
    inline float clamp1 (float value)
    {
        return (value > 1.0) ? 1.0 : ((value < 0.0) ? 0.0 : value);
    }
    
    /**
     * Clams a value to betwen -1 and 1
     * @param	value	Input value
     * @return			The value clamped between -1 and 1
     */
    inline float clamp2 (float value)
    {
        return (value > 1.0) ? 1.0 : ((value < -1.0) ? -1.0 : value);
    }
    
    /**
     * Clams a value to betwen a and b
     * @param	value	Input value
     * @param	min		min value
     * @param	max		max value
     * @return			The value clamped between min and max
     */
    inline float clamp (float value, float min, float max)
    {
        return (value > max) ? max : ((value < min) ? min : value);
    }
    
    //--------------------------------------------------------------------------
    //
    //  Properties
    //
    //--------------------------------------------------------------------------
    
    static const int WAVETYPECOUNT;
    
    /** If the parameters have been changed since last time (shouldn't used cached sound) */
    bool paramsDirty = true;
    
    //interface uses this to disable square sliders when non-square wavetype selected
    std::vector<std::string> squareParams = {"squareDuty","dutySweep"};
    
    //params to exclude from list
    std::vector<std::string> excludeParams = {"waveType", "masterVolume"};
    
    std::vector<Param> params = {
        // real name, decription, grouping,name, default, min, max,
        {"Wave Type","Shape of the wave.", 0,"waveType",2,0,WAVETYPECOUNT-0.0001f}, // the 6.999 thing is because this is really an int parameter...
        
        {"Master Volume","Overall volume of the sound.", 1,"masterVolume",0.5,0,1},
        {"Attack Time","Length of the volume envelope attack.", 1,"attackTime",0,0,1},
        {"Sustain Time","Length of the volume envelope sustain.", 1,"sustainTime",0.3,0,1},
        {"Punch","Tilts the sustain envelope for more 'pop'.", 1,"sustainPunch",0,0,1},
        {"Decay Time","Length of the volume envelope decay (yes, I know it's called release).", 1,"decayTime",0.4,0,1},
        
        {"Compression","Pushes amplitudes together into a narrower range to make them stand out more.  Very good for sound effects, where you want them to stick out against background music.",15,"compressionAmount",0.3,0,1},
        
        {"Frequency","Base note of the sound.", 2,"startFrequency",0.3,0,1},
        {"Frequency Cutoff","If sliding, the sound will stop at this frequency, to prevent really low notes.  If unlocked, this is set to zero during randomization.", 2,"minFrequency",0.0,0,1},
        
        {"Frequency Slide","Slides the frequency up or down.", 3, "slide",0.0,-1,1},
        {"Delta Slide","Accelerates the frequency slide.  Can be used to get the frequency to change direction.", 3,"deltaSlide",0.0,-1,1},
        
        {"Vibrato Depth","Strength of the vibrato effect.", 4,"vibratoDepth",0,0,1},
        {"Vibrato Speed","Speed of the vibrato effect (i.e. frequency).", 4,"vibratoSpeed",0,0,1},
        
        {"Harmonics","Overlays copies of the waveform with copies and multiples of its frequency.  Good for bulking out or otherwise enriching the texture of the sounds (warning: this is the number 1 cause of bfxr slowdown!).", 13,"overtones",0,0,1},
        {"Harmonics Falloff","The rate at which higher overtones should decay.", 13,"overtoneFalloff",0,0,1},
        
        {"Pitch Jump Repeat Speed","Larger Values means more pitch jumps, which can be useful for arpeggiation.", 5,"changeRepeat",0,0,1},
        {"Pitch Jump Amount 1","Jump in pitch, either up or down.", 5,"changeAmount",0,-1,1},
        {"Pitch Jump Onset 1","How quickly the note shift happens.", 5,"changeSpeed",0,0,1},
        {"Pitch Jump Amount 2","Jump in pitch, either up or down.", 5,"changeAmount2",0,-1,1},
        {"Pitch Jump Onset 2","How quickly the note shift happens.", 5,"changeSpeed2",0,0,1},
        
        {"Square Duty","Square waveform only : Controls the ratio between the up and down states of the square wave, changing the tibre.", 8,"squareDuty",0,0,1},
        {"Duty Sweep","Square waveform only : Sweeps the duty up or down.", 8,"dutySweep",0,-1,1},
        
        {"Repeat Speed","Speed of the note repeating - certain variables are reset each time.", 9,"repeatSpeed",0,0,1},
        
        {"Flanger Offset","Offsets a second copy of the wave by a small phase, changing the tibre.", 10,"flangerOffset",0,-1,1},
        {"Flanger Sweep","Sweeps the phase up or down.", 10,"flangerSweep",0,-1,1},
        
        {"LP Filter Cutoff","Frequency at which the low-pass filter starts attenuating higher frequencies.  Named most likely to result in 'Huh why can't I hear anything?' at her high-school grad. ", 11,"lpFilterCutoff",1,0,1},
        {"LP Filter Cutoff Sweep","Sweeps the low-pass cutoff up or down.", 11,"lpFilterCutoffSweep",0,-1,1},
        {"LP Filter Resonance","Changes the attenuation rate for the low-pass filter, changing the timbre.", 11,"lpFilterResonance",0,0,1},
        
        {"HP Filter Cutoff","Frequency at which the high-pass filter starts attenuating lower frequencies.", 12,"hpFilterCutoff",0,0,1},
        {"HP Filter Cutoff Sweep","Sweeps the high-pass cutoff up or down.", 12,"hpFilterCutoffSweep",0,-1,1},
        
        {"Bit Crush","Resamples the audio at a lower frequency.", 14,"bitCrush",0,0,1},
        {"Bit Crush Sweep","Sweeps the Bit Crush filter up or down.", 14,"bitCrushSweep",0,-1,1}
        
    };
    
    std::vector<std::string> lockedParams; // stores list of strings, these strings represent parameters that will be locked during randomization/mutation

};
