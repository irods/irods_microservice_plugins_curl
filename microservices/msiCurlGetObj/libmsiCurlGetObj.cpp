/*
 * libmsiCurlGetObj.cpp
 *
 *  Created on: May 20, 2014
 *      Author: adt
 */



// =-=-=-=-=-=-=-
#include "apiHeaderAll.hpp"
#include "msParam.hpp"
#include "reGlobalsExtern.hpp"
#include "irods_ms_plugin.hpp"
#include "irods_server_api_call.hpp"
#include "microservice.hpp"



// =-=-=-=-=-=-=-
// STL Includes
#include <string>
#include <iostream>


#include <curl/curl.h>
#include <curl/easy.h>

/* types.h is not included in newer versions of libcurl */
#if LIBCURL_VERSION_NUM < 0x071503
#include <curl/types.h>
#endif


typedef struct {
    char objPath[MAX_NAME_LEN];
    int l1descInx;
    rsComm_t *rsComm;
} writeDataInp_t;


typedef struct {
	size_t downloaded;
	size_t cutoff;	/* 0 means unlimited */
} curlProgress_t;



class irodsCurl {
private:
    // iRODS server handle
    rsComm_t *rsComm;

    // cURL handle
    CURL *curl;


public:
    irodsCurl( rsComm_t *comm ) {
        rsComm = comm;

        curl = curl_easy_init();
        if ( !curl ) {
            rodsLog( LOG_ERROR, "irodsCurl: %s", curl_easy_strerror( CURLE_FAILED_INIT ) );
        }
    }

    ~irodsCurl() {
        if ( curl ) {
            curl_easy_cleanup( curl );
        }
    }

    irods::error get( char *url, char *objPath, size_t *transferred ) {
    	CURLcode res = CURLE_OK;
    	writeDataInp_t writeDataInp;			// the "file descriptor" for our destination object
    	openedDataObjInp_t openedDataObjInp;	// for closing iRODS object after writing
    	curlProgress_t prog;					// for progress and cutoff
    	int status;

    	// Zero fill openedDataObjInp
    	memset( &openedDataObjInp, 0, sizeof( openedDataObjInp_t ) );

    	// Set up writeDataInp
    	snprintf( writeDataInp.objPath, MAX_NAME_LEN, "%s", objPath );
    	writeDataInp.l1descInx = 0;	// the object is yet to be created
    	writeDataInp.rsComm = rsComm;

    	// Progress struct init
    	prog.downloaded = 0;
    	prog.cutoff = 0;

    	// Set up easy handler
    	curl_easy_setopt( curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, &irodsCurl::my_write_obj );
    	curl_easy_setopt( curl, CURLOPT_WRITEDATA, &writeDataInp );
    	curl_easy_setopt( curl, CURLOPT_URL, url );

    	/* Progress settings */
    	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
    	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
    	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    	// CURL call
    	res = curl_easy_perform( curl );

    	// Some error logging
    	if ( res != CURLE_OK ) {
    		rodsLog( LOG_ERROR, "irodsCurl::get: cURL error: %s", curl_easy_strerror( res ) );
    	}

    	// close iRODS object
    	if ( writeDataInp.l1descInx ) {
    		openedDataObjInp.l1descInx = writeDataInp.l1descInx;

    		//status = rsDataObjClose( rsComm, &openedDataObjInp );
    		status = irods::server_api_call( DATA_OBJ_CLOSE_AN, rsComm, &openedDataObjInp );

    		if ( status < 0 ) {
    			rodsLog( LOG_ERROR, "irodsCurl::get: rsDataObjClose failed for %s, status = %d",
    					writeDataInp.objPath, status );
    		}
    	}

    	// log total transferred
    	*transferred = prog.downloaded;

    	return CODE(res);
    }


