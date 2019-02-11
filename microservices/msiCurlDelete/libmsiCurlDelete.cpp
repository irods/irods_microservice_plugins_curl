/*
 * libmsiCurlDelete.cpp
 *
 *  Created on: June 1, 2017
 *      Author: Hurng-Chun Lee <h.lee@donders.ru.nl>
 */


// =-=-=-=-=-=-=-
#include "irods_ms_plugin_curl.hpp"


// =-=-=-=-=-=-=-
// New microservice plugin definition style
MICROSERVICE_BEGIN(
	msiCurlDelete,
    STR, url, INPUT,
    KeyValPair, curl_options, INPUT,
    STR, buffer, OUTPUT PTR NO_ALLOC )

    irods::error res = SUCCESS();

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::get_str
    res = myCurl.del( url, &curl_options, &buffer );

    // Done
    RETURN ( res.code());

MICROSERVICE_END
