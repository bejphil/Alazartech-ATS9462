#include "ats9462engine.h"
#include <thread>
#include <algorithm>

ATS9462Engine::ATS9462Engine(uint signal_samples, uint num_averages , uint ring_buffer_size) :\
    alazar::ATS9462( 1, 1, ring_buffer_size ),\
    average_engine ( (signal_samples % 2 == 0) ? (signal_samples / 2) : (( signal_samples - 1) / 2) ),\
    number_averages( num_averages),\
    samples_per_average( signal_samples ), \
    fft_er( true ) \
{

    samples_half = (samples_per_average % 2 == 0) ? (samples_per_average / 2) : (( samples_per_average - 1) / 2);
    fft_er.SetUp( signal_samples );

    DEBUG_PRINT( "Built new ATS9462Engine" );
}

void ATS9462Engine::FinalThreadCleanUp() {

    DEBUG_PRINT( __func__ )
    DEBUG_PRINT( "Cleaning up " << worker_threads.size() << " Worker Threads" );

    for ( auto& thread : worker_threads ) {
        if ( thread.joinable() ) {
            try {
                thread.join();
                DEBUG_PRINT( "Worker thread re-joined main thread" );
            } catch (std::system_error &e) {
                DEBUG_PRINT( "Thread joining failed!" << e.what() );
            }
        }
    }

    worker_threads.clear();
}

void ATS9462Engine::ThreadCleanUp() {

    DEBUG_PRINT( __func__ )
    DEBUG_PRINT( "Cleaning up " << worker_threads.size() << " Worker Threads" );

    auto iter = std::begin(worker_threads);

    while ( iter != std::end(worker_threads)) {

        auto thred = &(*iter);

        auto find_iter = std::find( complete_thread_ids.begin(), complete_thread_ids.end(), thred->get_id() );

        if ( find_iter != complete_thread_ids.end() ) {

            DEBUG_PRINT( "Found Thread in completed threads lists" );

            if ( thred->joinable() ) {
                try {
                    thred->join();
                    DEBUG_PRINT( "Worker thread re-joined main thread" );
                } catch (std::system_error &e) {
                    DEBUG_PRINT( "Thread joining failed!" << e.what() );
                }

                iter = worker_threads.erase( iter );
            }
        } else {
            ++ iter;
        }
    }
}

ATS9462Engine::~ATS9462Engine() {

    FinalThreadCleanUp();
    fft_er.TearDown();
    DEBUG_PRINT( "Destroyed ATS9462Engine" );
}

void ATS9462Engine::Start() {

    ready_flag = false;
    pending_avg_index = 0;
    average_engine.Reset();

    signal_callback = static_cast< void (alazar::ATS9462::*)( unsigned long )>( &ATS9462Engine::CallBackUpdate );

}

void ATS9462Engine::Stop() {

    DEBUG_PRINT( "ATS9462Engine: Averaging finished, stopping...");

    signal_callback = static_cast< void (alazar::ATS9462::*)( unsigned long )>( &ATS9462Engine::CallBackWait );
    ready_flag = true;

}

void ATS9462Engine::CallBackWait( unsigned long signal_size ) {

    DEBUG_PRINT( "ATS9462Engine::CallBackWait " );
    ThreadCallback( num_active_threads );
}

void ATS9462Engine::CallBackUpdate( unsigned long signal_size ) {

    DEBUG_PRINT( "ATS9462Engine::CallBackUpdate" );
    DEBUG_PRINT( "ATS9462Engine::Current pending index " << pending_avg_index );

    if ( pending_avg_index >= number_averages ) {
        Stop();
        return;
    }

    bool check_criteria = ( signal_size >= samples_per_average ); //Are there enough samples in the ring buffer (probably)?
    check_criteria &= ( num_active_threads <= thread_limit );//Are there too many active threads?
    check_criteria &= internal_buffer.CheckTail( samples_per_average );//Explicitly check if there are enough samples to read

    //If all checks are good, make a new thread and increment counters
    if ( check_criteria ) {
        pending_avg_index ++;
        num_active_threads ++;
        DEBUG_PRINT( "Number of active threads: " << num_active_threads );
        ThreadCleanUp();
        worker_threads.push_back( std::thread( &ATS9462Engine::UpdateAverage, this ) );

    }
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
        num_active_threads --;
        pending_avg_index --;
        complete_thread_ids.push_back( std::this_thread::get_id() );

        return;
    }

    auto volts_data = PullVoltageDataTail( samples_per_average );
    fft_er.PowerSpectrum( volts_data );

    float samples_f = static_cast<float>( samples_per_average );

    //Remove negative part of FFT
    volts_data.erase( volts_data.end() - samples_half , volts_data.end() );

    std::for_each( volts_data.begin(), volts_data.end(), VoltsTodBm_FFTCorrection(samples_f) );

    float time_correction = 1.0f / integration_time;
    if( time_correction != 1.0f ) {
        std::for_each(volts_data.begin(),\
                      volts_data.end(),\
                      std::bind1st (std::multiplies <float> (), time_correction) );
    }

    average_engine( volts_data );

    num_active_threads --;

    complete_thread_ids.push_back( std::this_thread::get_id() );

}

void ATS9462Engine::ThreadCallback( unsigned int num_threads ) {

    DEBUG_PRINT( num_threads << " Currently Active" );

    if( num_threads == 0 ) {
        DEBUG_PRINT( "Cleaning up..." );
        FinalThreadCleanUp();
    } else {
        DEBUG_PRINT( "Not all threads are finished." );
        return;
    }
}

bool ATS9462Engine::Finished() {
    return( static_cast<uint>( average_engine.Index() >= number_averages ) );
}

std::vector < float > ATS9462Engine::FinishedSignal() {
    return average_engine.ReturnValue();
}
