#include <Python.h>
#include <boost/python.hpp>
using namespace boost::python;

#include "ats9462engine.h"
#include "ats9462engine.cpp"


/*
class ATS9462Engine : public ATS9462 {

  public:
    ATS9462Engine(uint signal_samples , uint num_averages, uint ring_buffer_size );
    ~ATS9462Engine();

    void Start();
    void Stop();
    bool Finished();

    void ThreadPoolSize( uint num_threads );
    std::vector < float > FinishedSignal();
*/

BOOST_PYTHON_MODULE( ats9462 )
{

    class_< ATS9462Engine, boost::noncopyable >("ATS9462Engine",init<uint,uint,uint>())
            .def(init<uint,uint,uint>())
            .def("Start", &ATS9462Engine::Start)
            .def("Stop", &ATS9462Engine::Stop)
            .def("Finished", &ATS9462Engine::Finished)
            .def("ThreadPoolSize", &ATS9462Engine::ThreadPoolSize)
            .def("FinishedSignal", &ATS9462Engine::FinishedSignal);
    ;

}
