#include "ofxEasyFft.h"
#include <algorithm>

ofxEasyFft::ofxEasyFft() : useNormalization(true) {
}

ofxEasyFft::~ofxEasyFft() {
    // Close the audio stream properly
    stream.close();
}

void ofxEasyFft::setup(int bufferSize, fftWindowType windowType, fftImplementation implementation, int audioBufferSize, int audioSampleRate) {
    // Validate buffer size
    if (bufferSize < audioBufferSize) {
        ofLogWarning("ofxEasyFft") << "bufferSize (" << bufferSize
                                   << ") less than audioBufferSize ("
                                   << audioBufferSize
                                   << "), using " << audioBufferSize;
        bufferSize = audioBufferSize;
    }
    
    // Create FFT instance
    fft = ofxFft::create(bufferSize, windowType, implementation);
    
    // Allocate buffers
    bins.resize(fft->getBinSize(), 0.0f);
    audioFront.resize(bufferSize, 0.0f);
    audioMiddle.resize(bufferSize, 0.0f);
    audioBack.resize(bufferSize, 0.0f);
    audioRaw.resize(bufferSize, 0.0f);
    
    // Setup audio input
    ofSoundStreamSettings settings;
    settings.numInputChannels = 1;
    settings.numOutputChannels = 0;
    settings.sampleRate = audioSampleRate;
    settings.bufferSize = audioBufferSize;
    settings.numBuffers = 2;
    settings.setInListener(this);
    
    // Get available audio devices
    auto devices = stream.getDeviceList();
    
    // Platform-specific device selection
#if defined(__arm__) || defined(__aarch64__)
    // For Raspberry Pi and similar ARM devices, try to find specific devices
    bool foundDevice = false;
    for (const auto& device : devices) {
        if (device.isDefaultInput) {
            ofLogNotice("ofxEasyFft") << "Using default input device: " << device.name;
            settings.setInDevice(device);
            foundDevice = true;
            break;
        }
    }
    
    if (!foundDevice && !devices.empty()) {
        ofLogNotice("ofxEasyFft") << "Default input device not found, using first available: " << devices[0].name;
        settings.setInDevice(devices[0]);
    }
#endif
    
    // Setup stream with the settings
    stream.setup(settings);
}

void ofxEasyFft::setUseNormalization(bool useNormalization) {
    this->useNormalization.store(useNormalization, std::memory_order_relaxed);
}

void ofxEasyFft::update() {
    // Thread-safe copy of audio buffer
    {
        std::lock_guard<std::mutex> lock(soundMutex);
        audioFront = audioMiddle;
    }
    
    // Perform FFT analysis
    fft->setSignal(audioFront);
    
    // Copy amplitude data to bins
    const auto& amplitude = fft->getAmplitudeVector();
    std::copy(amplitude.begin(), amplitude.end(), bins.begin());
    
    // Normalize if needed
    if (useNormalization.load(std::memory_order_relaxed)) {
        normalize(bins);
    }
}

const std::vector<float>& ofxEasyFft::getAudio() const {
    return audioFront;
}

const std::vector<float>& ofxEasyFft::getBins() const {
    return bins;
}

void ofxEasyFft::audioReceived(float* input, int bufferSize, int nChannels) {
    // Process incoming audio data
    
    // Circular buffer approach: shift old data and add new data
    if (audioRaw.size() > bufferSize) {
        // Shift old data
        std::copy(audioRaw.begin() + bufferSize, audioRaw.end(), audioRaw.begin());
    }
    
    // Copy new data to end of buffer
    std::copy(input, input + bufferSize, audioRaw.end() - bufferSize);
    
    // Copy full buffer to audioBack
    std::copy(audioRaw.begin(), audioRaw.end(), audioBack.begin());
    
    // Normalize if needed
    if (useNormalization.load(std::memory_order_relaxed)) {
        normalize(audioBack);
    }
    
    // Thread-safe handoff to audioMiddle
    {
        std::lock_guard<std::mutex> lock(soundMutex);
        audioMiddle = audioBack;
    }
}

