/*
 * libmsiCurlPut.cpp
 *
 *  Created on: May 27, 2014
 *      Author: adt
 */


// =-=-=-=-=-=-=-
#include "irods_ms_plugin_curl.hpp"


// =-=-=-=-=-=-=-
// New microservice plugin definition style
MICROSERVICE_BEGIN(
	msiCurlPut,
    STR, url, INPUT,
    KeyValPair, post_fields, INPUT,
    KeyValPair, curl_options, INPUT,
    STR, response, OUTPUT PTR NO_ALLOC )

    irods::error res = SUCCESS();

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::post
    res = myCurl.put( url, &post_fields, &curl_options, &response );

    // Done
    RETURN ( res.code());

MICROSERVICE_END

