// In a new header, ofxFftKiss.h
#pragma once

/**
 * @brief Direct access to KISS FFT functionality
 *
 * This header provides direct access to KISS FFT for advanced users
 * who need more control over the FFT process.
 */

#include "ofxFft.h"

extern "C" {
#include "kiss/kiss_fft.h"
#include "kiss/kiss_fftr.h"
}

namespace ofxFftKiss {
    /**
     * @brief Get the optimal FFT size for performance
     *
     * KISS FFT performs best with sizes that factor nicely into
     * products of 2, 3, and 5.
     *
     * @param minSize Minimum FFT size needed
     * @return int Optimal FFT size (>= minSize)
     */
    inline int getOptimalFftSize(int minSize) {
        return kiss_fft_next_fast_size(minSize);
    }
    
    /**
     * @brief Get direct access to KISS FFT configuration
     *
     * For advanced usage only - users should normally use ofxFft classes
     *
     * @param fft Pointer to ofxFftBasic instance
     * @return kiss_fftr_cfg KISS FFT configuration
     */
    kiss_fftr_cfg getKissConfig(ofxFftBasic* fft);
}
