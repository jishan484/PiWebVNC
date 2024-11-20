/*
    vncserver.hpp - source code

    =================================
      vnc class for PIwebVNC in C++
    =================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.1
*/

#ifndef XRECT_HPP
#define XRECT_HPP

#include <X11/Xlib.h>
#include "appConfigs.hpp"

struct RectLookup
{
    XRectangle rect;
    int frequency = -1;
};

struct LOOKUP_MANAGER{
    int index = 0;
    int min_freq_index = -1;
    int lastMatchedFrequency = 0;
    RectLookup LOOKUPS[MAX_LOOKUP_COUNTS];

    bool match(XRectangle rect){
        bool isMatched = false;
        int minFreq = -1;
        for (int i = 0; i < MAX_LOOKUP_COUNTS; i++)
        {
            if (rect.x == LOOKUPS[i].rect.x && rect.y == LOOKUPS[i].rect.y && rect.width == LOOKUPS[i].rect.width && rect.height == LOOKUPS[i].rect.height)
            {
                isMatched = true;
                this->lastMatchedFrequency = LOOKUPS[i].frequency;
            }
            if(LOOKUPS[i].frequency < minFreq){
                minFreq = LOOKUPS[i].frequency;
                this->min_freq_index = i;
            }
        }
        return isMatched;
    }

    void push(XRectangle rect){
        RectLookup lookup;
        lookup.rect = rect;
        lookup.frequency = 0;
        if(this->index < MAX_LOOKUP_COUNTS) {
            LOOKUPS[this->index++] = lookup;
        } else {
            LOOKUPS[this->min_freq_index] = lookup;
        }
    }
} myLOOKUP_MANAGER;

class OptimizationManager{
public:
    int checkOptimization(XRectangle rect);
} optimizationManager;

int OptimizationManager::checkOptimization(XRectangle rect)
{
    if (myLOOKUP_MANAGER.match(rect))
    {
        if (myLOOKUP_MANAGER.lastMatchedFrequency < 10)
            return 80;
        else if (myLOOKUP_MANAGER.lastMatchedFrequency < 30)
            return 50;
        else
            return 20;
    }
    else
    {
        myLOOKUP_MANAGER.push(rect);
        return 100;
    }
    return 100;
}

#endif