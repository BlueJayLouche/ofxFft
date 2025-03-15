#include "ofxProcessFFT.h"
#include <numeric>
#include <cmath>

ofxProcessFFT::ofxProcessFFT() :
    normalize(false),
    volumeRange(400),
    scaleFactor(10000),
    numBins(16384),
    noisiness(0.0f),
    spectralCentroid(0.0f),
    delta(0.0f),
    loudestBand(0),
    maxSound(0.0f),
    avgMaxSoundOverTime(0.0f),
    superLowEqAvg(0.0f),
    lowEqAvg(0.0f),
    midEqAvg(0.0f),
    highEqAvg(0.0f),
    saveHistory(false),
    graphMaxSize(200),
    numFFTbins(32),
    FFTpercentage(0.14f),
    exponent(1.0f)
{
}

void ofxProcessFFT::setup() {
    // Initialize FFT analyzer
    fft.setup(numBins);
    fft.setUseNormalization(false);
    
    // Allocate history buffers (approx 10sec at 60fps)
    graphLow.assign(graphMaxSize, 0.0f);
    graphMid.assign(graphMaxSize, 0.0f);
    graphHigh.assign(graphMaxSize, 0.0f);
    graphSuperLow.assign(graphMaxSize, 0.0f);
    graphMaxSound.assign(graphMaxSize, 200.0f);
}

void ofxProcessFFT::update() {
    // Update FFT analysis
    fft.update();
    
    // Manage history buffers
    if (saveHistory) {
        if (graphHigh.size() > graphMaxSize) {
            graphHigh.erase(graphHigh.begin());
            graphMid.erase(graphMid.begin());
            graphLow.erase(graphLow.begin());
            graphSuperLow.erase(graphSuperLow.begin());
        }
    }
    
    // Always manage maxSound history
    if (graphMaxSound.size() > graphMaxSize) {
        graphMaxSound.erase(graphMaxSound.begin());
    }
    
    // Process FFT data
    calculateFFT(fft.getBins(), FFTpercentage, numFFTbins);
}

void ofxProcessFFT::calculateFFT(const std::vector<float>& buffer, float fftPercentage, int numFFTbins) {
    // Store parameters
    this->numFFTbins = numFFTbins;
    this->FFTpercentage = fftPercentage;
    
    // Reset data
    fftSpectrum.clear();
    fftSpectrum.resize(numFFTbins, 0.0f);
    
    maxSound = 0.0f;
    loudestBand = 0;
    
    superLowEqAvg = lowEqAvg = midEqAvg = highEqAvg = 0.0f;
    
    // Calculate the bin size for FFT processing
    const float bin_size = buffer.size() * fftPercentage;
    
    // Process FFT data into spectrum bands
    for (int i = 0; i < numFFTbins; i++) {
        // Calculate range of raw bins to include in this spectrum band
        const int startBin = bin_size * ((float)i / numFFTbins);
        const int endBin = bin_size * ((float)(i + 1) / numFFTbins);
        
        // Sum values in this band
        for (int j = startBin; j < endBin; j++) {
            if (j < buffer.size()) {
                fftSpectrum[i] += buffer[j] * scaleFactor;
            }
        }
        
        // Average and apply frequency weighting
        const int numBinsInBand = endBin - startBin;
        if (numBinsInBand > 0) {
            fftSpectrum[i] = std::abs(fftSpectrum[i] / numBinsInBand *
                             (1.0f + std::pow(i, exponent) / numFFTbins));
        }
        
        // Find maximum band
        if (fftSpectrum[i] > maxSound) {
            maxSound = fftSpectrum[i];
            loudestBand = i;
        }
    }
    
    // Store maximum sound in history
    graphMaxSound.push_back(maxSound);
    
    // Calculate average maximum sound over time
    avgMaxSoundOverTime = std::accumulate(graphMaxSound.begin(),
                                        graphMaxSound.end(), 0.0f) /
                                        graphMaxSound.size();
    
    // Process spectrum into frequency bands
    for (int i = 0; i < fftSpectrum.size(); i++) {
        // Normalize if enabled
        if (normalize) {
            fftSpectrum[i] = ofMap(fftSpectrum[i], 0.0f, avgMaxSoundOverTime, 0.0f, 1.0f, true);
        }
        
        // Calculate EQ bands
        if (i == 1) {
            superLowEqAvg = fftSpectrum[0]; // Just use the lowest bin for super low
        }
        
        // Split spectrum into three bands (low, mid, high)
        if (i > 0 && i < numFFTbins * 0.333f) {
            lowEqAvg += fftSpectrum[i];
        } else if (i >= numFFTbins * 0.333f && i < numFFTbins * 0.666f) {
            midEqAvg += fftSpectrum[i];
        } else if (i >= numFFTbins * 0.666f) {
            highEqAvg += fftSpectrum[i];
        }
    }
    
    // Average the bands
    lowEqAvg /= (numFFTbins * 0.333f);
    midEqAvg /= (numFFTbins * 0.333f);
    highEqAvg /= (numFFTbins * 0.333f);
    
    // Save history if enabled
    if (saveHistory) {
        graphSuperLow.push_back(superLowEqAvg);
        graphLow.push_back(lowEqAvg);
        graphMid.push_back(midEqAvg);
        graphHigh.push_back(highEqAvg);
    }
}

