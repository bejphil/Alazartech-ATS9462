#ifndef DEBUG_H
#define DEBUG_H

#ifdef QT_DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

// C System-Headers
#include <stdio.h>
// C++ System headers
#include <sstream>
#include <string>
#include <stdexcept>
//AlazarTech Headers
#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"
// Boost Headers
//
// Miscellaneous Headers
//

#define ALAZAR_DEBUG(err, ...) \
            do { if (DEBUG) std::cout << __func__ <<\
        __FILE__ <<\
        " Line# "<<\
        __LINE__ <<\
        " Status: " <<\
        AlazarErrorToText( err ) << std::endl; } while (0)

#define ALAZAR_ASSERT(err, ...) \
            do { \
    if ( err != ApiSuccess ) {\
        std::string err_str = __func__;\
        err_str += __FILE__;\
        err_str += " Line # ";\
        err_str += boost::lexical_cast<std::string>( __LINE__ );\
        err_str += " ";\
        err_str += std::string( AlazarErrorToText( err ) );\
        throw std::runtime_error( err_str );\
    } } while (0)



class alazar_error: public std::runtime_error {
public:

  alazar_error( RETURN_CODE ret ): runtime_error( "AlazarTech Error" ), err( ret ) {}

  virtual const char* what() const throw()
  {
    cnvt.str( "" );

    cnvt << runtime_error::what() << GetErrCode() ;

    return cnvt.str().c_str();
  }

  int GetErrCode() const
    { return err; }

private:
    RETURN_CODE err;
    static std::ostringstream cnvt;
};

//std::ostringstream alazar_error::cnvt;

#endif // DEBUG_H
