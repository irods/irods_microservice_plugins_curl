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
	char *ptr;
	size_t len;	/* not counting terminating null char */
} string_t;


typedef struct {
	char objPath[MAX_NAME_LEN];
	int l1descInx;
	rsComm_t *rsComm;
} writeDataInp_t;


typedef struct {
	size_t downloaded;
	size_t cutoff;	/* 0 means unlimited */
} curlProgress_t;


/* Callback progress function for the curl handler */
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


/* Custom callback function for the curl handler, to write to an iRODS object */
static size_t createAndWriteToDataObj(void *buffer, size_t size, size_t nmemb, void *stream)
{
	writeDataInp_t *writeDataInp;	/* the "file descriptor" for our destination object */
	dataObjInp_t dataObjInp;	/* input structure for rsDataObjCreate */
	openedDataObjInp_t openedDataObjInp;	/* input structure for rsDataObjWrite */
	bytesBuf_t bytesBuf;	/* input buffer for rsDataObjWrite */
	size_t written;	/* output value */


	/* retrieve writeDataInp_t input */
	writeDataInp = (writeDataInp_t *)stream;


	/* to avoid unpleasant surprises */
	memset(&dataObjInp, 0, sizeof(dataObjInp_t));
	memset(&openedDataObjInp, 0, sizeof(openedDataObjInp_t));


	/* If this is the first call we need to create our data object before writing to it */
	if (writeDataInp && !writeDataInp->l1descInx)
	{
		strcpy(dataObjInp.objPath, writeDataInp->objPath);
		writeDataInp->l1descInx = rsDataObjCreate(writeDataInp->rsComm, &dataObjInp);

		/* problem? */
		if (writeDataInp->l1descInx <= 2)
		{
			rodsLog (LOG_ERROR, "createAndWriteToDataObj: rsDataObjCreate failed for %s, status = %d", dataObjInp.objPath, writeDataInp->l1descInx);
			return (writeDataInp->l1descInx);
		}
	}


	/* set up input buffer for rsDataObjWrite */
	bytesBuf.len = (int)(size * nmemb);
	bytesBuf.buf = buffer;


	/* set up input data structure for rsDataObjWrite */
	openedDataObjInp.l1descInx = writeDataInp->l1descInx;
	openedDataObjInp.len = bytesBuf.len;


	/* write to data object */
	written = rsDataObjWrite(writeDataInp->rsComm, &openedDataObjInp, &bytesBuf);

	return (written);
}


int msiCurlGetObj(msParam_t *url, msParam_t *object, msParam_t *downloaded, ruleExecInfo_t *rei) {
	CURL *curl;	/* curl handler */
	CURLcode res;
	char *my_url;

	dataObjInp_t destObjInp, *myDestObjInp;	/* for parsing input object */

	writeDataInp_t writeDataInp;	/* custom file descriptor for our callback function */
	openedDataObjInp_t openedDataObjInp;	/* to close iRODS object after writing */

	curlProgress_t prog;	/* for progress and cutoff */


	/* For testing mode when used with irule --test */
	RE_TEST_MACRO (" Calling msiCurlGetObj")

	/* Sanity checks */
	if (!rei || !rei->rsComm) {
		rodsLog (LOG_ERROR, "msiCurlGetObj: Input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* Pad data structures with null chars */
	memset(&writeDataInp, 0, sizeof(writeDataInp_t));
	memset(&openedDataObjInp, 0, sizeof(openedDataObjInp_t));


	/* Check url input */
	my_url = parseMspForStr(url);
	if (!my_url || !strlen(my_url)) {
		rodsLog (LOG_ERROR, "msiCurlGetObj: Null or empty url input.");
		return (USER_INPUT_STRING_ERR);
	}


	/* Get path of destination object */
	rei->status = parseMspForDataObjInp (object, &destObjInp, &myDestObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiCurlGetObj: Input object error. status = %d", rei->status);
		return (rei->status);
	}


	/* Set up writeDataInp */
	strcpy(writeDataInp.objPath, destObjInp.objPath);
	writeDataInp.l1descInx = 0; /* the object is yet to be created */
	writeDataInp.rsComm = rei->rsComm;


	/* Progress struct init */
	prog.downloaded = 0;
	prog.cutoff = 0;


	/* CURL easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		/* Set up easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, my_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, createAndWriteToDataObj);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeDataInp);

		/* Progress settings */
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

		/* CURL call */
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		res = curl_easy_perform(curl);

		/* Cleanup curl handler */
		curl_easy_cleanup(curl);
	}
	else {
		res = CURLE_FAILED_INIT;
	}

	/* Some error logging */
	if(res != CURLE_OK) {
		rodsLog (LOG_ERROR, "msiCurlGetObj: cURL error: %s", curl_easy_strerror(res));
	}

	/* CURL cleanup before returning */
	curl_global_cleanup();

	/* close iRODS object */
	if (writeDataInp.l1descInx)
	{
		openedDataObjInp.l1descInx = writeDataInp.l1descInx;
		rei->status = rsDataObjClose(rei->rsComm, &openedDataObjInp);
		if (rei->status < 0) {
			rodsLog (LOG_ERROR, "msiCurlGetObj: rsDataObjClose failed for %s, status = %d",
					writeDataInp.objPath, rei->status);
		}
	}

	/* Return bytes read/written */
	fillIntInMsParam(downloaded, prog.downloaded);

	/* Cleanup and done */
	return 0;
}

