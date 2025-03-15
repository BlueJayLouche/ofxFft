#pragma once

/**
 * @brief Easy-to-use real-time audio FFT analysis
 *
 * This class provides a simplified interface for capturing audio
 * and performing FFT analysis in real-time.
 */

#include "ofMain.h"
#include "ofxFft.h"
#include <mutex>
#include <atomic>

class ofxEasyFft : public ofBaseSoundInput {
public:
    /**
     * @brief Constructor
     */
    ofxEasyFft();
    
    /**
     * @brief Destructor
     */
    ~ofxEasyFft();
    
    /**
     * @brief Initialize the FFT processor
     *
     * @param bufferSize Size of the FFT buffer (should be a power of 2)
     * @param windowType Window function to use
     * @param implementation FFT implementation to use
     * @param audioBufferSize Size of the audio buffer
     * @param audioSampleRate Audio sample rate in Hz
     */
    void setup(int bufferSize = 512,
               fftWindowType windowType = OF_FFT_WINDOW_HAMMING,
               fftImplementation implementation = OF_FFT_BASIC,
               int audioBufferSize = 256,
               int audioSampleRate = 44100);
    
    /**
     * @brief Set whether to normalize FFT output
     *
     * When enabled, FFT output will be normalized to the range [0, 1]
     *
     * @param useNormalization Whether to enable normalization
     */
    void setUseNormalization(bool useNormalization);
    
    /**
     * @brief Update FFT analysis
     *
     * Should be called regularly (typically in the update() method of an app)
     */
    void update();
    
    /**
     * @brief Get the raw audio buffer
     * @return Reference to the audio buffer vector
     */
    const std::vector<float>& getAudio() const;
    
    /**
     * @brief Get the FFT frequency bins
     * @return Reference to the frequency bins vector
     */
    const std::vector<float>& getBins() const;
    
    /**
     * @brief Audio input callback
     *
     * Receives audio data from the sound system
     *
     * @param input Pointer to audio input buffer
     * @param bufferSize Size of the audio buffer
     * @param nChannels Number of audio channels
     */
    void audioReceived(float* input, int bufferSize, int nChannels) override;
    
    /**
     * @brief Get the FFT processor instance
     * @return Shared pointer to the FFT processor
     */
    std::shared_ptr<ofxFft> getFft() const;

private:
    std::shared_ptr<ofxFft> fft;      ///< FFT processor instance
    ofSoundStream stream;              ///< Audio input stream
    std::atomic<bool> useNormalization; ///< Whether to normalize FFT output
    
    std::mutex soundMutex;            ///< Mutex for thread safety
    std::vector<float> audioFront;    ///< Front audio buffer (for drawing/analysis)
    std::vector<float> audioMiddle;   ///< Middle audio buffer (thread-safe handoff)
    std::vector<float> audioBack;     ///< Back audio buffer (for audio thread)
    std::vector<float> audioRaw;      ///< Raw accumulated audio data
    std::vector<float> bins;          ///< FFT frequency bins
    
    /**
     * @brief Normalize a data buffer to range [0, 1]
     * @param data Reference to the data buffer to normalize
     */
    void normalize(std::vector<float>& data);
};