void ofxProcessFFT::drawHistoryGraph(const glm::vec2& position, fftRangeType drawType) {
    // Enable history saving
    saveHistory = true;
    
    // Draw appropriate graph based on type
    switch (drawType) {
        case SUPERLOW:
            drawAvgGraph(position, graphSuperLow, ofColor(0, 100, 255, 200));
            break;
        case LOW:
            drawAvgGraph(position, graphLow, ofColor(0, 100, 255, 200));
            break;
        case MID:
            drawAvgGraph(position, graphMid, ofColor(0, 255, 100, 200));
            break;
        case HIGH:
            drawAvgGraph(position, graphHigh, ofColor(255, 0, 100, 200));
            break;
        case MAXSOUND:
            drawAvgGraphUnScaled(position, graphMaxSound, ofColor(255, 100, 255, 200));
            break;
        default:
            drawAvgGraphUnScaled(position, graphMaxSound, ofColor(255, 100, 255, 200));
            break;
    }
}

void ofxProcessFFT::drawAvgGraph(const glm::vec2& position, const std::vector<float>& values, const ofColor& color) {
    if (values.empty()) return;
    
    // Only draw if normalizing or if there's data to show
    if (normalize) {
        ofEnableAlphaBlending();
        ofPushMatrix();
        ofFill();
        ofSetColor(color);
        ofTranslate(position);
        
        ofBeginShape();
        
        // Start at the bottom left
        ofVertex(0, 200);
        
        float avgVal = 0.0f;
        const int drawWidth = 200;
        
        // Draw each point, mapping from the buffer size to 200px width
        for (int i = 0; i < drawWidth; i++) {
            // Map buffer index based on position
            int bufferIndex = ofMap(i, 0, drawWidth, 0, values.size(), true);
            
            // Map value to height (0-1 -> 200-0)
            float height = ofMap(values[bufferIndex], 0.0f, 1.0f, 200.0f, 0.0f, true);
            
            // Draw vertex
            ofVertex(i, height);
            
            // Accumulate for average
            avgVal += values[bufferIndex];
            
            // End at bottom right
            if (i == drawWidth - 1) ofVertex(i, 200);
        }
        
        ofEndShape(false);
        
        // Draw average line
        avgVal /= values.size();
        ofSetColor(255);
        ofDrawLine(0, ofMap(avgVal, 0.0f, 1.0f, 200.0f, 0.0f, true),
                   drawWidth, ofMap(avgVal, 0.0f, 1.0f, 200.0f, 0.0f, true));
        
        ofPopMatrix();
        ofDisableAlphaBlending();
    } else {
        // Draw unnormalized graph
        drawAvgGraphUnScaled(position, values, color);
    }
}

