#ifndef RINGBUFFER_H
#define RINGBUFFER_H

// C System-Headers
//
// C++ System headers
#include <mutex>
#include <vector>
//AlazarTech Headers
//
// Boost Headers
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>
// Project Specific Headers
#include "../Digitizer/debug.h"

namespace threadsafe {

template < typename T >
class ring_buffer {

    typedef boost::mutex::scoped_lock lock;

  public:
    ring_buffer();
    ring_buffer( uint buffer_size );

    void HeadInsert( T* to_insert );
    void TailInsert( T* to_insert, uint insert_size );

    std::vector<T> HeadRead( uint read_size );
    bool CheckHead( uint read_size );

    std::vector<T> TailRead( uint read_size );
    bool CheckTail( uint read_size );

    uint index();
    uint size();

  private:

    void update_counter( uint insert_size );

    boost::circular_buffer<T> internal_buffer;
    boost::mutex monitor;
    uint last_read_counter = 0;

};

template < typename T >
ring_buffer<T>::ring_buffer() {
    //
}

template < typename T>
ring_buffer<T>::ring_buffer( uint buffer_size ) : internal_buffer( buffer_size ) {
    DEBUG_PRINT( "Built new ring_buffer of size " << buffer_size );
}

template < typename T >
uint ring_buffer<T>::index() {
    lock read_lock( monitor );
    return last_read_counter;
}

template < typename T >
uint ring_buffer<T>::size() {
    lock read_lock( monitor );
    return internal_buffer.size();
}

template < typename T >
void ring_buffer<T>::TailInsert( T* to_insert, uint insert_size ) {

    DEBUG_PRINT( "Inserting " << insert_size << " elements into ring buffer" );

//    lock write_lock( monitor );
    auto head = to_insert;
    auto tail = head + insert_size;

    internal_buffer.insert( internal_buffer.end(), head, tail );
    update_counter( insert_size );

}

template < typename T >
bool ring_buffer<T>::CheckHead( uint data_size ) {

    lock read_lock(monitor);
    auto first = internal_buffer.begin();
    auto last = internal_buffer.begin() + data_size;

    //Attempt to read more samples than are current stored in
    //the ring buffer
    bool check_condition = (std::distance(first, last) < data_size);
    //Attempt to read data that was read in last cycle ( eg. Old Data )
    check_condition |= ( static_cast<uint>( std::distance(first, last) ) > last_read_counter);

    if (check_condition) {
        DEBUG_PRINT( __func__ << " Unable to read, not enough samples in ring buffer" );
    } else {
        DEBUG_PRINT( __func__ << " Able to read." );
    }

    return !check_condition;
}

template < typename T >
bool ring_buffer<T>::CheckTail( uint data_size ) {

    lock read_lock(monitor);
    auto first = internal_buffer.end() - data_size;
    auto last = internal_buffer.end() ;

    //Attempt to read more samples than are current stored in
    //the ring buffer
    bool check_condition = (std::distance(first, last) < data_size);
    //Attempt to read data that was read in last cycle ( eg. Old Data )
    check_condition |= ( (uint)std::distance(first, last) > last_read_counter);

    if (check_condition) {
        DEBUG_PRINT( __func__ << " Unable to read, not enough samples in ring buffer" );
    } else {
        DEBUG_PRINT( __func__ << " Able to read." );
    }

    return !check_condition;
}

template < typename T >
std::vector<T> ring_buffer<T>::TailRead( uint read_size ) {

    lock read_lock( monitor );

    auto first = internal_buffer.end() - read_size;
    auto last = internal_buffer.end() ;
    //Attempt to read more samples than are current stored in
    //the ring buffer
    bool check_condition = (std::distance(first, last) < read_size);
    //Attempt to read data that was read in last cycle ( eg. Old Data )
    check_condition |= ( (uint)std::distance(first, last) > last_read_counter);

    if (check_condition) {
        DEBUG_PRINT( __func__ << " Unable to read, not enough samples in ring buffer" );

        std::string err_str = "Not enough new data in ring buffer: Have ";
        err_str += boost::lexical_cast<std::string>( last_read_counter);
        err_str += " new data samples, but  ";
        err_str += boost::lexical_cast<std::string>( read_size );
        err_str += " were requested.";
        throw std::ios_base::failure(err_str);

    } else {
        DEBUG_PRINT( __func__ << " Able to read." );
    }

    auto copy_vec = std::vector< T >( first, last );

    last_read_counter = 0;

    return copy_vec;

}

template < typename T >
std::vector<T> ring_buffer<T>::HeadRead( uint read_size ) {

    lock read_lock( monitor );

    auto first = internal_buffer.begin() + 0;
    auto last = internal_buffer.begin() + read_size;
    //Attempt to read more samples than are current stored in
    //the ring buffer
    bool check_condition = (std::distance(first, last) < read_size);
    //Attempt to read data that was read in last cycle ( eg. Old Data )
    check_condition |= ( (uint)std::distance(first, last) > last_read_counter);

    if (check_condition) {
        DEBUG_PRINT( __func__ << " Unable to read, not enough samples in ring buffer" );

        std::string err_str = "Not enough new data in ring buffer: Have ";
        err_str += boost::lexical_cast<std::string>( last_read_counter);
        err_str += " new data samples, but  ";
        err_str += boost::lexical_cast<std::string>( read_size );
        err_str += " were requested.";
        throw std::ios_base::failure(err_str);
    } else {
        DEBUG_PRINT( __func__ << " Able to read." );
    }

    auto copy_vec = std::vector< T >( first, last );

    last_read_counter = 0;

    return copy_vec;
}

template < typename T >
void ring_buffer<T>::update_counter( uint insert_size ) {

    lock write_lock( monitor );
    if ( insert_size > ( ULONG_MAX - last_read_counter ) ) {
        last_read_counter -= insert_size;
    } else {
        last_read_counter += insert_size;
        DEBUG_PRINT( "Samples since last read event " << last_read_counter );
    }

}

}

#endif // RINGBUFFER_H
