// C System-Headers
//
// C++ System headers
//
//AlazarTech Headers
//
// Boost Headers
#include <boost/lexical_cast.hpp>
//Miscellaneous Headers
#include <omp.h> //OpenMP pragmas
// Project Specific Headers
#include "ats9462.h"

namespace alazar {

ATS9462::ATS9462(uint system_id, uint board_id) {

  board_handle = AlazarGetBoardBySystemID(system_id, board_id);

  if (board_handle == NULL) {
    std::string err_str = "Could not open board handle with system ID# ";
    err_str += boost::lexical_cast<std::string>(system_id);
    err_str += " and board ID# ";
    err_str += boost::lexical_cast<std::string>(board_id);
    throw std::ios_base::failure(err_str);
  }

  err = AlazarGetChannelInfo(board_handle, &max_samples_per_channel,
                             &bits_per_sample);
  ALAZAR_ASSERT(err);

  internal_buffer = boost::circular_buffer<unsigned short int> (ring_buffer_size);

}

ATS9462::~ATS9462() {

  AbortCapture();

  if (ring_buffer_thread.joinable()) {
    ring_buffer_thread.join();
  }

}

void ATS9462::SetDefaultConfig() {

  SelectChannel(Channel::A);
  SetSampleRate(10e6);
  InputControlChannelA();
  InputControlChannelB();
  SetBWLimit();
  SetTriggerOperation();
  SetExternalTrigger();
  SetTriggerTimeOut();
  ConfigureAuxIO();

  SetIntegrationTime(1.0f);

}

void ATS9462::SelectChannel(Channel selection) {
  switch (selection) {
  case (Channel::A):
    channel_mask = CHANNEL_A;
    break;

  case (Channel::B):
    channel_mask = CHANNEL_B;
    break;

  case (Channel::AB):
    channel_mask = CHANNEL_A | CHANNEL_B;
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
  case (1e3):
    return SAMPLE_RATE_1KSPS;
    break;

  case (2e3):
    return SAMPLE_RATE_2KSPS;
    break;

  case (5e3):
    return SAMPLE_RATE_5KSPS;
    break;

  case (10e3):
    return SAMPLE_RATE_10KSPS;
    break;

  case (20e3):
    return SAMPLE_RATE_20KSPS;
    break;

  case (50e3):
    return SAMPLE_RATE_50KSPS;
    break;

  case (100e3):
    return SAMPLE_RATE_100KSPS;
    break;

  case (200e3):
    return SAMPLE_RATE_200KSPS;
    break;

  case (500e3):
    return SAMPLE_RATE_500KSPS;
    break;

  case (1e6):
    return SAMPLE_RATE_1MSPS;
    break;

  case (2e6):
    return SAMPLE_RATE_2MSPS;
    break;

  case (5e6):
    return SAMPLE_RATE_5MSPS;
    break;

  case (10e6):
    return SAMPLE_RATE_10MSPS;
    break;

  case (20e6):
    return SAMPLE_RATE_20MSPS;
    break;

  case (25e6):
    return SAMPLE_RATE_25MSPS;
    break;

  case (50e6):
    return SAMPLE_RATE_50MSPS;
    break;

  case (100e6):
    return SAMPLE_RATE_100MSPS;
    break;

  case (125e6):
    return SAMPLE_RATE_125MSPS;
    break;

  case (160e6):
    return SAMPLE_RATE_160MSPS;
    break;

  case (180e6):
    break;

  default:
    return SAMPLE_RATE_180MSPS;
    return 0;
    break;
  }
}

void ATS9462::SetSampleRate(uint samples_per_sec) {

  auto rate = SamplesToAlazarMacro(samples_per_sec);

  if (rate == 0) {
    std::string err_str = "Requested sample rate of ";
    err_str += boost::lexical_cast<std::string>(samples_per_sec);
    err_str += " samples per second is not valid.";
    throw std::ios_base::failure(err_str);
  }


//SAMPLE_RATE_10MSPS
  err = AlazarSetCaptureClock(board_handle,
                              INTERNAL_CLOCK,
                              rate,
                              CLOCK_EDGE_RISING,
                              0);
  ALAZAR_ASSERT(err);

  sample_rate = static_cast<double>(samples_per_sec);
}

void ATS9462::InputControlChannelA() {
  err = AlazarInputControl(board_handle,
                           CHANNEL_A,
                           DC_COUPLING,
                           INPUT_RANGE_PM_400_MV,
                           IMPEDANCE_50_OHM);
  ALAZAR_ASSERT(err);
}

void ATS9462::InputControlChannelB() {
  err = AlazarInputControl(board_handle,
                           CHANNEL_B,
                           DC_COUPLING,
                           INPUT_RANGE_PM_400_MV,
                           IMPEDANCE_50_OHM);
  ALAZAR_ASSERT(err);

}

void ATS9462::SetBWLimit() {
  err = AlazarSetBWLimit(board_handle,
                         CHANNEL_A,
                         0);
  ALAZAR_ASSERT(err);
}

void ATS9462::SetTriggerOperation() {
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
  AlazarSetExternalTrigger(board_handle,
                           DC_COUPLING,
                           ETR_5V);
  ALAZAR_ASSERT(err);
}

//void ATS9462::SetTriggerDelay(uint triggerDelay_samples) {

//}

void ATS9462::SetTriggerTimeOut(double trigger_timerout_sec) {

  uint triggerTimeout_clocks = static_cast<uint>(trigger_timerout_sec /
                               sample_rate +
                               0.5);

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

    integration_time = time_sec;

  }

}

