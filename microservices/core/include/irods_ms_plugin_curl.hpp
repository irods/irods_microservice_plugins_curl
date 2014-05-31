/*
 * irods_ms_plugin_curl.hpp
 *
 *  Created on: May 23, 2014
 *      Author: adt
 */
#ifndef IRODS_MS_PLUGIN_CURL_HPP_
#define IRODS_MS_PLUGIN_CURL_HPP_

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

#define IRODS_CURL_DATA_KW		"data"
#define IRODS_CURL_HEADERS_KW	"headers"


// Input struct for curl write callback function.
// Does not go over the wire as far as iRODS is concerned.
typedef struct {
    char objPath[MAX_NAME_LEN];
    int l1descInx;
    keyValPair_t *options;
    rsComm_t *rsComm;
} writeDataInp_t;


typedef struct {
	size_t downloaded;
	size_t cutoff;	/* 0 means unlimited */
} curlProgress_t;


typedef struct {
	char *ptr;
	size_t len;	/* not counting terminating null char */
} string_t;


class irodsCurl {
private:
    // iRODS server handle
    rsComm_t *rsComm;

    // cURL handle
    CURL *curl;


public:
    // ctor
    irodsCurl( rsComm_t* );

    // dtor
    ~irodsCurl();

    irods::error get_obj( char*, keyValPair_t*, size_t* );

    irods::error get_str( char*, char** );

    irods::error post( char*, keyValPair_t*, char** );


    // Callback progress function for the curl handler
    static int progress( void*, double, double, double, double );


    // Custom callback function for the curl handler, to write to an iRODS object
    static size_t write_obj( void*, size_t, size_t, writeDataInp_t* );

    // Custom callback function for the curl handler, to write to a string
    static size_t write_str(void*, size_t, size_t, void*);

}; 	// class irodsCurl


#endif /* IRODS_MS_PLUGIN_CURL_HPP_ */
