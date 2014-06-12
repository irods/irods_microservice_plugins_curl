/*
 * libmsiCurlGetStr.cpp
 *
 *  Created on: May 20, 2014
 *      Author: adt
 */



// =-=-=-=-=-=-=-
#include "irods_ms_plugin_curl.hpp"


// =-=-=-=-=-=-=-
// New microservice plugin definition style
MICROSERVICE_BEGIN(
	msiCurlGetStr,
    STR, url, INPUT,
    STR, buffer, OUTPUT PTR NO_ALLOC )

    irods::error res = SUCCESS();

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::get_str
    res = myCurl.get_str( url, &buffer );

    // Done
    RETURN ( res.code());

MICROSERVICE_END