void ofxEasyFft::normalize(std::vector<float>& data) {
    if (data.empty()) return;
    
#if defined(__x86_64__) || defined(_M_X64) // 64-bit Intel
    // Use AVX2 if available for Intel architectures
    #if defined(__AVX2__)
    // AVX2 implementation for 8 floats at once
    const int numElements = data.size();
    const int numFullIterations = numElements / 8;
    
    // Find maximum using AVX
    __m256 maxVal = _mm256_setzero_ps();
    for (int i = 0; i < numFullIterations; i++) {
        __m256 values = _mm256_loadu_ps(&data[i * 8]);
        __m256 absValues = _mm256_andnot_ps(_mm256_set1_ps(-0.0f), values); // Absolute value trick
        maxVal = _mm256_max_ps(maxVal, absValues);
    }
    
    // Find max of the 8 floats in the AVX register
    float maxArray[8];
    _mm256_storeu_ps(maxArray, maxVal);
    float maxFloat = maxArray[0];
    for (int i = 1; i < 8; i++) {
        maxFloat = std::max(maxFloat, maxArray[i]);
    }
    
    // Check remaining elements
    for (int i = numFullIterations * 8; i < numElements; i++) {
        maxFloat = std::max(maxFloat, std::abs(data[i]));
    }
    
    // Normalize only if we have a non-zero maximum
    if (maxFloat > 0.0f) {
        __m256 invMax = _mm256_set1_ps(1.0f / maxFloat);
        for (int i = 0; i < numFullIterations; i++) {
            __m256 values = _mm256_loadu_ps(&data[i * 8]);
            __m256 normalized = _mm256_mul_ps(values, invMax);
            _mm256_storeu_ps(&data[i * 8], normalized);
        }
        
        // Normalize remaining elements
        for (int i = numFullIterations * 8; i < numElements; i++) {
            data[i] /= maxFloat;
        }
    }
    #else
    // Standard implementation for older Intel processors
    float maxValue = 0.0f;
    for (float value : data) {
        maxValue = std::max(maxValue, std::abs(value));
    }
    
    if (maxValue > 0.0f) {
        for (float& value : data) {
            value /= maxValue;
        }
    }
    #endif
#elif defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))
    // Apple Silicon - use Accelerate framework
    const int numElements = data.size();
    float maxValue = 0.0f;
    
    // Find maximum using vDSP
    vDSP_maxmgv(data.data(), 1, &maxValue, numElements);
    
    // Normalize using vDSP if max > 0
    if (maxValue > 0.0f) {
        float scale = 1.0f / maxValue;
        vDSP_vsmul(data.data(), 1, &scale, data.data(), 1, numElements);
    }
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    // ARM NEON optimization for Raspberry Pi etc.
    const int numElements = data.size();
    const int numFullIterations = numElements / 4;
    
    // Find maximum using NEON
    float32x4_t maxVal = vdupq_n_f32(0);
    for (int i = 0; i < numFullIterations; i++) {
        float32x4_t values = vld1q_f32(&data[i * 4]);
        // Get absolute values
        float32x4_t absValues = vabsq_f32(values);
        maxVal = vmaxq_f32(maxVal, absValues);
    }
    
    // Find max of the 4 floats in the NEON register
    float maxArray[4];
    vst1q_f32(maxArray, maxVal);
    float maxFloat = maxArray[0];
    for (int i = 1; i < 4; i++) {
        maxFloat = std::max(maxFloat, maxArray[i]);
    }
    
    // Check remaining elements
    for (int i = numFullIterations * 4; i < numElements; i++) {
        maxFloat = std::max(maxFloat, std::abs(data[i]));
    }
    
    // Normalize only if we have a non-zero maximum
    if (maxFloat > 0.0f) {
        float32x4_t invMax = vdupq_n_f32(1.0f / maxFloat);
        for (int i = 0; i < numFullIterations; i++) {
            float32x4_t values = vld1q_f32(&data[i * 4]);
            float32x4_t normalized = vmulq_f32(values, invMax);
            vst1q_f32(&data[i * 4], normalized);
        }
        
        // Normalize remaining elements
        for (int i = numFullIterations * 4; i < numElements; i++) {
            data[i] /= maxFloat;
        }
    }
#else
    // Fallback implementation for other platforms
    float maxValue = 0.0f;
    for (float value : data) {
        maxValue = std::max(maxValue, std::abs(value));
    }
    
    if (maxValue > 0.0f) {
        for (float& value : data) {
            value /= maxValue;
        }
    }
#endif
}

std::shared_ptr<ofxFft> ofxEasyFft::getFft() const {
    return fft;
}
