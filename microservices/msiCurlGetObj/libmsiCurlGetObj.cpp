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

    // Call irodsCurl::get
    res = myCurl.get_obj( url, object, &transferred );

	// Return bytes read/written
    downloaded = transferred;

    // Done
    RETURN ( res.code());

MICROSERVICE_END


#if 0
// =-=-=-=-=-=-=-
// Old style (still valid)
extern "C" {


	// =-=-=-=-=-=-=-
	// 1. Write a standard issue microservice
	int msiCurlGetObj(msParam_t *url, msParam_t *object, msParam_t *downloaded, ruleExecInfo_t *rei) {
        dataObjInp_t destObjInp, *myDestObjInp;		// for parsing input object
        size_t transferred = 0;						// total transferred
        irods::error res = SUCCESS();
        char* my_url;

        // Sanity checks
        if ( !rei || !rei->rsComm ) {
            rodsLog( LOG_ERROR, "msiCurlGetObj: Input rei or rsComm is NULL." );
            return ( SYS_INTERNAL_NULL_INPUT_ERR );
        }

        // Check url input
        my_url = parseMspForStr(url);
        if (!my_url || !strlen(my_url)) {
        	rodsLog (LOG_ERROR, "msiCurlGetObj: Null or empty url input.");
        	return (USER_INPUT_STRING_ERR);
        }

        // Get path of destination object
        rei->status = parseMspForDataObjInp( object, &destObjInp, &myDestObjInp, 0 );
        if ( rei->status < 0 ) {
            rodsLog( LOG_ERROR, "msiCurlGetObj: Input object error. status = %d", rei->status );
            return ( rei->status );
        }

        // Create irodsCurl instance
        irodsCurl myCurl( rei->rsComm );

        // Call irodsCurl::get
        res = myCurl.get( my_url, destObjInp.objPath, &transferred );

    	// Return bytes read/written
        fillIntInMsParam(downloaded, transferred);

        // Done
        return res.code();

    }


	// =-=-=-=-=-=-=-
	// 2.  Create the plugin factory function which will return a microservice
	//     table entry
    irods::ms_table_entry*  plugin_factory() {
        // =-=-=-=-=-=-=-
        // 3.  allocate a microservice plugin which takes the number of function
        //     params as a parameter to the constructor
        irods::ms_table_entry* msvc = new irods::ms_table_entry( 3 );

        // =-=-=-=-=-=-=-
        // 4. add the microservice function as an operation to the plugin
        //    the first param is the name / key of the operation, the second
        //    is the name of the function which will be the microservice
        msvc->add_operation( "msiCurlGetObj", "msiCurlGetObj" );

        // =-=-=-=-=-=-=-
        // 5. return the newly created microservice plugin
        return msvc;
    }

}	// extern "C"

#endif






