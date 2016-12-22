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

class ATS9462Engine : alazar::ATS9462 {

  public:
    ATS9462Engine(uint signal_samples , uint num_averages);
    ~ATS9462Engine();

    void Start();
    bool Finished();
    std::vector < float > FinishedSignal();

  protected:
    uint number_averages;
    uint samples_per_average;

  private:

    void UpdateAverage();

    jaspl::RecurseMean< std::vector < float > > average_engine;
    std::vector < float > current_signal;
    jaspl::JFFT fft_er;

    uint samples_half;

    std::vector< std::thread > worker_threads;
    void Rebin( std::vector < float >& to_bin );
};

#endif // ATS9462ENGINE_H
