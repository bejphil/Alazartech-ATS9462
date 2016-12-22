#include "ats9462engine.h"

ATS9462Engine::ATS9462Engine( uint signal_samples, uint num_averages ) :\
    average_engine ( signal_samples ),\
    number_averages( num_averages),\
    samples_per_average( signal_samples) {

    samples_half = (samples_per_average % 2 == 0) ? (samples_per_average / 2) : (( samples_per_average - 1) / 2);
    fft_er.SetUp( signal_samples );

}

ATS9462Engine::~ATS9462Engine() {
    fft_er.TearDown();
}

void VoltsTodBm(float &voltage) {
    voltage = 20.0f * log10f(voltage / 50.0f);
}

//Combination of operations designed to compensate for
//1/N term introduce in FFT, and then convert from volts
//to dBm
//1) compensate for FFT effects: voltage/signal_size
//2) convert to dBm: volts -> dBm
//3) compensate for discarding half of spectrum: dBm/2
struct VoltsTodBm_FFTCorrection {
    float val;
    VoltsTodBm_FFTCorrection(float signal_size) : val(signal_size) {}

    void operator()(float &voltage) const {
        voltage = 10.0f * log10f(voltage / (50.0f * val));
    }
};

void ATS9462Engine::UpdateAverage() {

    auto volts_data = PullVoltageDataTail( samples_per_average );
    fft_er.PowerSpectrum( volts_data );

    float samples_f = static_cast<float>( samples_per_average );

    volts_data.erase(volts_data.end() - samples_half , volts_data.end());

    std::for_each(volts_data.begin(), volts_data.end(), VoltsTodBm_FFTCorrection(samples_f));

    float time_correction = 1.0f / integration_time;
    std::for_each(volts_data.begin(), volts_data.end(), std::bind1st (std::multiplies <float> () , time_correction) );

    average_engine( volts_data );

    uint N = static_cast<uint>( average_engine.Index() );

    if ( N >= number_averages ) {
        average_engine.Reset();
        AbortCapture();
    }
}
