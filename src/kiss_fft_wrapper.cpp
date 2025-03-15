/**
 * This file provides fallback implementations for KissFFT functions
 * when using platforms that rely on other FFT libraries like Accelerate
 */

#if defined(OFX_FFT_USE_ACCELERATE) || defined(OFX_FFT_USE_APPLE_ACCELERATE)

#include "kiss/kiss_fft.h"
#include "kiss/kiss_fftr.h"
#include <stdlib.h>
#include <string.h>

// Stub implementation for kiss_fft_cleanup
// Safe to be a no-op since we're not using KissFFT
void kiss_fft_cleanup(void) {
    // No-op for Accelerate
}

// Determine next fast size for FFT
// For Accelerate, power of 2 sizes are optimal
int kiss_fft_next_fast_size(int n) {
    // Find next power of 2
    int power = 1;
    while (power < n) {
        power *= 2;
    }
    return power;
}

// Dummy structure for kiss_fftr_cfg
struct kiss_fftr_state {
    int nfft;
    int inverse;
    void* accel_setup;
};

// Allocate a dummy KissFFT real FFT config
kiss_fftr_cfg kiss_fftr_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem) {
    kiss_fftr_cfg cfg;
    size_t memneeded = sizeof(struct kiss_fftr_state);
    
    if (lenmem == NULL) {
        cfg = (kiss_fftr_cfg)malloc(memneeded);
    } else {
        if (mem != NULL && *lenmem >= memneeded) {
            cfg = (kiss_fftr_cfg)mem;
        }
        *lenmem = memneeded;
        if (mem == NULL) return NULL;
    }
    
    if (cfg) {
        cfg->nfft = nfft;
        cfg->inverse = inverse_fft;
        cfg->accel_setup = NULL;
    }
    
    return cfg;
}

// Dummy implementations for KissFFT functions
// These won't be called in practice when using Accelerate, but they
// satisfy the linker requirements
void kiss_fftr(kiss_fftr_cfg cfg, const kiss_fft_scalar* timedata, kiss_fft_cpx* freqdata) {
    // This would normally be implemented using the Accelerate framework
    // But since we're using ofxFftAccelerate class, this won't be called
    memset(freqdata, 0, sizeof(kiss_fft_cpx) * (cfg->nfft/2 + 1));
}

void kiss_fftri(kiss_fftr_cfg cfg, const kiss_fft_cpx* freqdata, kiss_fft_scalar* timedata) {
    // This would normally be implemented using the Accelerate framework
    // But since we're using ofxFftAccelerate class, this won't be called
    memset(timedata, 0, sizeof(kiss_fft_scalar) * cfg->nfft);
}

#endif // defined(OFX_FFT_USE_ACCELERATE) || defined(OFX_FFT_USE_APPLE_ACCELERATE)
