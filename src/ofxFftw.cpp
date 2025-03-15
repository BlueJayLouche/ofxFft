#include "ofxFftw.h"

#ifdef OFX_FFT_USE_FFTW

ofxFftw::ofxFftw() : fftPlan(nullptr), ifftPlan(nullptr) {
}

void ofxFftw::setup(int signalSize, fftWindowType windowType) {
    // Call base class setup first
    ofxFft::setup(signalSize, windowType);
    
    // Allocate FFTW-compatible memory buffers
    fftIn.resize(signalSize, 0.0f);
    fftOut.resize(signalSize, 0.0f);
    ifftIn.resize(signalSize, 0.0f);
    ifftOut.resize(signalSize, 0.0f);
    
    // Create FFTW plans
    // Using FFTW_DESTROY_INPUT flag because we copy our input data
    // Using FFTW_MEASURE for best performance (creates an optimal plan)
    
    // Forward FFT plan (R2HC = real to halfcomplex)
    fftPlan = fftwf_plan_r2r_1d(signalSize,
                               fftIn.data(),
                               fftOut.data(),
                               FFTW_R2HC,
                               FFTW_DESTROY_INPUT | FFTW_MEASURE);
    
    // Inverse FFT plan (HC2R = halfcomplex to real)
    ifftPlan = fftwf_plan_r2r_1d(signalSize,
                                ifftIn.data(),
                                ifftOut.data(),
                                FFTW_HC2R,
                                FFTW_DESTROY_INPUT | FFTW_MEASURE);
}

void ofxFftw::executeFft() {
    // Copy signal to FFT input buffer
    std::copy(signal.begin(), signal.end(), fftIn.begin());
    
    // Apply window function
    runWindow(fftIn.data());
    
    // Execute FFTW plan
    fftwf_execute(fftPlan);
    
    // Copy results to real and imaginary vectors
    // Halfcomplex format: r0, r1, r2, ..., rn/2, i(n/2-1), ..., i2, i1
    copyReal(fftOut.data());
    imag[0] = 0;  // DC component has no imaginary part
    
    for (int i = 1; i < binSize; i++) {
        imag[i] = fftOut[signalSize - i];
    }
    
    // Mark Cartesian representation as updated
    cartesianUpdated = true;
}

void ofxFftw::executeIfft() {
    // Copy real components directly
    std::copy(real.begin(), real.end(), ifftIn.begin());
    
    // Copy imaginary components in FFTW's halfcomplex format
    for (int i = 1; i < binSize; i++) {
        ifftIn[signalSize - i] = imag[i];
    }
    
    // Execute FFTW plan
    fftwf_execute(ifftPlan);
    
    // Apply inverse window function
    runInverseWindow(ifftOut.data());
    
    // Copy result to signal buffer
    std::copy(ifftOut.begin(), ifftOut.end(), signal.begin());
    
    // Mark signal as updated
    signalUpdated = true;
}

ofxFftw::~ofxFftw() {
    // Clean up FFTW resources
    if (fftPlan != nullptr) {
        fftwf_destroy_plan(fftPlan);
        fftPlan = nullptr;
    }
    
    if (ifftPlan != nullptr) {
        fftwf_destroy_plan(ifftPlan);
        ifftPlan = nullptr;
    }
    
    // Call fftwf_cleanup for complete cleanup
    fftwf_cleanup();
    
    // Vectors will be automatically freed by their destructors
}

#endif // OFX_FFT_USE_FFTW
