#include "ats9462engine.h"
#include <thread>
#include <future>

ATS9462Engine::ATS9462Engine( uint signal_samples, uint num_averages ) :\
    average_engine ( (signal_samples % 2 == 0) ? (signal_samples / 2) : (( signal_samples - 1) / 2) ),\
    number_averages( num_averages),\
    samples_per_average( signal_samples ), \
    fft_er( false ) \
{

    samples_half = (samples_per_average % 2 == 0) ? (samples_per_average / 2) : (( samples_per_average - 1) / 2);

    fft_er.SetUp( signal_samples );

    DEBUG_PRINT( "Built new ATS9462Engine" );
}

ATS9462Engine::~ATS9462Engine() {

    for ( auto& thread : worker_threads ) {
        if ( thread.joinable() ) {
            try {
                thread.join();
            } catch (std::system_error &e) {
                std::cout << "Thread joining failed!" << e.what() << std::endl;
            }
        }
    }

    fft_er.TearDown();
    DEBUG_PRINT( "Destroyed ATS9462Engine" );
}

void ATS9462Engine::Start() {

    ready_flag == false;
    average_engine.Reset();

    signal_callback = static_cast< void (alazar::ATS9462::*)( unsigned long )>( &ATS9462Engine::CallBackUpdate );

}

void ATS9462Engine::Stop() {

    DEBUG_PRINT( "ATS9462Engine: Averaging finished, stopping...");

    signal_callback = static_cast< void (alazar::ATS9462::*)( unsigned long )>( &ATS9462Engine::CallBackWait );

//    for ( auto& thread : worker_threads ) {
//        if ( thread.joinable() ) {
//            try {
//                thread.join();
//            } catch (std::system_error &e) {
//                std::cout << "Thread joining failed!" << e.what() << std::endl;
//            }
//        }
//    }

//    current_signal = average_engine.ReturnValue();

//    average_engine.Reset();
//    ready_flag = true;

}

void ATS9462Engine::CallBackWait( unsigned long signal_size ) {

    DEBUG_PRINT( "ATS9462Engine::CallBackWait " << signal_size << " Samples read" );
}

void ATS9462Engine::CallBackUpdate( unsigned long signal_size ) {

    DEBUG_PRINT( "ATS9462Engine::CallBackUpdate" );

    DEBUG_PRINT( "ATS9462Engine::Current pending index " << pending_avg_index );

    if ( pending_avg_index >= number_averages ) {
        Stop();
        return;
    }

    if ( signal_size >= samples_per_average ) {

        if( CheckTail( samples_per_average ) ) {

            pending_avg_index ++;
            worker_threads.push_back( std::thread( &ATS9462Engine::UpdateAverage, this ) );
        }

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

inline float Samples2VoltsFast( short unsigned int sample_value ) {
    return 0.400f*( (sample_value - ( ( 1 << 15 ) - 0.5 ) ) / ( ( 1 << 15 ) - 0.5 )  );
}

void ATS9462Engine::UpdateAverage() {

    DEBUG_PRINT( "ATS9462Engine Updating Average..." );
//    auto raw_data = PullRawDataTail( samples_per_average );

    std::vector < short unsigned int > raw_data;
    do {
        raw_data = PullRawDataTail( samples_per_average );
    } while ( raw_data.size() < samples_per_average );

    std::vector<float> volts_data;
    volts_data.reserve( raw_data.size() );

    for (uint i = 0; i < raw_data.size() ; i ++) {
        volts_data.push_back( Samples2VoltsFast( raw_data[i] ) );
    }

    fft_er.PowerSpectrum( volts_data );

    float samples_f = static_cast<float>( samples_per_average );

    volts_data.erase( volts_data.end() - samples_half , volts_data.end() );

    std::for_each( volts_data.begin(), volts_data.end(), VoltsTodBm_FFTCorrection(samples_f) );

    float time_correction = 1.0f / integration_time;
    if( time_correction != 1.0f )
        std::for_each(volts_data.begin(), volts_data.end(), std::bind1st (std::multiplies <float> () , time_correction) );

    average_engine( volts_data );
}

bool ATS9462Engine::Finished() {
    return( static_cast<uint>( average_engine.Index() >= number_averages ) );
}

std::vector < float > ATS9462Engine::FinishedSignal() {
    return average_engine.ReturnValue();
}
