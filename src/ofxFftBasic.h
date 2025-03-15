#pragma once

/**
 * @brief Basic FFT implementation using KISS FFT
 *
 * This implementation uses the KISS FFT library which provides a good balance
 * between performance and simplicity.
 */

#include "ofxFft.h"
#include "ofTypes.h"

// Include KISS FFT headers with proper error handling
#ifndef KISS_FFT_H
extern "C" {
#include "kiss_fftr.h"
}
#endif

// Use more specific include path if needed
#ifdef USE_SPECIFIC_KISS_PATH
#include "kiss/kiss_fftr.h"
#endif

#include <vector>
#include <mutex>
#include <cstring>

/**
 * @brief Basic FFT implementation using KISS FFT
 *
 * This implementation uses the KISS FFT library which provides a good balance
 * between performance and simplicity. For optimal performance:
 *
 * 1. Use power-of-two sizes or sizes returned by getOptimalFftSize()
 * 2. Reuse FFT instances rather than creating new ones for each calculation
 * 3. Be aware that KISS FFT uses global scratch buffers that are not thread-safe
 *    by default - use the thread-safe methods provided by ofxFftBasic
 */

class ofxFftBasic : public ofxFft {
public:
    /**
     * @brief Initialize the FFT processor
     * @param signalSize Size of the input signal
     * @param windowType Window function to use
     */
    void setup(int signalSize, fftWindowType windowType) override;
    
    /**
     * @brief Clean up KISS FFT global resources
     *
     * Call this when shutting down your application to
     * free KISS FFT global scratch buffers
     */
    static void cleanupGlobalResources();
    
    /**
     * @brief Destructor
     *
     * Properly frees KISS FFT resources
     */
    ~ofxFftBasic() override;

protected:
    /**
     * @brief Execute forward FFT (time domain to frequency domain)
     *
     * Converts the time domain signal to frequency domain representation
     * in Cartesian coordinates (real/imaginary).
     */
    void executeFft() override;
    
    /**
     * @brief Execute inverse FFT (frequency domain to time domain)
     *
     * Converts the frequency domain representation in Cartesian coordinates
     * (real/imaginary) to time domain signal.
     */
    void executeIfft() override;

private:
    kiss_fftr_cfg fftCfg;    ///< KISS FFT forward configuration
    kiss_fftr_cfg ifftCfg;   ///< KISS FFT inverse configuration
    
    std::vector<float> windowedSignal;    ///< Buffer for windowed signal data
    std::vector<kiss_fft_cpx> cx_out;     ///< Buffer for complex FFT output
    std::vector<kiss_fft_cpx> cx_in;      ///< Buffer for complex FFT input
    
    // Thread-local buffers for KISS FFT operations
    std::vector<kiss_fft_cpx> localScratchBuf;
    std::mutex fftMutex; ///< For operations that can't be made thread-safe
};
