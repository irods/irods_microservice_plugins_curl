/*
 * libmsiCurlPut.cpp
 *
 *  Created on: 12 June 2017 
 *      Author: Hurng-Chun Lee <h.lee@donders.ru.nl>
 */
#include "irods_ms_plugin_curl.hpp"

int msiCurlPut(msParam_t* msp_in_str_url, msParam_t* msp_in_kvp_options, msParam_t* msp_out_str_response, ruleExecInfo_t* rei) {

    if (rei == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlPut: input rei is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }

    if (msp_in_str_url == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlPut: msp_in_str_url is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }
    if (msp_in_str_url->type == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlPut: msp_in_str_url->type is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }
    if (strcmp(msp_in_str_url->type, STR_MS_T)) {
        rodsLog(LOG_ERROR, "msiCurlPut: first argument should be STR_MS_T, was [%s]", msp_in_str_url->type);
        return USER_PARAM_TYPE_ERR;
    }

    if (msp_in_kvp_options == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlPut: msp_in_kvp_options is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }
    if (msp_in_kvp_options->type == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlPut: msp_in_kvp_options->type is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }
    if (strcmp(msp_in_kvp_options->type, KeyValPair_MS_T)) {
        rodsLog(LOG_ERROR, "msiCurlPut: second argument should be KeyValPair_MS_T, was [%s]", msp_in_kvp_options->type);
        return USER_PARAM_TYPE_ERR;
    }

    if (msp_out_str_response == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlPut: msp_out_str_response is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }

    irodsCurl myCurl(rei->rsComm);
    msp_out_str_response->type = strdup(STR_MS_T);
    irods::error res = myCurl.put(static_cast<char*>(msp_in_str_url->inOutStruct), static_cast<keyValPair_t*>(msp_in_kvp_options->inOutStruct), reinterpret_cast<char**>(&msp_out_str_response->inOutStruct));
    return res.code();
}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(3);
    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>("msiCurlPut",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(msiCurlPut));
    return msvc;
}
