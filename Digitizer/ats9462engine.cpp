#include "ats9462engine.h"

ATS9462Engine::ATS9462Engine(uint signal_samples, uint num_averages , uint ring_buffer_size) :\
    alazar::ATS9462( 1, 1, ring_buffer_size ),\
    number_averages( num_averages ),\
    samples_per_average( signal_samples ), \
    average_engine ( (signal_samples % 2 == 0) ? (signal_samples / 2) : (( signal_samples - 1) / 2) ),\
    fft_er( true ) \
{

    samples_half = (samples_per_average % 2 == 0) ? (samples_per_average / 2) : (( samples_per_average - 1) / 2);
    fft_er.SetUp( signal_samples );

    DEBUG_PRINT( "Built new ATS9462Engine" );
}

template <typename T>
bool thread_is_finished( std::future<T>& to_check ) {
    // Use wait_for() with zero milliseconds to check thread status.
    auto status = to_check.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
        DEBUG_PRINT( "Thread finished" );
        return true;
    } else {
        DEBUG_PRINT( "Thread still running...");
        return false;
    }
}

void ATS9462Engine::FuturesCleanUp() {

    DEBUG_PRINT( __func__ )
    DEBUG_PRINT( "Cleaning up " << results.size() << " Futures" );

    for ( uint i = 0; i < results.size() ; i ++ ) {

        bool check_cond = results.at(i).valid() && thread_is_finished( results.at(i) );
        if( check_cond ) {
            try {
                results.at(i).get();
                DEBUG_PRINT( "Future finished successfully" );
                results.erase( results.begin() + i );
            } catch (std::future_error &e) {
                DEBUG_PRINT( "Future thread an error:" << e.what() );
            }
        }
    }
}

ATS9462Engine::~ATS9462Engine() {

    FuturesCleanUp();
    fft_er.TearDown();
    DEBUG_PRINT( "Destroyed ATS9462Engine" );
}

void ATS9462Engine::Start() {

    ready_flag = false;
    pending_avg_index = 0;
    average_engine.Reset();

    signal_callback = static_cast< void (alazar::ATS9462::*)()>( &ATS9462Engine::CallBackUpdate );

}

void ATS9462Engine::Stop() {

    DEBUG_PRINT( "ATS9462Engine: Averaging finished, stopping...");

    signal_callback = static_cast< void (alazar::ATS9462::*)()>( &ATS9462Engine::CallBackWait );
    ready_flag = true;

}

void ATS9462Engine::CallBackWait() {

    DEBUG_PRINT( "ATS9462Engine::CallBackWait " );
    ThreadCallback();
}

void ATS9462Engine::CallBackUpdate() {

    DEBUG_PRINT( "ATS9462Engine::CallBackUpdate" );
    DEBUG_PRINT( "ATS9462Engine::Current pending index " << pending_avg_index );

    if ( pending_avg_index >= number_averages ) {
        Stop();
        return;
    } else {
        FuturesCleanUp();
    }

    bool check_criteria = ( internal_buffer.size() >= samples_per_average ); //Are there enough samples in the ring buffer (probably)?
    check_criteria &= ( results.size() <= thread_limit );//Are there too many active threads?
    check_criteria &= internal_buffer.CheckTail( samples_per_average );//Explicitly check if there are enough samples to read

    //If all checks are good, make a new thread and increment counters
    if ( check_criteria ) {
        pending_avg_index ++;
        DEBUG_PRINT( "Number of active threads: " << results.size() );

        results.push_back( std::async( std::launch::async, &ATS9462Engine::UpdateAverage, this ) );
    }
}

float volts_to_dbm( float voltage ) {
    return 20.0f * log10f( voltage / 50.0f );
}

struct VoltsTodBm {

    void operator()(float &voltage) const {
        voltage = volts_to_dbm( voltage );
    }
};

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

inline float SamplesToVolts(short unsigned int sample_value) {
    // AlazarTech digitizers are calibrated as follows
    int bitsPerSample = 16;
    float codeZero = (1 << (bitsPerSample - 1)) - 0.5;
    float codeRange = (1 << (bitsPerSample - 1)) - 0.5;

    float inputRange_volts = 0.400f;

    // Convert sample code to volts
    return inputRange_volts * ((sample_value - codeZero) / codeRange);
}

inline float Samples2Volts( const short unsigned int& sample_value) {

    float code = (1 << (15)) - 0.5;
    return 0.400f * ((sample_value - code) / code);
}

void ATS9462Engine::UpdateAverage() {

    if ( !internal_buffer.CheckTail( samples_per_average ) ) {
        pending_avg_index --;
        return;
    }

    auto volts_data = PullVoltageDataTail( samples_per_average );
    fft_er.PowerSpectrum( volts_data );

    //Remove negative part of FFT
    volts_data.erase( volts_data.end() - samples_half , volts_data.end() );

    float correction_term = 1.0/( 0.5 );

    std::transform(volts_data.begin(),\
                   volts_data.end(),\
                   volts_data.begin(),
                   std::bind1st(std::multiplies<float>(), correction_term));

    std::for_each( volts_data.begin(), volts_data.end(), VoltsTodBm() );

    average_engine( volts_data );

    DEBUG_PRINT( "Thread finished" );
}

void ATS9462Engine::ThreadCallback() {

    FuturesCleanUp();

}

bool ATS9462Engine::Finished() {
    return( static_cast<uint>( average_engine.Index() >= number_averages ) );
}

std::vector < float > ATS9462Engine::FinishedSignal() {
    return average_engine.ReturnValue();
}
