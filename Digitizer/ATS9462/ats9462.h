#ifndef ATS9462_H
#define ATS9462_H

// C System-Headers
#include <stdio.h>
#include <stdint.h>
// C++ System headers
#include <string>
#include <stdexcept>
#include <list>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
//AlazarTech Headers
#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"
// Boost Headers
//
// Project Specific Headers
#include "../../Debug/debug.h"
#include "../../../JASPL/Containers/ouroboros.h"

class ATS9462 {

    enum class Channel {
        A, B, AB
    };

  public:

    typedef std::lock_guard< std::mutex > lock;

    ATS9462(uint system_id = 1, uint board_id = 1, uint ring_buffer_size = 1e8 );
    ~ATS9462();

    //Non-wrapper functions

    virtual std::vector<short unsigned int> PullRawDataHead(uint data_size);
    virtual std::vector<float> PullVoltageDataHead(uint data_size);

    virtual std::vector<short unsigned int> PullRawDataTail(uint data_size);
    virtual std::vector<float> PullVoltageDataTail(uint data_size);

    virtual void SetDefaultConfig();

    void SelectChannel(Channel selection);

    //Wrapper over AlazarTech API functions
    void SetSampleRate(uint samples_per_sec);
    void InputControlChannelA();
    void InputControlChannelB();
    void SetBWLimit();
    void SetTriggerOperation();
    void SetExternalTrigger();
    void SetTriggerTimeOut(double trigger_timerout_sec = 0.0f);
    void ConfigureAuxIO();

    void SetIntegrationTime(double time_sec);

    virtual void StartCapture();
    virtual void AbortCapture();

  protected:
    u_char bits_per_sample;
    uint max_samples_per_channel;

    // All buffer read from the digitizer MUST
    // have a 64 byte alignment, e.g. the need
    // to be allocated with aligned_alloc( 64, 64*buffer_bytes );
    const uint byte_alignment = 64;

    uint bytes_per_buffer = 0;
    long int samples_per_acquisition = 0;
    uint samples_per_buffer = 204800;
    const uint buffers_per_acquisition = 8;

    double integration_time = 0.0f;
    double sample_rate = 0.0f;

    virtual void Prequel();
    virtual void CaptureLoop();
    virtual void SignalCallback() {
        DEBUG_PRINT( "alazar::ATS9462::SignalCallBack " );
    }

    void (ATS9462::*signal_callback)() = NULL;

    jaspl::ouroborus< short unsigned int > internal_buffer;

  private:
    std::vector< std::unique_ptr< short unsigned int > > buffer_array;

    HANDLE board_handle = NULL;
    RETURN_CODE err;

    uint channel_mask;
    uint channel_count = 0;

    bool capture_switch = false;

    std::thread ring_buffer_thread;

};

#endif // ATS9462_H
