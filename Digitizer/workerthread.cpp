#include "workerthread.h"

WorkerThread::WorkerThread( std::function<void(void)> &processor ) {

//    thread_finished.connect( &WorkerThread::CleanUpThread );
    result = std::async( std::launch::async, processor );
//    internal_thread = std::thread( &WorkerThread::process, processor, this );
}

void WorkerThread::CleanUpThread() {
    if ( internal_thread.joinable() ) {
        try {
            internal_thread.join();
            DEBUG_PRINT( "Worker thread re-joined main thread" );
        } catch (std::system_error &e) {
            DEBUG_PRINT( "Thread joining failed!" << e.what() );
        }
    }
}

void WorkerThread::process( const std::function<void(void)> &f  ) {
    f();
    thread_finished();
}