void ofxProcessFFT::drawAvgGraphUnScaled(const glm::vec2& position, const std::vector<float>& values, const ofColor& color) {
    if (values.empty()) return;
    
    ofEnableAlphaBlending();
    ofPushMatrix();
    ofFill();
    ofSetColor(color);
    ofTranslate(position);
    
    ofBeginShape();
    
    // Start at the bottom left
    ofVertex(0, 200);
    
    float avgMaximum = 0.0f;
    float prevAvgMaximum = 0.0f;
    const int drawWidth = 200;
    
    // Draw each point, mapping from the buffer size to 200px width
    for (int i = 0; i < drawWidth; i++) {
        // Map buffer index based on position
        int bufferIndex = ofMap(i, 0, drawWidth, 0, values.size(), true);
        
        // Map value to height (0-volumeRange -> 200-0)
        float height = ofMap(values[bufferIndex], 0.0f, volumeRange, 200.0f, 0.0f, true);
        
        // Draw vertex
        ofVertex(i, height);
        
        // Accumulate for average
        avgMaximum += values[bufferIndex];
        
        // Accumulate for first half average
        if (i < drawWidth / 2) {
            prevAvgMaximum += values[bufferIndex];
        }
        
        // End at bottom right
        if (i == drawWidth - 1) ofVertex(i, 200);
    }
    
    ofEndShape(false);
    
    // Calculate averages
    avgMaximum /= values.size();
    prevAvgMaximum /= (values.size() / 2);
    
    // Calculate delta
    delta = avgMaximum - prevAvgMaximum;
    
    // Draw average line
    ofSetColor(255);
    ofDrawLine(0, ofMap(avgMaximum, 0.0f, volumeRange, 200.0f, 0.0f, true),
               drawWidth, ofMap(avgMaximum, 0.0f, volumeRange, 200.0f, 0.0f, true));
    
    ofPopMatrix();
    ofDisableAlphaBlending();
}

void ofxProcessFFT::drawBars() {
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofSetLineWidth(2);
    
    const float barWidth = ofGetWidth() / (float)numFFTbins;
    const float maxHeight = ofGetHeight() - 50;
    
    for (int i = 0; i < fftSpectrum.size(); i++) {
        // Highlight the loudest band
        if (i == loudestBand) {
            ofSetColor(255, 0, 0);
        } else {
            ofSetColor(100, 100, 200);
        }
        
        ofNoFill();
        
        // Calculate bar height based on normalization setting
        float barHeight;
        if (normalize) {
            barHeight = ofMap(fftSpectrum[i], 0.0f, 1.0f, 0.0f, maxHeight);
        } else {
            barHeight = ofMap(fftSpectrum[i], 0.0f, volumeRange, 0.0f, maxHeight);
        }
        
        // Draw the bar
        ofDrawRectangle(i * barWidth, ofGetHeight() - 20, barWidth, -barHeight);
    }
    
    ofPopStyle();
}

