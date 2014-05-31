/*
 * libmsiCurlPost.cpp
 *
 *  Created on: May 27, 2014
 *      Author: adt
 */


// =-=-=-=-=-=-=-
#include "irods_ms_plugin_curl.hpp"


// =-=-=-=-=-=-=-
// New microservice plugin definition style
MICROSERVICE_BEGIN(
	msiCurlPost,
    STR, url, INPUT,
    KeyValPair, post_fields, INPUT,
    STR, response, OUTPUT PTR ALLOC )

    irods::error res = SUCCESS();

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::post
    res = myCurl.post( url, &post_fields, &response );

    // Done
    RETURN ( res.code());

MICROSERVICE_END

