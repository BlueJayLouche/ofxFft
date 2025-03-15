#include "ofxFft.h"
#include "ofxFftBasic.h"

// Now that we've included ofxFftBasic.h, we can safely define the default implementation
#if !defined(OFX_FFT_USE_ACCELERATE) && !defined(OFX_FFT_USE_NEON)
    typedef ofxFftBasic ofxFftPlatformOptimized;
#endif

#ifdef OFX_FFT_USE_FFTW
    #include "ofxFftw.h"
#endif

#ifdef OFX_FFT_USE_NEON
// Optimized window function application using NEON intrinsics
inline void runWindowNeon(float* signal, const float* window, int signalSize) {
    int i = 0;
    // Process 4 elements at a time with NEON
    for (; i <= signalSize - 4; i += 4) {
        float32x4_t signalVec = vld1q_f32(&signal[i]);
        float32x4_t windowVec = vld1q_f32(&window[i]);
        float32x4_t result = vmulq_f32(signalVec, windowVec);
        vst1q_f32(&signal[i], result);
    }
    // Handle remaining elements
    for (; i < signalSize; i++) {
        signal[i] *= window[i];
    }
}
#endif

std::shared_ptr<ofxFft> ofxFft::create(int signalSize, fftWindowType windowType, fftImplementation implementation) {
    std::shared_ptr<ofxFft> fft;
    
    if (implementation == OF_FFT_BASIC) {
        fft = std::make_shared<ofxFftBasic>();
    } else if (implementation == OF_FFT_FFTW) {
        #ifdef OFX_FFT_USE_FFTW
            fft = std::make_shared<ofxFftw>();
        #else
            ofLogWarning() << "FFTW support requires defining OFX_FFT_USE_FFTW in your C++ flags, falling back to KISS";
            fft = std::make_shared<ofxFftBasic>();
        #endif
    }
    
    if (fft) {
        fft->setup(signalSize, windowType);
    }
    
    return fft;
}

void ofxFft::setup(int signalSize, fftWindowType windowType) {
    this->signalSize = signalSize;
    this->binSize = (signalSize / 2) + 1;

    signalNormalized = true;
    signal.resize(signalSize, 0.0f);

    cartesianUpdated = true;
    cartesianNormalized = true;
    real.resize(binSize, 0.0f);
    imag.resize(binSize, 0.0f);

    polarUpdated = true;
    polarNormalized = true;
    amplitude.resize(binSize, 0.0f);
    phase.resize(binSize, 0.0f);

    window.resize(signalSize, 0.0f);
    inverseWindow.resize(signalSize, 0.0f);
    setWindowType(windowType);
    
    clear();
}

int ofxFft::getBinSize() const {
    return binSize;
}

int ofxFft::getSignalSize() const {
    return signalSize;
}

void ofxFft::setWindowType(fftWindowType windowType) {
    this->windowType = windowType;
    
    // Calculate window function values
    if (windowType == OF_FFT_WINDOW_RECTANGULAR) {
        std::fill(window.begin(), window.end(), 1.0f);
    } else if (windowType == OF_FFT_WINDOW_BARTLETT) {
        int half = signalSize / 2;
        for (int i = 0; i < half; i++) {
            window[i] = ((float)i / half);
            window[i + half] = (1 - ((float)i / half));
        }
    } else if (windowType == OF_FFT_WINDOW_HANN) {
        for (int i = 0; i < signalSize; i++) {
            window[i] = 0.5f * (1.0f - cos((TWO_PI * i) / (signalSize - 1)));
        }
    } else if (windowType == OF_FFT_WINDOW_HAMMING) {
        for (int i = 0; i < signalSize; i++) {
            window[i] = 0.54f - 0.46f * cos((TWO_PI * i) / (signalSize - 1));
        }
    } else if (windowType == OF_FFT_WINDOW_SINE) {
        for (int i = 0; i < signalSize; i++) {
            window[i] = sin((PI * i) / (signalSize - 1));
        }
    }

    // Calculate window sum for normalization
    windowSum = 0.0f;
    for (int i = 0; i < signalSize; i++) {
        windowSum += window[i];
    }

    // Calculate inverse window values
    for (int i = 0; i < signalSize; i++) {
        inverseWindow[i] = 1.0f / window[i];
    }
}

ofxFft::~ofxFft() {
    // No manual memory deallocation needed with std::vector
}

void ofxFft::clear() {
    std::fill(signal.begin(), signal.end(), 0.0f);
    std::fill(real.begin(), real.end(), 0.0f);
    std::fill(imag.begin(), imag.end(), 0.0f);
    std::fill(amplitude.begin(), amplitude.end(), 0.0f);
    std::fill(phase.begin(), phase.end(), 0.0f);
}

void ofxFft::copySignal(const float* signal) {
    std::copy(signal, signal + signalSize, this->signal.begin());
}

void ofxFft::copyReal(float* real) {
    std::copy(real, real + binSize, this->real.begin());
}

void ofxFft::copyImaginary(float* imag) {
    if (imag == nullptr) {
        std::fill(this->imag.begin(), this->imag.end(), 0.0f);
    } else {
        std::copy(imag, imag + binSize, this->imag.begin());
    }
}

