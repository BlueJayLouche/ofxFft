#include "ofxFftAccelerate.h"

#ifdef OFX_FFT_USE_APPLE_ACCELERATE

#include <cmath>

ofxFftAccelerate::ofxFftAccelerate() : fftSetup(nullptr) {
}

void ofxFftAccelerate::setup(int signalSize, fftWindowType windowType) {
    // Call base class setup
    ofxFft::setup(signalSize, windowType);
    
    // Calculate log2 of FFT size (required by vDSP)
    log2n = log2(signalSize);
    
    // Create FFT setup
    fftSetup = vDSP_create_fftsetup(log2n, FFT_RADIX2);
    if (fftSetup == nullptr) {
        ofLogError("ofxFftAccelerate") << "Failed to create FFT setup";
        throw std::runtime_error("vDSP FFT setup failed");
    }
    
    // Create split complex buffer
    realp.resize(signalSize / 2, 0.0f);
    imagp.resize(signalSize / 2, 0.0f);
    splitComplex.resize(1);
    splitComplex[0].realp = realp.data();
    splitComplex[0].imagp = imagp.data();
    
    // Create buffer for windowed signal
    windowedSignal.resize(signalSize, 0.0f);
}

ofxFftAccelerate::~ofxFftAccelerate() {
    // Clean up FFT setup
    if (fftSetup != nullptr) {
        vDSP_destroy_fftsetup(fftSetup);
        fftSetup = nullptr;
    }
}

void ofxFftAccelerate::executeFft() {
    const vDSP_Length N = signalSize;
    const vDSP_Length N2 = N / 2;
    
    // Copy signal to windowed signal buffer
    std::copy(signal.begin(), signal.end(), windowedSignal.begin());
    
    // Apply window function
    runWindow(windowedSignal.data());
    
    // Convert real signal to split complex format
    vDSP_ctoz((DSPComplex*)windowedSignal.data(), 2, &splitComplex[0], 1, N2);
    
    // Perform forward FFT
    vDSP_fft_zrip(fftSetup, &splitComplex[0], 1, log2n, FFT_FORWARD);
    
    // vDSP forward FFT does not scale - apply scaling
    const float scale = 2.0f;
    vDSP_vsmul(splitComplex[0].realp, 1, &scale, splitComplex[0].realp, 1, N2);
    vDSP_vsmul(splitComplex[0].imagp, 1, &scale, splitComplex[0].imagp, 1, N2);
    
    // Copy results to real and imaginary vectors
    // DC component (0Hz)
    real[0] = splitComplex[0].realp[0];
    imag[0] = 0.0f; // DC has no imaginary part
    
    // Nyquist component (N/2 * sample_rate/N = sample_rate/2)
    real[N2] = splitComplex[0].imagp[0]; // Nyquist real part is stored in imagp[0]
    imag[N2] = 0.0f; // Nyquist has no imaginary part
    
    // All other components
    for (vDSP_Length i = 1; i < N2; i++) {
        real[i] = splitComplex[0].realp[i];
        imag[i] = splitComplex[0].imagp[i];
    }
    
    // Mark Cartesian representation as updated
    cartesianUpdated = true;
}

void ofxFftAccelerate::executeIfft() {
    const vDSP_Length N = signalSize;
    const vDSP_Length N2 = N / 2;
    
    // Copy real and imaginary values to split complex format
    // DC component (0Hz)
    splitComplex[0].realp[0] = real[0];
    
    // Nyquist component (N/2 * sample_rate/N = sample_rate/2)
    splitComplex[0].imagp[0] = real[N2];
    
    // All other components
    for (vDSP_Length i = 1; i < N2; i++) {
        splitComplex[0].realp[i] = real[i];
        splitComplex[0].imagp[i] = imag[i];
    }
    
    // Perform inverse FFT
    vDSP_fft_zrip(fftSetup, &splitComplex[0], 1, log2n, FFT_INVERSE);
    
    // Convert split complex format to real signal
    vDSP_ztoc(&splitComplex[0], 1, (DSPComplex*)signal.data(), 2, N2);
    
    // Apply inverse window and scaling
    runInverseWindow(signal.data());
    
    // Scale the result - vDSP inverse FFT does not scale
    const float scale = 1.0f / N;
    vDSP_vsmul(signal.data(), 1, &scale, signal.data(), 1, N);
    
    // Mark signal as updated
    signalUpdated = true;
}

#endif // OFX_FFT_USE_APPLE_ACCELERATE
