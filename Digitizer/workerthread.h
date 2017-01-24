#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

// C System-Headers
//
// C++ System headers
#include <iostream>
#include <thread>
#include <algorithm>
#include <functional>
#include <future>
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

class WorkerThread {

  public:
    WorkerThread( std::function<void(void)> &processor );

  private:
    std::thread internal_thread;
    void process( const std::function<void(void)> &f );

    std::future<void> result;

    void CleanUpThread();
    boost::signals2::signal< void( void ) > thread_finished;

};

#endif // WORKERTHREAD_HH
