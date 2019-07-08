#pragma once
/*
class taken from http://www.firstpr.com.au/dsp/pink-noise/#Filtering
*/
#include <vector>

#include "Util.h"

class PinkNumber
{
public:
    PinkNumber()
    {
        max_key = 0x1f; // Five bits set
        range = 128;
        key = 0;
                
        for (int i = 0; i < 5; i++)
            white_values.push_back (int (uniformRandom() * (range / 5)));
    }
    
    //returns number between -1 and 1
    double getNextValue()
    {
        int last_key = key;
        unsigned int sum;
        
        key++;
        if (key > max_key)
            key = 0;
        
        // Exclusive-Or previous value with current value. This gives
        // a list of bits that have changed.
        int diff = last_key ^ key;
        sum = 0;
        for (int i = 0; i < 5; i++)
        {
            // If bit changed get new random number for corresponding
            // white_value
            if (diff & (1 << i))
                white_values[i] = int (uniformRandom() * (range / 5));
            sum += white_values[i];
        }
        return sum / 64.0 - 1.0;
    }

private:
    int max_key;
    int key;
    std::vector<int> white_values;
    unsigned int range;
};
