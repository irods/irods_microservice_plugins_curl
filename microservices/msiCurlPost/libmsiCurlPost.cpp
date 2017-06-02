/*
 * libmsiCurlPost.cpp
 *
 *  Created on: June 1, 2017
 *      Author: Hurng-Chun Lee <h.lee@donders.ru.nl> 
 */


// =-=-=-=-=-=-=-
#include "irods_ms_plugin_curl.hpp"

// =-=-=-=-=-=-=-
// New microservice plugin definition style
MICROSERVICE_BEGIN(
	msiCurlPost,
    STR, url, INPUT,
    KeyValPair, post_fields, INPUT,
    STR, response, OUTPUT PTR NO_ALLOC )

    irods::error res = SUCCESS();

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::post
    res = myCurl.post( url, &post_fields, &response );

    // Done
    RETURN ( res.code());

MICROSERVICE_END

