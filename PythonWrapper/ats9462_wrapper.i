%module ats9462

%{
#include "../Digitizer/ats9462engine.h"
%}

%include "std_vector.i"

namespace std {
    %template(VectorFloat) vector<float>;
};

class ATS9462Engine {

  public:
    ATS9462Engine( uint signal_samples , uint num_averages, uint ring_buffer_size );

    void Start();
    void Stop();

    bool Finished();

    void ThreadPoolSize( uint num_threads );
    std::vector < float > FinishedSignal();

};
