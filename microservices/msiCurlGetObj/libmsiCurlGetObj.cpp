/*
 * libmsiCurlGetObj.cpp
 *
 *  Created on: May 20, 2014
 *      Author: adt
 */



// =-=-=-=-=-=-=-
#include "irods_ms_plugin_curl.hpp"


// =-=-=-=-=-=-=-
// New microservice plugin definition style
MICROSERVICE_BEGIN(
	msiCurlGetObj,
    STR, url, INPUT,
    STR, object, INPUT,
    INT, downloaded, OUTPUT ALLOC )

    size_t transferred = 0;			// total transferred
    irods::error res = SUCCESS();

    // Sanity checks
    if ( !rei || !rei->rsComm ) {
        rodsLog( LOG_ERROR, "msiCurlGetObj: Input rei or rsComm is NULL." );
        RETURN ( SYS_INTERNAL_NULL_INPUT_ERR );
    }

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::get_obj
    res = myCurl.get_obj( url, object, &transferred );

	// Return bytes read/written
    downloaded = transferred;

    // Done
    RETURN ( res.code());

MICROSERVICE_END

