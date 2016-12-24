#ifndef ATS9462ENGINE_H
#define ATS9462ENGINE_H

// C System-Headers
//
// C++ System headers
//
// AlazarTech Headers
//
// Boost Headers
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
// Qt Headers
//
// Project Specific Headers
#include "ats9462.h"
#include "../JASPL/jAlgorithm/jalgorithm.h"
#include "../JASPL/jFFT/jfft.h"

class ATS9462Engine : public alazar::ATS9462 {

    typedef std::lock_guard<std::mutex> lock;

  public:
    ATS9462Engine(uint signal_samples , uint num_averages);
    ~ATS9462Engine();

    void Start();
    void Stop();
    bool Finished();
    std::vector < float > FinishedSignal();

  protected:
    uint number_averages;
    uint samples_per_average;

    void CallBackUpdate( unsigned long signal_size );
    void CallBackWait( unsigned long signal_size );

  private:

    void UpdateAverage();

    typedef jaspl::RecurseMean< std::vector < float > > float_vec_avg;
    float_vec_avg average_engine;
//    std::unique_ptr< float_vec_avg > average_engine;

    std::vector < float > current_signal;
    jaspl::JFFT fft_er;

    uint samples_half;
    bool ready_flag = false;

    void clean_up();
    std::vector< std::thread > worker_threads;
    void Rebin( std::vector < float >& to_bin );

    std::mutex read_monitor;
};

#endif // ATS9462ENGINE_H