    // Callback progress function for the curl handler */
    static int progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow) {
    	curlProgress_t *prog = (curlProgress_t *)p;

    	/* Update total so far */
    	prog->downloaded = (size_t)dlnow;

    	/* Abort if next transfer could exceed cutoff */
    	if (prog->cutoff && (dlnow + CURL_MAX_WRITE_SIZE > prog->cutoff)) {
    		rodsLog(LOG_NOTICE, "progress(): Aborting curl download, max size is %d bytes", prog->cutoff);
    		return -1;
    	}

    	return 0;
    }


    // Custom callback function for the curl handler, to write to an iRODS object
    static size_t my_write_obj( void *buffer, size_t size, size_t nmemb, writeDataInp_t *writeDataInp ) {
        dataObjInp_t dataObjInp;				// input struct for rsDataObjCreate
        openedDataObjInp_t openedDataObjInp;	// input struct for rsDataObjWrite
        bytesBuf_t bytesBuf;					// input buffer for rsDataObjWrite
        size_t written;							// return value

        int l1descInx;


        // Make sure we have something to write to
        if ( !writeDataInp ) {
            rodsLog( LOG_ERROR, "my_write_obj: writeDataInp is NULL, status = %d", SYS_INTERNAL_NULL_INPUT_ERR );
            return SYS_INTERNAL_NULL_INPUT_ERR;
        }

        // Zero fill input structs
        memset( &dataObjInp, 0, sizeof( dataObjInp_t ) );
        memset( &openedDataObjInp, 0, sizeof( openedDataObjInp_t ) );


        // If this is the first call we need to create our data object before writing to it
        if ( !writeDataInp->l1descInx ) {
            strncpy( dataObjInp.objPath, writeDataInp->objPath, MAX_NAME_LEN );

            // Overwrite existing file (for this tutorial only, in case the example has been run before)
            addKeyVal( &dataObjInp.condInput, FORCE_FLAG_KW, "" );

            //writeDataInp->l1descInx = rsDataObjCreate( writeDataInp->rsComm, &dataObjInp );
            writeDataInp->l1descInx = irods::server_api_call( DATA_OBJ_CREATE_AN, writeDataInp->rsComm, &dataObjInp );

            // No create?
            if ( writeDataInp->l1descInx <= 2 ) {
                rodsLog( LOG_ERROR, "my_write_obj: rsDataObjCreate failed for %s, status = %d", dataObjInp.objPath, writeDataInp->l1descInx );
                return ( writeDataInp->l1descInx );
            }
        }


        // Set up input buffer for rsDataObjWrite
        bytesBuf.len = ( int )( size * nmemb );
        bytesBuf.buf = buffer;


        // Set up input struct for rsDataObjWrite
        openedDataObjInp.l1descInx = writeDataInp->l1descInx;;
        openedDataObjInp.len = bytesBuf.len;


        // Write to data object
        //written = rsDataObjWrite( writeDataInp->rsComm, &openedDataObjInp, &bytesBuf );
        written = irods::server_api_call( DATA_OBJ_WRITE_AN,  writeDataInp->rsComm, &openedDataObjInp, &bytesBuf );

        return ( written );
    }

}; 	// class irodsCurl


extern "C" {

#if 1
// =-=-=-=-=-=-=-
// 1. Write a standard issue microservice
MICROSERVICE_BEGIN(
	msiCurlGetObj,
    STR, url, INPUT,
    STR, object, INPUT,
    INT, downloaded, OUTPUT )

    dataObjInp_t destObjInp, *myDestObjInp;		// for parsing input object
    size_t transferred = 0;						// total transferred
    irods::error res = SUCCESS();
    char* my_url = url;
    char* objPath = object;

    // Sanity checks
    if ( !rei || !rei->rsComm ) {
        rodsLog( LOG_ERROR, "msiCurlGetObj: Input rei or rsComm is NULL." );
        RETURN ( SYS_INTERNAL_NULL_INPUT_ERR );
    }

    // Create irodsCurl instance
    irodsCurl myCurl( rei->rsComm );

    // Call irodsCurl::get
    res = myCurl.get( my_url, objPath, &transferred );

	// Return bytes read/written
    //fillIntInMsParam(downloaded, transferred);

    // How to return stuff?
    downloaded = (int*)&transferred;

    // Done
    RETURN ( res.code());

MICROSERVICE_END
#endif

#if 0
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
#endif


}	// extern "C"



