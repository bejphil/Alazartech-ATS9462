// C System-Headers
//
// C++ System headers
#include <iostream>
//AlazarTech Headers
//
// Boost Headers
#include <boost/lexical_cast.hpp>
//Miscellaneous Headers
#include <omp.h> //OpenMP pragmas
// Project Specific Headers
#include "ats9462.h"

namespace alazar {

ATS9462::ATS9462( uint system_id, uint board_id, uint ring_buffer_size ) : internal_buffer( ring_buffer_size ) {

    board_handle = AlazarGetBoardBySystemID(system_id, board_id);

    if (board_handle == NULL) {
        std::string err_str = "Could not open board handle with system ID# ";
        err_str += boost::lexical_cast<std::string>(system_id);
        err_str += " and board ID# ";
        err_str += boost::lexical_cast<std::string>(board_id);
        throw std::ios_base::failure(err_str);
    }

    err = AlazarGetChannelInfo(board_handle, &max_samples_per_channel, &bits_per_sample);
    ALAZAR_ASSERT(err);

    SetDefaultConfig();

    signal_callback = &alazar::ATS9462::SignalCallback;

    DEBUG_PRINT( "Built new ATS9462" );

}

ATS9462::~ATS9462() {

    AbortCapture();
    DEBUG_PRINT( "Destroyed ATS9462" );

}

void ATS9462::SetDefaultConfig() {

    SelectChannel(Channel::A);
//    SetupRingBuffer( 1e8 );
    SetSampleRate( 10e6 );
    InputControlChannelA();
    InputControlChannelB();
    SetBWLimit();
    SetTriggerOperation();
    SetExternalTrigger();
    SetTriggerTimeOut();
    ConfigureAuxIO();

    SetIntegrationTime(1.0f);

}

//void ATS9462::SetupRingBuffer( uint buffer_size ) {
////    internal_buffer = boost::circular_buffer<unsigned short int> (buffer_size);
//    internal_buffer = threadsafe::ring_buffer< unsigned short int > ( buffer_size );
//    ring_buffer_size = buffer_size;
//}

void ATS9462::SelectChannel(Channel selection) {
    switch (selection) {
    case (Channel::A):
        channel_mask = CHANNEL_A;
        DEBUG_PRINT( "Using Channel A" );
        break;

    case (Channel::B):
        channel_mask = CHANNEL_B;
        DEBUG_PRINT( "Using Channel B" );
        break;

    case (Channel::AB):
        channel_mask = CHANNEL_A | CHANNEL_B;
        DEBUG_PRINT( "Using Channel A & B" );
        break;

    default:
        break;
    }

    channel_count = 0;

    uint total_board_channels = 2;

    for (uint channel = 0; channel < total_board_channels; channel++) {
        U32 channelId = 1U << channel;

        if (channel_mask & channelId) {
            channel_count++;
        }

    }
}

//Wrapper over AlazarTech API functions


unsigned long SamplesToAlazarMacro(uint requested_sample_rate) {

    switch (requested_sample_rate) {
    case (1000):
        return SAMPLE_RATE_1KSPS;
        break;

    case (2000):
        return SAMPLE_RATE_2KSPS;
        break;

    case (5000):
        return SAMPLE_RATE_5KSPS;
        break;

    case (10000):
        return SAMPLE_RATE_10KSPS;
        break;

    case (20000):
        return SAMPLE_RATE_20KSPS;
        break;

    case (50000):
        return SAMPLE_RATE_50KSPS;
        break;

    case (100000):
        return SAMPLE_RATE_100KSPS;
        break;

    case (200000):
        return SAMPLE_RATE_200KSPS;
        break;

    case (500000):
        return SAMPLE_RATE_500KSPS;
        break;

    case (1000000):
        return SAMPLE_RATE_1MSPS;
        break;
    case (2000000):
        return SAMPLE_RATE_2MSPS;
        break;

    case (5000000):
        return SAMPLE_RATE_5MSPS;
        break;

    case (10000000):
        return SAMPLE_RATE_10MSPS;
        break;

    case (20000000):
        return SAMPLE_RATE_20MSPS;
        break;

    case (25000000):
        return SAMPLE_RATE_25MSPS;
        break;

    case (50000000):
        return SAMPLE_RATE_50MSPS;
        break;

    case (100000000):
        return SAMPLE_RATE_100MSPS;
        break;

    case (125000000):
        return SAMPLE_RATE_125MSPS;
        break;

    case (160000000):
        return SAMPLE_RATE_160MSPS;
        break;

    case (180000000):
        DEBUG_PRINT( "Maximum power!");
        return SAMPLE_RATE_180MSPS;
        break;

    default:
        return 0;
        break;
    }
}

void ATS9462::SetSampleRate(uint samples_per_sec) {

    auto rate = SamplesToAlazarMacro(samples_per_sec);

    DEBUG_PRINT( "Sample-rate set to " << samples_per_sec << " samples per second" );

    if (rate == 0) {
        std::string err_str = "Requested sample rate of ";
        err_str += boost::lexical_cast<std::string>(samples_per_sec);
        err_str += " samples per second is not valid.";
        throw std::ios_base::failure(err_str);
    }

    err = AlazarSetCaptureClock(board_handle,
                                INTERNAL_CLOCK,
                                rate,
                                CLOCK_EDGE_RISING,
                                0);
    ALAZAR_ASSERT(err);

    sample_rate = static_cast<double>(samples_per_sec);
}

void ATS9462::InputControlChannelA() {

    DEBUG_PRINT( "Channel A Input Control Set" );

    err = AlazarInputControl(board_handle,
                             CHANNEL_A,
                             DC_COUPLING,
                             INPUT_RANGE_PM_400_MV,
                             IMPEDANCE_50_OHM);
    ALAZAR_ASSERT(err);
}

void ATS9462::InputControlChannelB() {

    DEBUG_PRINT( "Channel B Input Control Set" );

    err = AlazarInputControl(board_handle,
                             CHANNEL_B,
                             DC_COUPLING,
                             INPUT_RANGE_PM_400_MV,
                             IMPEDANCE_50_OHM);
    ALAZAR_ASSERT(err);

}

void ATS9462::SetBWLimit() {

    DEBUG_PRINT( "Bandwidth Limit Set" );

    err = AlazarSetBWLimit(board_handle,
                           CHANNEL_A,
                           0);
    ALAZAR_ASSERT(err);
}

void ATS9462::SetTriggerOperation() {

    DEBUG_PRINT( "Trigger Operation Set" );

    err = AlazarSetTriggerOperation(board_handle,
                                    TRIG_ENGINE_OP_J,
                                    TRIG_ENGINE_J,
                                    TRIG_CHAN_A,
                                    TRIGGER_SLOPE_POSITIVE,
                                    150,
                                    TRIG_ENGINE_K,
                                    TRIG_DISABLE,
                                    TRIGGER_SLOPE_POSITIVE,
                                    128);

    ALAZAR_ASSERT(err);
}

void ATS9462::SetExternalTrigger() {

    DEBUG_PRINT( "External Trigger Set" );

    AlazarSetExternalTrigger(board_handle,
                             DC_COUPLING,
                             ETR_5V);
    ALAZAR_ASSERT(err);
}

void ATS9462::SetTriggerTimeOut(double trigger_timerout_sec) {

    uint triggerTimeout_clocks = static_cast<uint>(trigger_timerout_sec /
                                 sample_rate +
                                 0.5);

    DEBUG_PRINT( "Trigger Time-out set to " << triggerTimeout_clocks << " clock cycles." );

    err = AlazarSetTriggerTimeOut(board_handle, triggerTimeout_clocks);
    ALAZAR_ASSERT(err);

}

void ATS9462::SetIntegrationTime(double time_sec) {

    if (time_sec < 0.0f) {

        std::string err_str = "Requested integration time of ";
        err_str += boost::lexical_cast<std::string>(time_sec);
        err_str += " is negative.";
        throw std::runtime_error(err_str);

    } else {

        DEBUG_PRINT( "Integration time set to " << time_sec << " seconds." );
        integration_time = time_sec;

    }

}

void ATS9462::ConfigureAuxIO() {

    DEBUG_PRINT( "Aux. IO set (Off)" );

    err = AlazarConfigureAuxIO(board_handle, AUX_OUT_TRIGGER, 0);
    ALAZAR_ASSERT(err);

}

void ATS9462::Prequel() {
    // Calculate the size of each DMA buffer in bytes

    float bytes_per_sample = (float)((bits_per_sample + 7) / 8);

    // Calculate the number of buffers in the acquisition

    samples_per_acquisition = static_cast<long int>
                              (sample_rate * integration_time +
                               0.5);

//    samples_per_buffer = static_cast<uint>( (samples_per_acquisition + samples_per_buffer - 1) / buffers_per_acquisition );

    bytes_per_buffer = (uint)(bytes_per_sample * samples_per_buffer * channel_count + 0.5);

    buffers_per_acquisition = static_cast<uint>((samples_per_acquisition +
                              samples_per_buffer -
                              1) / samples_per_buffer);

    DEBUG_PRINT( "Getting " << samples_per_acquisition << " total samples per acquisition." );
    DEBUG_PRINT( "Using " << samples_per_buffer << " samples per buffer." );
    DEBUG_PRINT( "Using " << buffers_per_acquisition << " individual buffers." );

    buffer_array.clear();

    for (uint i = 0; i < buffers_per_acquisition; i ++) {
//        buffer_array.push_back(std::unique_ptr< short unsigned int>((short unsigned int *)malloc(bytes_per_buffer)));
        buffer_array.push_back(std::unique_ptr< short unsigned int>( (short unsigned int*)aligned_alloc( byte_alignment, bytes_per_buffer ) ) );
    }

    uint adma_flags = ADMA_EXTERNAL_STARTCAPTURE | ADMA_CONTINUOUS_MODE;

    err = AlazarBeforeAsyncRead(board_handle,
                                channel_mask,
                                0, // Must be 0
                                samples_per_buffer,
                                1,          // Must be 1
                                0x7FFFFFFF, // Ignored. Behave as if infinite
                                adma_flags);
    ALAZAR_ASSERT(err);

    for (auto &buffer : buffer_array) {
        err = AlazarPostAsyncBuffer(board_handle, buffer.get(), bytes_per_buffer);
        ALAZAR_ASSERT(err);
    }

    DEBUG_PRINT( "Buffers to Digitizer Initialized" );

}

void ATS9462::StartCapture() {

    Prequel();

    err = AlazarStartCapture(board_handle);
    ALAZAR_ASSERT(err);

    capture_switch = true;

    DEBUG_PRINT( "Capture Started (Ring Buffer Thread Spinning)" );

    ring_buffer_thread = std::thread(&ATS9462::CaptureLoop, this);
//    ring_buffer_thread.detach();

}

void ATS9462::CaptureLoop() {

    while( capture_switch == true ) {

        for (uint i = 0; i < buffer_array.size() ; i++) {

            DEBUG_PRINT( "Dumping critical buffer #" << i );

            err = AlazarWaitAsyncBufferComplete(board_handle, buffer_array[i].get(), 5000); //500 = timeout in ms.
            ALAZAR_ASSERT(err);

            internal_buffer.TailInsert( buffer_array[i].get(), samples_per_buffer );

            err = AlazarPostAsyncBuffer(board_handle, buffer_array[i].get(), bytes_per_buffer);
            ALAZAR_ASSERT(err);

        }


        if ( samples_per_buffer > ULONG_MAX - samples_since_last_read ) {
            samples_since_last_read -= samples_per_acquisition;
        } else {
            samples_since_last_read += samples_per_acquisition;
            DEBUG_PRINT( "Samples since last read event " << samples_since_last_read );
        }

        (this->*signal_callback)( samples_since_last_read );

    }
}

inline float SamplesToVolts(short unsigned int sample_value) {
    // AlazarTech digitizers are calibrated as follows
    int bitsPerSample = 16;
    float codeZero = (1 << (bitsPerSample - 1)) - 0.5;
    float codeRange = (1 << (bitsPerSample - 1)) - 0.5;

    float inputRange_volts = 0.400f;

    // Convert sample code to volts
    return inputRange_volts * ((sample_value - codeZero) / codeRange);
}

std::vector<short unsigned int> ATS9462::PullRawDataHead( uint data_size ) {

    if ( !internal_buffer.CheckHead(data_size) ) {
        DEBUG_PRINT( __func__ << " Read Failed!" );
        return std::vector<short unsigned int>() = { 0 };
    }

    auto copy_vec = internal_buffer.HeadRead( data_size );
    DEBUG_PRINT( "Read " << copy_vec.size() << " samples from head of ring buffer." );
    samples_since_last_read = 0;

    return copy_vec;
}

std::vector<float> ATS9462::PullVoltageDataHead(uint data_size) {

    if (internal_buffer.size() <= data_size) {
        DEBUG_PRINT( __func__ << " Read Failed, not enough samples in ring buffer" );
        return std::vector < float >() = { 0.0f };

    }

    auto raw_data = PullRawDataHead(data_size);

    std::vector<float> converted_data(data_size, 0.0f);

    for (uint i = 0; i < raw_data.size() ; i ++) {
        converted_data[i] = SamplesToVolts(raw_data[i]);
    }

    return converted_data;
}

std::vector<short unsigned int> ATS9462::PullRawDataTail( uint data_size ) {

    if ( !internal_buffer.CheckTail(data_size) ) {
        DEBUG_PRINT( __func__ << " Read Failed!" );
        return std::vector<short unsigned int>() = { 0 };
    }

    auto copy_vec = internal_buffer.TailRead( data_size );
    DEBUG_PRINT( "Read " << copy_vec.size() << " samples from head of ring buffer." );
    samples_since_last_read = 0;

    return copy_vec;
}

std::vector<float> ATS9462::PullVoltageDataTail(uint data_size) {

    auto raw_data = PullRawDataTail( data_size );

    std::vector<float> converted_data;
    converted_data.reserve( data_size );

    for (uint i = 0; i < raw_data.size() ; i ++) {
        converted_data.push_back( raw_data[i] );
//        converted_data[i] = SamplesToVolts(raw_data[i]);
    }

    return converted_data;
}

void ATS9462::AbortCapture() {

    capture_switch = false;

    DEBUG_PRINT( "Capture stopped" );

    if ( ring_buffer_thread.joinable() ) {
        try {
            ring_buffer_thread.join();
        } catch (std::system_error &e) {
            std::cout << "Thread joining failed!" << e.what() << std::endl;
        }
    }

    DEBUG_PRINT( "Ring buffer thread re-joined main thread" );

    //Almost always throws, especially when called through destructor?
    err = AlazarAbortAsyncRead(board_handle);
    ALAZAR_ASSERT(err);
}

}
