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
#include "boost/circular_buffer.hpp"
#include <boost/lockfree/spsc_queue.hpp>
// Project Specific Headers
#include "debug.h"

namespace alazar {

class ATS9462 {

  enum class Channel { A, B, AB };

 public:

  typedef std::lock_guard<std::mutex> lock;
  ATS9462(uint system_id = 1, uint board_id = 1);
  ~ATS9462();
  void SetDefaultConfig();

  //Non-wrapper functions
  std::vector<short unsigned int> PullRawData(uint data_size);
  std::vector<float> PullVoltageData(uint data_size);

  std::vector<short unsigned int> PullRawDataTail(uint data_size);
  std::vector<float> PullVoltageDataTail(uint data_size);

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
//  void SetSampleRate(double samples_per_sec);

  void StartCapture();
  void AbortCapture();

 protected:
  u_char bits_per_sample;
  uint max_samples_per_channel;

  uint bytes_per_buffer = 0;
  uint buffers_per_acquisition = 0;


  uint buffer_count = 4;

  uint samples_per_buffer = 204800;

  double integration_time = 0.0f;
  double sample_rate = 0.0f;

  uint ring_buffer_size = 1e8;

 private:

  void Prequel();
  void CaptureLoop();

  std::vector< std::unique_ptr< short unsigned int > > buffer_array;
  boost::circular_buffer<short unsigned int> internal_buffer;

  HANDLE board_handle = NULL;
  RETURN_CODE err;

  uint channel_mask;
  uint channel_count = 0;
  long unsigned int samples_since_last_read = 0;

  bool capture_switch = false;
  std::thread ring_buffer_thread;
  std::mutex monitor;

};

}

#endif // ATS9462_H
