#pragma once

#include "ofMain.h"
#include <vector>
#include <memory>
#include <complex>
#include <functional>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    // Intel architecture - use AVX/SSE
    #include <immintrin.h>
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM64)
    // ARM architecture (Raspberry Pi, Apple Silicon)
    #if defined(__APPLE__)
        // Apple Silicon - use Apple's Accelerate framework for vector operations
        #include <Accelerate/Accelerate.h>
    #else
        // Other ARM (Raspberry Pi) - use NEON if available
        #if defined(__ARM_NEON) || defined(__ARM_NEON__)
            #include <arm_neon.h>
        #endif
    #endif
#endif

// Forward declarations
class ofxFftBasic;
class ofxFftAccelerate;
class ofxFftNeon;

/**
 * @brief Window types for FFT analysis
 *
 * Different window functions provide different trade-offs between
 * frequency resolution and spectral leakage.
 */
enum fftWindowType {
    OF_FFT_WINDOW_RECTANGULAR, ///< No windowing
    OF_FFT_WINDOW_BARTLETT,    ///< Triangular window (first-order)
    OF_FFT_WINDOW_HANN,        ///< Hann window (good for audio)
    OF_FFT_WINDOW_HAMMING,     ///< Hamming window (modified Hann window)
    OF_FFT_WINDOW_SINE         ///< Sine window
};

/**
 * @brief FFT implementation backend
 */
enum fftImplementation {
    OF_FFT_BASIC,  ///< KISS FFT implementation
    OF_FFT_FFTW    ///< FFTW implementation (faster, if available)
};

enum fftOptimizationFlags {
    OF_FFT_DEFAULT = 0,
    OF_FFT_FAST_SIZE = 1,  // Use kiss_fft_next_fast_size for optimal performance
    OF_FFT_OPENMP = 2      // Enable OpenMP parallelization if available
};

/**
 * @brief Fast Fourier Transform (FFT) base class
 *
 * This class provides methods to perform FFT analysis on signal data,
 * converting between time domain and frequency domain representations.
 *
 * Features:
 * - Convert between time domain (signal) and frequency domain (spectrum)
 * - Provide both Cartesian (real/imaginary) and polar (amplitude/phase) representations
 * - Support different window functions to shape the input signal
 * - Support different FFT implementations (KISS FFT and FFTW)
 */
class ofxFft {
public:
    /**
     * @brief Create an FFT processor instance
     *
     * Factory method to create an appropriate FFT processor based on the
     * requested implementation.
     *
     * @param signalSize Size of the input signal (should be a power of 2)
     * @param windowType Window function to apply to the input signal
     * @param implementation FFT implementation backend to use
     * @return std::shared_ptr<ofxFft> Shared pointer to the FFT processor
     */
    static std::shared_ptr<ofxFft> create(
        int signalSize = 512,
        fftWindowType windowType = OF_FFT_WINDOW_HAMMING,
        fftImplementation implementation = OF_FFT_BASIC);
    
    /**
     * @brief Destructor
     */
    virtual ~ofxFft();

    /**
     * @brief Set input signal from a vector
     * @param signal Vector containing the input signal
     */
    void setSignal(const std::vector<float>& signal);
    
    /**
     * @brief Set input signal from a raw pointer (legacy support)
     * @param signal Pointer to the input signal array
     */
    void setSignal(const float* signal);
    
    /**
     * @brief Set the Cartesian components (real/imaginary)
     * @param real Real component values
     * @param imag Imaginary component values (optional, defaults to zeros)
     */
    void setCartesian(float* real, float* imag = nullptr);
    
    /**
     * @brief Set the polar components (amplitude/phase)
     * @param amplitude Amplitude values
     * @param phase Phase values (optional, defaults to zeros)
     */
    void setPolar(float* amplitude, float* phase = nullptr);

    /**
     * @brief Get the size of the input signal
     * @return int Signal size
     */
    int getSignalSize() const;
    
    /**
     * @brief Get the input signal data
     * @return float* Pointer to the signal data
     */
    float* getSignal();
    