void ofxFft::copyAmplitude(float* amplitude) {
    std::copy(amplitude, amplitude + binSize, this->amplitude.begin());
}

void ofxFft::copyPhase(float* phase) {
    if (phase == nullptr) {
        std::fill(this->phase.begin(), this->phase.end(), 0.0f);
    } else {
        std::copy(phase, phase + binSize, this->phase.begin());
    }
}

void ofxFft::prepareSignal() {
    if (!signalUpdated) {
        updateSignal();
    }
    if (!signalNormalized) {
        normalizeSignal();
    }
}

void ofxFft::updateSignal() {
    prepareCartesian();
    executeIfft();
    signalUpdated = true;
    signalNormalized = false;
}

void ofxFft::normalizeSignal() {
    float normalizer = (float)windowSum / (2 * signalSize);
    for (int i = 0; i < signalSize; i++) {
        signal[i] *= normalizer;
    }
    signalNormalized = true;
}

float* ofxFft::getSignal() {
    prepareSignal();
    return signal.data();
}

const std::vector<float>& ofxFft::getSignalVector() const {
    return signal;
}

void ofxFft::clampSignal() {
    prepareSignal();
    for (int i = 0; i < signalSize; i++) {
        if (signal[i] > 1) {
            signal[i] = 1;
        } else if (signal[i] < -1) {
            signal[i] = -1;
        }
    }
}

void ofxFft::prepareCartesian() {
    if (!cartesianUpdated) {
        if (!polarUpdated) {
            executeFft();
        } else {
            updateCartesian();
        }
    }
    if (!cartesianNormalized) {
        normalizeCartesian();
    }
}

float* ofxFft::getReal() {
    prepareCartesian();
    return real.data();
}

const std::vector<float>& ofxFft::getRealVector() const {
    return real;
}

float* ofxFft::getImaginary() {
    prepareCartesian();
    return imag.data();
}

const std::vector<float>& ofxFft::getImaginaryVector() const {
    return imag;
}

void ofxFft::preparePolar() {
    if (!polarUpdated) {
        updatePolar();
    }
    if (!polarNormalized) {
        normalizePolar();
    }
}

float* ofxFft::getAmplitude() {
    preparePolar();
    return amplitude.data();
}

const std::vector<float>& ofxFft::getAmplitudeVector() const {
    return amplitude;
}

float* ofxFft::getPhase() {
    preparePolar();
    return phase.data();
}

const std::vector<float>& ofxFft::getPhaseVector() const {
    return phase;
}

float ofxFft::getAmplitudeAtBin(float bin) {
    float* amplitude = getAmplitude();
    int lowBin = ofClamp(floorf(bin), 0, binSize - 1);
    int highBin = ofClamp(ceilf(bin), 0, binSize - 1);
    return ofMap(bin, lowBin, highBin, amplitude[lowBin], amplitude[highBin]);
}

float ofxFft::getBinFromFrequency(float frequency, float sampleRate) {
    return frequency * binSize / (sampleRate / 2);
}

float ofxFft::getAmplitudeAtFrequency(float frequency, float sampleRate) {
    return getAmplitudeAtBin(getBinFromFrequency(frequency, sampleRate));
}

void ofxFft::updateCartesian() {
    for (int i = 0; i < binSize; i++) {
        real[i] = cosf(phase[i]) * amplitude[i];
        imag[i] = sinf(phase[i]) * amplitude[i];
    }
    cartesianUpdated = true;
    cartesianNormalized = polarNormalized;
}

void ofxFft::normalizeCartesian() {
    float normalizer = 2.0f / windowSum;
    for (int i = 0; i < binSize; i++) {
        real[i] *= normalizer;
        imag[i] *= normalizer;
    }
    cartesianNormalized = true;
}

void ofxFft::updatePolar() {
    prepareCartesian();
    for (int i = 0; i < binSize; i++) {
        amplitude[i] = cartesianToAmplitude(real[i], imag[i]);
        phase[i] = cartesianToPhase(real[i], imag[i]);
    }
    polarUpdated = true;
    polarNormalized = cartesianNormalized;
}

void ofxFft::normalizePolar() {
    float normalizer = 2.0f / windowSum;
    for (int i = 0; i < binSize; i++) {
        amplitude[i] *= normalizer;
    }
    polarNormalized = true;
}

void ofxFft::clearUpdates() {
    cartesianUpdated = false;
    polarUpdated = false;
    cartesianNormalized = false;
    polarNormalized = false;
    signalUpdated = false;
    signalNormalized = false;
}

void ofxFft::setSignal(const std::vector<float>& signal) {
    setSignal(signal.data());
}

void ofxFft::setSignal(const float* signal) {
    clearUpdates();
    copySignal(signal);
    signalUpdated = true;
    signalNormalized = true;
}

void ofxFft::setCartesian(float* real, float* imag) {
    clearUpdates();
    copyReal(real);
    copyImaginary(imag);
    cartesianUpdated = true;
    cartesianNormalized = true;
}

void ofxFft::setPolar(float* amplitude, float* phase) {
    clearUpdates();
    copyAmplitude(amplitude);
    copyPhase(phase);
    polarUpdated = true;
    polarNormalized = true;
}