void ATS9462::ConfigureAuxIO() {

  err = AlazarConfigureAuxIO(board_handle, AUX_OUT_TRIGGER, 0);
  ALAZAR_ASSERT(err);

}

void ATS9462::Prequel() {
  // Calculate the size of each DMA buffer in bytes

  float bytes_per_sample = (float)((bits_per_sample + 7) / 8);
  bytes_per_buffer = (uint)(bytes_per_sample * samples_per_buffer *
                            channel_count + 0.5);

  // Calculate the number of buffers in the acquisition

  long int samples_per_acquisition = static_cast<long int>
                                     (sample_rate * integration_time +
                                      0.5);

  buffers_per_acquisition = static_cast<uint>((samples_per_acquisition +
                            samples_per_buffer -
                            1) / samples_per_buffer);

  std::cout << buffers_per_acquisition << std::endl;

  for (uint i = 0; i < buffers_per_acquisition; i ++) {
    buffer_array.push_back(std::unique_ptr< short unsigned int>((
                             short unsigned int *)valloc(bytes_per_buffer)));
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

}

void ATS9462::StartCapture() {

  Prequel();

  err = AlazarStartCapture(board_handle);
  ALAZAR_ASSERT(err);

  capture_switch = true;

  ring_buffer_thread = std::thread(&ATS9462::CaptureLoop, this);
//  ring_buffer_thread.detach();

}

void ATS9462::CaptureLoop() {

  while (capture_switch == true) {

    for (uint i = 0; i < buffer_array.size() ; i++) {

      if ( samples_per_buffer > ULONG_MAX - samples_since_last_read ) {
      } else {
        samples_since_last_read += samples_per_buffer;
      }


      err = AlazarWaitAsyncBufferComplete(board_handle, buffer_array[i].get(),
                                          500); //500 = timeout in ms.
      ALAZAR_ASSERT(err);

//      lock lk(monitor);

      monitor.lock();
      auto head = buffer_array[i].get();
      auto tail = head + samples_per_buffer;

      internal_buffer.insert(internal_buffer.end(), head, tail);
      monitor.unlock();

      err = AlazarPostAsyncBuffer(board_handle, buffer_array[i].get(),
                                  bytes_per_buffer);

      ALAZAR_ASSERT(err);

      //Catch and ignore ApiDmaPaused error
//      if (err == 520) {
//        continue;
//      } else {
//        ALAZAR_ASSERT(err);
//      }

    }

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

std::vector<short unsigned int> ATS9462::PullRawData(uint data_size) {

  //Need to lock as soon as we access iterators so they are not
  //made invalid while this function is executing
  lock lk(monitor);
  auto first = internal_buffer.begin() + 0;
  auto last = internal_buffer.begin() + data_size;

  //Attempt to read more samples than are current stored in
  //the ring buffer
  bool check_condition = (std::distance(first, last) < data_size);
  //Attempt to read data that was read in last cycle ( eg. Old Data )
  check_condition |= (std::distance(first, last) > samples_since_last_read);

  if (check_condition) {
    return std::vector<short unsigned int>() = { 0 };
  }

  auto copy_vec = std::vector<short unsigned int>(first, last);
//  samples_since_last_read = 0;

  return copy_vec;
}

std::vector<float> ATS9462::PullVoltageData(uint data_size) {

  if (internal_buffer.size() <= data_size) {

    return std::vector < float >() = { 0.0f };

  }

  auto raw_data = PullRawData(data_size);

  std::vector<float> converted_data(data_size, 0.0f);

  for (uint i = 0; i < raw_data.size() ; i ++) {
    converted_data[i] = SamplesToVolts(raw_data[i]);
  }

  return converted_data;
}

std::vector<short unsigned int> ATS9462::PullRawDataTail(uint data_size) {

  //Need to lock as soon as we access iterators so they are not
  //made invalid while this function is executing
  lock lk(monitor);
  auto first = internal_buffer.end() - data_size;
  auto last = internal_buffer.end() ;

  //Attempt to read more samples than are current stored in
  //the ring buffer
  bool check_condition = (std::distance(first, last) < data_size);
  //Attempt to read data that was read in last cycle ( eg. Old Data )
  check_condition |= (std::distance(first, last) > samples_since_last_read);

  if (check_condition) {
    return std::vector<short unsigned int>() = { 0 };
  }

  auto copy_vec = std::vector<short unsigned int>(first, last);
//  samples_since_last_read = 0;

  return copy_vec;
  //  internal_buffer.erase(internal_buffer.end() - data_size,
  //                        internal_buffer.end());
}

std::vector<float> ATS9462::PullVoltageDataTail(uint data_size) {

  if (internal_buffer.size() <= data_size) {

    return std::vector < float >() = { 0.0f };

  }

  auto raw_data = PullRawDataTail(data_size);

  if (raw_data.size() <= 1)
    return std::vector<float>() = { 0.0f };

  std::vector<float> converted_data(data_size, 0.0f);

  for (uint i = 0; i < raw_data.size() ; i ++) {
    converted_data[i] = SamplesToVolts(raw_data[i]);
  }

  return converted_data;
}

void ATS9462::AbortCapture() {

  capture_switch = false;

  err = AlazarAbortAsyncRead(board_handle);
  ALAZAR_ASSERT(err);
}

}