    /**
     * @brief Get a constant reference to the signal vector
     * @return const std::vector<float>& Reference to the signal vector
     */
    const std::vector<float>& getSignalVector() const;
    
    /**
     * @brief Clamp signal values to range [-1, 1]
     */
    void clampSignal();

    /**
     * @brief Get the size of the frequency bins
     * @return int Number of frequency bins
     */
    int getBinSize() const;
    
    /**
     * @brief Get the real component of the frequency spectrum
     * @return float* Pointer to the real component data
     */
    float* getReal();
    
    /**
     * @brief Get a constant reference to the real component vector
     * @return const std::vector<float>& Reference to the real component vector
     */
    const std::vector<float>& getRealVector() const;
    
    /**
     * @brief Get the imaginary component of the frequency spectrum
     * @return float* Pointer to the imaginary component data
     */
    float* getImaginary();
    
    /**
     * @brief Get a constant reference to the imaginary component vector
     * @return const std::vector<float>& Reference to the imaginary component vector
     */
    const std::vector<float>& getImaginaryVector() const;
    
    /**
     * @brief Get the amplitude component of the frequency spectrum
     * @return float* Pointer to the amplitude component data
     */
    float* getAmplitude();
    
    /**
     * @brief Get a constant reference to the amplitude component vector
     * @return const std::vector<float>& Reference to the amplitude component vector
     */
    const std::vector<float>& getAmplitudeVector() const;
    
    // For backward compatibility
    std::vector<float> getAmplitudeVector() {
        float* amplitudePtr = getAmplitude();
        return std::vector<float>(amplitudePtr, amplitudePtr + getBinSize());
    }
    
    /**
     * @brief Get the phase component of the frequency spectrum
     * @return float* Pointer to the phase component data
     */
    float* getPhase();
    
    /**
     * @brief Get a constant reference to the phase component vector
     * @return const std::vector<float>& Reference to the phase component vector
     */
    const std::vector<float>& getPhaseVector() const;

    /**
     * @brief Get the amplitude at a specific bin, with interpolation
     * @param bin Bin index (can be fractional for interpolation)
     * @return float Interpolated amplitude value
     */
    float getAmplitudeAtBin(float bin);
    
    /**
     * @brief Convert a frequency to the corresponding bin index
     * @param frequency Frequency in Hz
     * @param sampleRate Sample rate in Hz
     * @return float Corresponding bin index
     */
    float getBinFromFrequency(float frequency, float sampleRate = 44100);
    
    /**
     * @brief Get the amplitude at a specific frequency
     * @param frequency Frequency in Hz
     * @param sampleRate Sample rate in Hz
     * @return float Amplitude at the specified frequency
     */
    float getAmplitudeAtFrequency(float frequency, float sampleRate = 44100);

protected:
    /**
     * @brief Initialize the FFT processor
     * @param signalSize Size of the input signal
     * @param windowType Window function to use
     */
    virtual void setup(int signalSize, fftWindowType windowType);
    
    /**
     * @brief Execute the forward FFT (time domain to frequency domain)
     *
     * This is a pure virtual function that must be implemented by derived classes.
     */
    virtual void executeFft() = 0;
    
    /**
     * @brief Execute the inverse FFT (frequency domain to time domain)
     *
     * This is a pure virtual function that must be implemented by derived classes.
     */
    virtual void executeIfft() = 0;

    /**
     * @brief Clear all data buffers
     */
    void clear();

    // Time domain data and methods
    fftWindowType windowType;        ///< Type of window function
    float windowSum;                 ///< Sum of window function values (for normalization)
    std::vector<float> window;       ///< Window function values
    std::vector<float> inverseWindow; ///< Inverse window function values

    /**
     * @brief Set the window function type
     * @param windowType Type of window function to use
     */
    void setWindowType(fftWindowType windowType);

    /**
     * @brief Apply the window function to a signal
     * @param signal Pointer to the signal to be windowed
     */
    inline void runWindow(float* signal) {
        if (windowType != OF_FFT_WINDOW_RECTANGULAR) {
            for (int i = 0; i < signalSize; i++) {
                signal[i] *= window[i];
            }
        }
    }