void ofxProcessFFT::drawDebug() {
    ofPushMatrix();
    
    const int startX = 250;
    const int lineHeight = 20;
    int y = 20;
    
    // Display various audio analysis values
    ofDrawBitmapStringHighlight("Loudest Band: " + ofToString(loudestBand), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Curr. Max Sound Val: " + ofToString(maxSound), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Super Low Avg: " + ofToString(superLowEqAvg), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Low Avg: " + ofToString(lowEqAvg), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Mid Avg: " + ofToString(midEqAvg), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("High Avg: " + ofToString(highEqAvg), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Noisiness: " + ofToString(noisiness), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("SpectralCentroid: " + ofToString(spectralCentroid), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Avg Max Sound: " + ofToString(avgMaxSoundOverTime), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Delta: " + ofToString(getDelta()), startX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Delta Shift Detected: " +
                              ofToString(abs(getDelta()) > (avgMaxSoundOverTime * 0.20f)),
                              startX, y);
    y += lineHeight;
    
    // Display frequency information
    const int freqInfoX = 450;
    y = 60;
    
    ofDrawBitmapStringHighlight("Freq Range up to: " +
                              ofToString(ofMap(FFTpercentage, 0.0f, 0.23f, 0.0f, 5000.0f)) + "Hz",
                              freqInfoX, y);
    y += lineHeight;
    
    float freqPerBin = ofMap(FFTpercentage, 0.0f, 0.23f, 0.0f, 5000.0f) / numFFTbins;
    ofDrawBitmapStringHighlight("Freq range per bin: " + ofToString(freqPerBin) + "Hz",
                              freqInfoX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Approx Number of octaves from C0: " +
                              ofToString(ofMap(ofMap(FFTpercentage, 0.0f, 0.23f, 0.0f, 5000.0f),
                                             0.0f, 5000.0f, 0.0f, 8.0f)),
                              freqInfoX, y);
    y += lineHeight;
    
    ofDrawBitmapStringHighlight("Approx Freq of Loudest Band: " +
                              ofToString(freqPerBin * loudestBand) + "Hz",
                              freqInfoX, y);
    
    ofPopMatrix();
}

// Getter methods
float ofxProcessFFT::getIntensityAtFrequency(float frequency) const {
    // Calculate which bin corresponds to this frequency
    const auto& bins = fft.getBins();
    int whichBin = ofMap(frequency, 0.0f, 22100.0f, 0, bins.size());
    
    // Ensure the bin is within valid range
    whichBin = ofClamp(whichBin, 0, bins.size() - 1);
    
    // Normalize the frequency intensity
    float normalizedFreq = ofMap(bins[whichBin] * scaleFactor,
                               0.0f, avgMaxSoundOverTime,
                               0.0f, 1.0f, true);
    
    return normalizedFreq;
}

float ofxProcessFFT::getDelta() const {
    if (graphMaxSound.empty()) return 0.0f;
    
    float prevAvgMaximum = 0.0f;
    float avgMaximum = 0.0f;
    
    for (size_t i = 0; i < graphMaxSound.size(); i++) {
        avgMaximum += graphMaxSound[i];
        
        if (i < graphMaxSound.size() / 2) {
            prevAvgMaximum += graphMaxSound[i];
        }
    }
    
    avgMaximum /= graphMaxSound.size();
    prevAvgMaximum /= (graphMaxSound.size() / 2);
    
    return avgMaximum - prevAvgMaximum;
}

float ofxProcessFFT::getUnScaledLoudestValue() const {
    return maxSound;
}

float ofxProcessFFT::getSmoothedUnScaledLoudestValue() const {
    return avgMaxSoundOverTime;
}

const std::vector<float>& ofxProcessFFT::getSpectrum() const {
    return fftSpectrum;
}

float ofxProcessFFT::getNoisiness() const {
    return noisiness;
}

bool ofxProcessFFT::getNormalized() const {
    return normalize;
}

float ofxProcessFFT::getLoudBand() const {
    return loudestBand;
}

float ofxProcessFFT::getSuperLowVal() const {
    return superLowEqAvg;
}

float ofxProcessFFT::getLowVal() const {
    return lowEqAvg;
}

float ofxProcessFFT::getMidVal() const {
    return midEqAvg;
}

float ofxProcessFFT::getHighVal() const {
    return highEqAvg;
}

float ofxProcessFFT::getSpectralCentroid() const {
    return spectralCentroid;
}

float ofxProcessFFT::getFFTpercentage() const {
    return FFTpercentage;
}

float ofxProcessFFT::getExponent() const {
    return exponent;
}

int ofxProcessFFT::getNumFFTbins() const {
    return numFFTbins;
}

// Setter methods
void ofxProcessFFT::setFFTpercentage(float fftPercentage) {
    this->FFTpercentage = fftPercentage;
}

void ofxProcessFFT::setExponent(float exponent) {
    this->exponent = exponent;
}

void ofxProcessFFT::setNumFFTBins(int numFFTBins) {
    this->numFFTbins = numFFTBins;
}

void ofxProcessFFT::setHistorySize(int framesOfHistory) {
    this->graphMaxSize = framesOfHistory;
    
    // Resize history buffers
    if (graphLow.size() > framesOfHistory) {
        graphLow.resize(framesOfHistory);
        graphMid.resize(framesOfHistory);
        graphHigh.resize(framesOfHistory);
        graphSuperLow.resize(framesOfHistory);
        graphMaxSound.resize(framesOfHistory);
    }
}

void ofxProcessFFT::setNormalize(bool normalize) {
    this->normalize = normalize;
}

void ofxProcessFFT::setVolumeRange(int volumeRange) {
    this->volumeRange = volumeRange;
}
