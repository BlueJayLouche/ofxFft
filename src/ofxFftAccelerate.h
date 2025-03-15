#pragma once

/**
 * @brief Accelerate framework FFT implementation for macOS and iOS
 *
 * This implementation uses Apple's Accelerate framework which provides
 * highly optimized FFT operations for both Intel and Apple Silicon processors.
 */

#include "ofxFft.h"

#if defined(OFX_FFT_USE_APPLE_ACCELERATE)

#include <Accelerate/Accelerate.h>
#include <vector>

class ofxFftAccelerate : public ofxFft {
public:
    /**
     * @brief Constructor
     */
    ofxFftAccelerate();
    
    /**
     * @brief Initialize the FFT processor
     * @param signalSize Size of the input signal
     * @param windowType Window function to use
     */
    void setup(int signalSize, fftWindowType windowType) override;
    
    /**
     * @brief Destructor
     *
     * Properly frees Accelerate framework resources
     */
    ~ofxFftAccelerate() override;

protected:
    /**
     * @brief Execute forward FFT (time domain to frequency domain)
     *
     * Converts the time domain signal to frequency domain representation
     * in Cartesian coordinates (real/imaginary) using vDSP.
     */
    void executeFft() override;
    
    /**
     * @brief Execute inverse FFT (frequency domain to time domain)
     *
     * Converts the frequency domain representation in Cartesian coordinates
     * (real/imaginary) to time domain signal using vDSP.
     */
    void executeIfft() override;

private:
    vDSP_Length log2n;                ///< Log base 2 of FFT size
    FFTSetup fftSetup;                ///< Apple FFT setup object
    
    std::vector<float> realp;         ///< Real part buffer for Apple FFT
    std::vector<float> imagp;         ///< Imaginary part buffer for Apple FFT
    std::vector<DSPSplitComplex> splitComplex; ///< Split complex buffer for vDSP
    std::vector<float> windowedSignal; ///< Windowed signal buffer
};

#endif // OFX_FFT_USE_APPLE_ACCELERATE