    /**
     * @brief Apply the inverse window function to a signal
     * @param signal Pointer to the signal to be inversely windowed
     */
    inline void runInverseWindow(float* signal) {
        if (windowType != OF_FFT_WINDOW_RECTANGULAR) {
            for (int i = 0; i < signalSize; i++) {
                signal[i] *= inverseWindow[i];
            }
        }
    }

    std::vector<float> signal;       ///< Time domain signal data
    bool signalUpdated;              ///< Flag indicating if signal has been updated
    bool signalNormalized;           ///< Flag indicating if signal has been normalized
    
    /**
     * @brief Prepare the signal for processing
     */
    void prepareSignal();
    
    /**
     * @brief Update the signal from frequency data
     */
    void updateSignal();
    
    /**
     * @brief Normalize the signal
     */
    void normalizeSignal();
    
    /**
     * @brief Copy signal data from a raw pointer
     * @param signal Pointer to the signal data to copy
     */
    void copySignal(const float* signal);

    // Frequency domain data and methods
    int signalSize;                  ///< Size of the input signal
    int binSize;                     ///< Number of frequency bins

    std::vector<float> real;         ///< Real component of frequency data
    std::vector<float> imag;         ///< Imaginary component of frequency data
    bool cartesianUpdated;           ///< Flag indicating if Cartesian data has been updated
    bool cartesianNormalized;        ///< Flag indicating if Cartesian data has been normalized
    
    /**
     * @brief Prepare Cartesian data for processing
     */
    void prepareCartesian();
    
    /**
     * @brief Update Cartesian data from polar data
     */
    void updateCartesian();
    
    /**
     * @brief Normalize Cartesian data
     */
    void normalizeCartesian();
    
    /**
     * @brief Copy real component data from a raw pointer
     * @param real Pointer to the real component data to copy
     */
    void copyReal(float* real);
    
    /**
     * @brief Copy imaginary component data from a raw pointer
     * @param imag Pointer to the imaginary component data to copy
     */
    void copyImaginary(float* imag);

    std::vector<float> amplitude;    ///< Amplitude component of frequency data
    std::vector<float> phase;        ///< Phase component of frequency data
    bool polarUpdated;               ///< Flag indicating if polar data has been updated
    bool polarNormalized;            ///< Flag indicating if polar data has been normalized
    
    /**
     * @brief Prepare polar data for processing
     */
    void preparePolar();
    
    /**
     * @brief Update polar data from Cartesian data
     */
    void updatePolar();
    
    /**
     * @brief Normalize polar data
     */
    void normalizePolar();
    
    /**
     * @brief Copy amplitude component data from a raw pointer
     * @param amplitude Pointer to the amplitude component data to copy
     */
    void copyAmplitude(float* amplitude);
    
    /**
     * @brief Copy phase component data from a raw pointer
     * @param phase Pointer to the phase component data to copy
     */
    void copyPhase(float* phase);

    /**
     * @brief Clear update flags
     */
    void clearUpdates();

    /**
     * @brief Convert Cartesian coordinates to amplitude
     * @param x Real component
     * @param y Imaginary component
     * @return float Amplitude value
     */
    inline float cartesianToAmplitude(float x, float y) {
        return sqrtf(x * x + y * y);
    }

    /**
     * @brief Convert Cartesian coordinates to phase
     * @param x Real component
     * @param y Imaginary component
     * @return float Phase value
     */
    inline float cartesianToPhase(float x, float y) {
        return atan2f(y, x);
    }
};

// This section needs to be at the end of the file
// after all the forward declarations
#if defined(OFX_FFT_USE_ACCELERATE)
    #include "ofxFftAccelerate.h"
    typedef ofxFftAccelerate ofxFftPlatformOptimized;
#elif defined(OFX_FFT_USE_NEON)
    #include "ofxFftNeon.h"
    typedef ofxFftNeon ofxFftPlatformOptimized;
#else
    // Default implementation will be defined after including ofxFftBasic.h
    // in ofxFft.cpp
#endif
