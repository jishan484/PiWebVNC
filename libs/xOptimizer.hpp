/*
    xOptimier.hpp - source code

    ====================================
      lookup class for PIwebVNC in C++
    ====================================

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
#include <iostream>
#include "appConfigs.hpp"
#include <climits>

struct RectLookup
{
    XRectangle rect;
    int frequency = -1;  // -1 indicates an unused or empty entry
    short encounter = 0; // Tracks the number of checks on this rectangle
};

struct LOOKUP_MANAGER
{
    int index = 0;
    int min_freq_index = -1;
    int lastMatchedFrequency = 0;
    RectLookup LOOKUPS[MAX_LOOKUP_COUNTS];

    // Match function to check if a rectangle already exists in the lookup table
    bool match(XRectangle rect)
    {
        bool isMatched = false;
        int minFreq = INT_MAX;
        this->min_freq_index = -1;

        for (int i = 0; i < MAX_LOOKUP_COUNTS; i++)
        {
            if (LOOKUPS[i].frequency == -1)
                break; // Stop if we've reached unused entries

            // Increment the encounter count
            LOOKUPS[i].encounter++;

            // Check if this rectangle matches
            if (rect.x == LOOKUPS[i].rect.x && rect.y == LOOKUPS[i].rect.y &&
                rect.width == LOOKUPS[i].rect.width && rect.height == LOOKUPS[i].rect.height)
            {
                isMatched = true;
                this->lastMatchedFrequency = LOOKUPS[i].frequency;
                LOOKUPS[i].frequency++;   // Increment the frequency as it's matched
                LOOKUPS[i].encounter = 0; // Reset encounter count as it's a match
            }

            // Track the least frequent, but encountered rectangle
            if (LOOKUPS[i].frequency < minFreq && LOOKUPS[i].encounter > 0)
            {
                minFreq = LOOKUPS[i].frequency;
                this->min_freq_index = i;
            }
        }
        return isMatched;
    }

    // Push a new rectangle into the lookup manager or replace an existing one
    void push(XRectangle rect)
    {
        RectLookup lookup;
        lookup.rect = rect;
        lookup.frequency = 0; // New rectangle starts with a frequency of 0
        lookup.encounter = 0; // New rectangle hasn't been encountered yet

        // If there's room in the lookup array, add the new rectangle
        if (this->index < MAX_LOOKUP_COUNTS)
        {
            LOOKUPS[this->index++] = lookup;
        }
        else
        {
            // Replace the least frequently encountered rectangle if the table is full
            if (this->min_freq_index >= 0 && this->min_freq_index < MAX_LOOKUP_COUNTS)
            {
                LOOKUPS[this->min_freq_index] = lookup;
            }
        }
    }
} myLOOKUP_MANAGER;

class OptimizationManager
{
public:
    int checkOptimization(XRectangle rect);
} optimizationManager;

// Function to check the optimization percentage based on frequency of the rectangle
int OptimizationManager::checkOptimization(XRectangle rect)
{
    if (myLOOKUP_MANAGER.match(rect))
    {
        // Based on frequency, return a reduced optimization level
        if (myLOOKUP_MANAGER.lastMatchedFrequency < 10)
            return 80; // High optimization if matched but not frequent
        else if (myLOOKUP_MANAGER.lastMatchedFrequency < 30)
            return 50; // Moderate optimization
        else
            return 20; // Low optimization for frequently matched rectangles
    }
    else
    {
        // If not matched, add the rectangle and apply full optimization
        myLOOKUP_MANAGER.push(rect);
        return 100;
    }
    return 100; // Default case, should never be reached
}

#endif
