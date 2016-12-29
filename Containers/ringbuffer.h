#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <mutex>
#include <vector>

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>

template < typename T >
class RingBuffer {

    typedef boost::mutex::scoped_lock lock;

  public:
    RingBuffer();
    RingBuffer( uint buffer_size );

    void HeadInsert( T& to_insert );
    void TailInsert( T*& to_insert, uint insert_size );

    std::vector<T> HeadRead( uint read_size );
    std::vector<T> TailRead( uint read_size );

    uint index();

  private:

    void update_counter( uint insert_size );

    boost::circular_buffer<T> internal_buffer;
    std::mutex monitor;
    uint last_read_counter = 0;

};

template < typename T >
uint RingBuffer<T>::index() {
    lock read_lock( monitor );
    return last_read_counter;
}

template < typename T >
void RingBuffer<T>::TailInsert( T*& to_insert, uint insert_size ) {

    lock write_lock( monitor );
    auto head = to_insert;
    auto tail = head + insert_size;

    internal_buffer.insert( internal_buffer.end(), head, tail );
    update_counter( insert_size );

}

template < typename T >
bool RingBuffer<T>::CheckHead( uint data_size ) {

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
bool RingBuffer<T>::CheckTail( uint data_size ) {

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
std::vector<T> RingBuffer<T>::TailRead( uint read_size ) {

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

    read_size = 0;

    return copy_vec;

}

template < typename T >
std::vector<T> RingBuffer<T>::HeadRead( uint read_size ) {

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

    read_size = 0;

    return copy_vec;
}

template < typename T >
uint RingBuffer<T>::update_counter( uint insert_size ) {

    lock write_lock( monitor );
    if ( insert_size > ( ULONG_MAX - last_read_counter ) ) {
        last_read_counter -= insert_size;
    } else {
        last_read_counter += insert_size;
        DEBUG_PRINT( "Samples since last read event " << last_read_counter );
    }

}

#endif // RINGBUFFER_H
