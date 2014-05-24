/*
 * irods_ms_plugin_curl.cpp
 *
 *  Created on: May 23, 2014
 *      Author: adt
 */

// =-=-=-=-=-=-=-
#include "irods_ms_plugin_curl.hpp"


irodsCurl::irodsCurl( rsComm_t *comm ) {
	rsComm = comm;

	curl = curl_easy_init();
	if ( !curl ) {
		rodsLog( LOG_ERROR, "irodsCurl: %s", curl_easy_strerror( CURLE_FAILED_INIT ) );
	}
}

irodsCurl::~irodsCurl() {
	if ( curl ) {
		curl_easy_cleanup( curl );
	}
}

irods::error irodsCurl::get_obj( char *url, char *objPath, size_t *transferred ) {
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
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, &irodsCurl::write_obj );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, &writeDataInp );
	curl_easy_setopt( curl, CURLOPT_URL, url );

	// Progress settings
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &irodsCurl::progress);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

	// CURL call
	res = curl_easy_perform( curl );

	// Some error logging
	if ( res != CURLE_OK ) {
		rodsLog( LOG_ERROR, "irodsCurl::get_obj: cURL error: %s", curl_easy_strerror( res ) );
	}

	// close iRODS object
	if ( writeDataInp.l1descInx ) {
		openedDataObjInp.l1descInx = writeDataInp.l1descInx;

		//status = rsDataObjClose( rsComm, &openedDataObjInp );
		status = irods::server_api_call( DATA_OBJ_CLOSE_AN, rsComm, &openedDataObjInp );

		if ( status < 0 ) {
			rodsLog( LOG_ERROR, "irodsCurl::get_obj: rsDataObjClose failed for %s, status = %d",
					writeDataInp.objPath, status );
		}
	}

	// log total transferred
	*transferred = prog.downloaded;

	return CODE(res);
}


irods::error irodsCurl::get_str( char *url, char **buffer ) {
	CURLcode res = CURLE_OK;
	string_t string;
	curlProgress_t prog;	// for progress and cutoff
	int status;


	// Destination string_t init
	string.ptr = strdup("");
	string.len = 0;

	// Progress struct init
	prog.downloaded = 0;
	prog.cutoff = 0;

	// Set up easy handler
	curl_easy_setopt( curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, &irodsCurl::write_str );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, &string );
	curl_easy_setopt( curl, CURLOPT_URL, url );

	// Progress settings
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &irodsCurl::progress);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

	// CURL call
	res = curl_easy_perform( curl );

	// Some error logging
	if ( res != CURLE_OK ) {
		rodsLog( LOG_ERROR, "irodsCurl::get_str: cURL error: %s", curl_easy_strerror( res ) );
	}

	// Output
	*buffer = string.ptr;

	return CODE(res);
}


// Callback progress function for the curl handler
int irodsCurl::progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow) {
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
size_t irodsCurl::write_obj( void *buffer, size_t size, size_t nmemb, writeDataInp_t *writeDataInp ) {
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


// Custom callback function for the curl handler, to write to a string
size_t irodsCurl::write_str(void *ptr, size_t size, size_t nmeb, void *stream)
{
	size_t newLen;
	string_t *string;
	void *tmpPtr;

	if (!stream) {
		rodsLog (LOG_ERROR, "%s", "write_string() error. NULL destination stream.");
		return 0;
	}

	string = (string_t *)stream;

	newLen = string->len + size*nmeb;

	/* Reallocate memory with space for new content */
	/* Add an extra byte for terminating null char */
	tmpPtr = realloc(string->ptr, newLen + 1);
	if (!tmpPtr) {
		rodsLog(LOG_ERROR, "%s", "write_string(): realloc failed.");
		return -1;
	}
	string->ptr = (char*)tmpPtr;


	/* Append new content to string and terminating '\0' */
	memcpy(string->ptr + string->len, ptr, size*nmeb);
	string->ptr[newLen] = '\0';
	string->len = newLen;

	return size*nmeb;

}



