#pragma once

/**
 * @brief FFTW-based FFT implementation
 *
 * This implementation uses the FFTW library which provides optimized FFT
 * operations. It requires the OFX_FFT_USE_FFTW macro to be defined.
 */

#include "ofxFft.h"

#if defined(OFX_FFT_USE_FFTW) || defined(HAVE_FFTW3)

#include "fftw3.h"
#include <vector>

class ofxFftw : public ofxFft {
public:
    /**
     * @brief Constructor
     */
    ofxFftw();
    
    /**
     * @brief Initialize the FFT processor
     * @param signalSize Size of the input signal
     * @param windowType Window function to use
     */
    void setup(int signalSize, fftWindowType windowType) override;
    
    /**
     * @brief Destructor
     *
     * Properly frees FFTW resources
     */
    ~ofxFftw() override;

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
    std::vector<float> fftIn;     ///< Input buffer for forward FFT
    std::vector<float> fftOut;    ///< Output buffer for forward FFT
    std::vector<float> ifftIn;    ///< Input buffer for inverse FFT
    std::vector<float> ifftOut;   ///< Output buffer for inverse FFT
    
    fftwf_plan fftPlan;           ///< FFTW plan for forward FFT
    fftwf_plan ifftPlan;          ///< FFTW plan for inverse FFT
};

#endif // OFX_FFT_USE_FFTW
