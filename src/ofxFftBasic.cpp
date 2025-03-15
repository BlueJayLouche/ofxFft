#include "ofxFftBasic.h"

void ofxFftBasic::setup(int signalSize, fftWindowType windowType) {
    // Call base class setup first
    ofxFft::setup(signalSize, windowType);
    
    // Initialize KISS FFT with better error handling
    fftCfg = kiss_fftr_alloc(signalSize, 0, nullptr, nullptr);
    if (!fftCfg) {
        ofLogError("ofxFftBasic") << "Failed to allocate forward FFT configuration";
        throw std::runtime_error("KISS FFT allocation failed");
    }
    
    ifftCfg = kiss_fftr_alloc(signalSize, 1, nullptr, nullptr);
    if (!ifftCfg) {
        // Clean up forward config to avoid memory leak
        kiss_fftr_free(fftCfg);
        fftCfg = nullptr;
        ofLogError("ofxFftBasic") << "Failed to allocate inverse FFT configuration";
        throw std::runtime_error("KISS FFT allocation failed");
    }
    
    // Allocate buffers
    windowedSignal.resize(signalSize, 0.0f);
    cx_out.resize(binSize);
    cx_in.resize(binSize);
    
    // Ensure complex buffers are initialized to zero
    std::memset(cx_out.data(), 0, sizeof(kiss_fft_cpx) * binSize);
    std::memset(cx_in.data(), 0, sizeof(kiss_fft_cpx) * binSize);
}

void ofxFftBasic::cleanupGlobalResources() {
    kiss_fft_cleanup();
}

ofxFftBasic::~ofxFftBasic() {
    // Free KISS FFT resources
    if (fftCfg) {
        kiss_fftr_free(fftCfg);
        fftCfg = nullptr;
    }
    
    if (ifftCfg) {
        kiss_fftr_free(ifftCfg);
        ifftCfg = nullptr;
    }
    
    // Vectors will be automatically freed by their destructors
}

void ofxFftBasic::executeFft() {
    // Copy signal to windowedSignal buffer
    std::copy(signal.begin(), signal.end(), windowedSignal.begin());
    
    // Apply window function
    runWindow(windowedSignal.data());
    
    // Use optimized real-to-complex FFT (more efficient than complex FFT)
    kiss_fftr(fftCfg, windowedSignal.data(), cx_out.data());
    
    // Copy results to real and imaginary vectors
    for (int i = 0; i < binSize; i++) {
        real[i] = cx_out[i].r;
        imag[i] = cx_out[i].i;
    }
    
    // Mark Cartesian representation as updated
    cartesianUpdated = true;
}

void ofxFftBasic::executeIfft() {
    // Copy real and imaginary data to complex input buffer
    for (int i = 0; i < binSize; i++) {
        cx_in[i].r = real[i];
        cx_in[i].i = imag[i];
    }
    
    // Perform inverse FFT
    kiss_fftri(ifftCfg, cx_in.data(), signal.data());
    
    // Apply inverse window
    runInverseWindow(signal.data());
    
    // Mark signal as updated
    signalUpdated = true;
}

int getOptimalFftSize(int requestedSize) {
    return kiss_fft_next_fast_size(requestedSize);
}
