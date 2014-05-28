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
    STR, response, OUTPUT ALLOC )

    //char *buffer = NULL;
    irods::error res = SUCCESS();

    // Sanity checks
    if ( !rei || !rei->rsComm ) {
        rodsLog( LOG_ERROR, "msiCurlGetObj: Input rei or rsComm is NULL." );
        RETURN ( SYS_INTERNAL_NULL_INPUT_ERR );
    }

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::get
    res = myCurl.post( url, &post_fields, &response );

    // Done
    RETURN ( res.code());

MICROSERVICE_END

