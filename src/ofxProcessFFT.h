#pragma once

/**
 * @brief Advanced FFT processing and visualization
 *
 * This class provides utilities for processing and visualizing FFT data,
 * including filtering, EQ bands, and various visualization methods.
 */

#include "ofMain.h"
#include "ofxEasyFft.h"
#include <vector>
#include <array>

/**
 * @brief Frequency range types for analysis
 */
enum fftRangeType {
    SUPERLOW,   ///< Super low frequencies (sub-bass)
    LOW,        ///< Low frequencies (bass)
    MID,        ///< Mid frequencies
    HIGH,       ///< High frequencies (treble)
    MAXSOUND    ///< Loudest frequencies
};

class ofxProcessFFT {
public:
    /**
     * @brief Constructor
     */
    ofxProcessFFT();
    
    /**
     * @brief Initialize the FFT processor
     */
    void setup();
    
    /**
     * @brief Update FFT processing
     *
     * Should be called regularly (typically in the update() method of an app)
     */
    void update();
    
    /**
     * @brief Draw a history graph of a specific frequency range
     *
     * @param position Position to draw the graph
     * @param drawType Type of frequency range to visualize
     */
    void drawHistoryGraph(const glm::vec2& position, fftRangeType drawType);
    
    /**
     * @brief Get the loudest frequency band
     * @return Index of the loudest frequency band
     */
    float getLoudBand() const;
    
    /**
     * @brief Get the super low frequency average amplitude
     * @return Average amplitude of super low frequencies
     */
    float getSuperLowVal() const;
    
    /**
     * @brief Get the low frequency average amplitude
     * @return Average amplitude of low frequencies
     */
    float getLowVal() const;
    
    /**
     * @brief Get the mid frequency average amplitude
     * @return Average amplitude of mid frequencies
     */
    float getMidVal() const;
    
    /**
     * @brief Get the high frequency average amplitude
     * @return Average amplitude of high frequencies
     */
    float getHighVal() const;
    
    /**
     * @brief Get the noisiness index (spectral flatness)
     * @return Noisiness index (0-1, higher means more noise-like)
     */
    float getNoisiness() const;
    
    /**
     * @brief Check if normalization is enabled
     * @return True if normalization is enabled
     */
    bool getNormalized() const;
    
    /**
     * @brief Get the spectral centroid (brightness)
     * @return Spectral centroid value
     */
    float getSpectralCentroid() const;
    
    /**
     * @brief Get the amplitude change rate
     * @return Delta value indicating amplitude change
     */
    float getDelta() const;
    
    /**
     * @brief Get the unscaled loudest value
     * @return Raw value of the loudest frequency
     */
    float getUnScaledLoudestValue() const;
    
    /**
     * @brief Get the smoothed unscaled loudest value
     * @return Smoothed raw value of the loudest frequency
     */
    float getSmoothedUnScaledLoudestValue() const;
    
    /**
     * @brief Get amplitude at a specific frequency
     * @param frequency Target frequency in Hz
     * @return Amplitude value at the specified frequency
     */
    float getIntensityAtFrequency(float frequency) const;
    
    /**
     * @brief Get the full FFT spectrum
     * @return Reference to the spectrum vector
     */
    const std::vector<float>& getSpectrum() const;
    
    /**
     * @brief Get the number of FFT bins
     * @return Number of FFT bins
     */
    int getNumFFTbins() const;
    
    /**
     * @brief Get the FFT frequency percentage
     * @return Percentage of the frequency range used
     */
    float getFFTpercentage() const;
    
    /**
     * @brief Get the frequency weighting exponent
     * @return Exponent value for frequency weighting
     */
    float getExponent() const;
    
    /**
     * @brief Set the number of FFT bins
     * @param numFFTBins Number of bins to use
     */
    void setNumFFTBins(int numFFTBins);
    
    /**
     * @brief Set the FFT frequency percentage
     * @param fftPercentage Percentage of frequency range to analyze
     */
    void setFFTpercentage(float fftPercentage);
    
    /**
     * @brief Set the frequency weighting exponent
     * @param exponent Exponent value for frequency weighting
     */
    void setExponent(float exponent);
    
    /**
     * @brief Set the history buffer size
     * @param framesOfHistory Number of frames to keep in history
     */
    void setHistorySize(int framesOfHistory);
    
    /**
     * @brief Set the volume range for unnormalized output
     * @param volumeRange Maximum volume value
     */
    void setVolumeRange(int volumeRange);
    
    /**
     * @brief Enable or disable normalization
     * @param normalize Whether to normalize output
     */
    void setNormalize(bool normalize);
    
    /**
     * @brief Draw frequency bars visualization
     */
    void drawBars();
    
    /**
     * @brief Draw debug information
     */
    void drawDebug();
    
    /// The FFT analyzer instance
    ofxEasyFft fft;

private:
    bool normalize;            ///< Whether to normalize output
    int volumeRange;           ///< Volume range for unnormalized output
    
    int scaleFactor;           ///< Scaling factor for FFT values
    int numBins;               ///< Number of FFT bins
    float noisiness;           ///< Spectral flatness measure
    float spectralCentroid;    ///< Spectral centroid (brightness)
    
    float delta;               ///< Rate of change in amplitude
    
    /**
     * @brief Calculate FFT spectrum
     * @param buffer Input FFT buffer
     * @param fftPercentage Percentage of frequency range to analyze
     * @param numFFTbins Number of bins to use
     */
    void calculateFFT(const std::vector<float>& buffer, float fftPercentage, int numFFTbins);
    
    bool saveHistory;          ///< Whether to save history data
    int graphMaxSize;          ///< Maximum size of history graphs
    
    int numFFTbins;            ///< Number of FFT bins for analysis
    float FFTpercentage;       ///< Percentage of frequency range
    float exponent;            ///< Frequency weighting exponent
    
    int loudestBand;           ///< Index of the loudest frequency band
    
    float maxSound;            ///< Maximum sound amplitude
    float avgMaxSoundOverTime; ///< Average maximum sound over time
    
    std::vector<float> fftSpectrum;     ///< Processed FFT spectrum
    std::vector<float> graphLow;        ///< History of low frequencies
    std::vector<float> graphMid;        ///< History of mid frequencies
    std::vector<float> graphHigh;       ///< History of high frequencies
    std::vector<float> graphSuperLow;   ///< History of super low frequencies
    std::vector<float> graphMaxSound;   ///< History of maximum sound
    
    float superLowEqAvg;   ///< Average amplitude of super low frequencies
    float lowEqAvg;        ///< Average amplitude of low frequencies
    float midEqAvg;        ///< Average amplitude of mid frequencies
    float highEqAvg;       ///< Average amplitude of high frequencies
    
    /**
     * @brief Draw an average graph with normalized values
     * @param position Position to draw the graph
     * @param values Values to visualize
     * @param color Color of the graph
     */
    void drawAvgGraph(const glm::vec2& position, const std::vector<float>& values, const ofColor& color);
    
    /**
     * @brief Draw an average graph with unscaled values
     * @param position Position to draw the graph
     * @param values Values to visualize
     * @param color Color of the graph
     */
    void drawAvgGraphUnScaled(const glm::vec2& position, const std::vector<float>& values, const ofColor& color);
};
