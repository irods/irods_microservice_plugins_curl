/*
 * libmsiCurlDelete.cpp
 *
 *  Created on: 12 June 2017 
 *      Author: Hurng-Chun Lee <h.lee@donders.ru.nl>
 */
#include "irods_ms_plugin_curl.hpp"

int msiCurlDelete(msParam_t* msp_in_str_url, msParam_t* msp_out_str_response, ruleExecInfo_t* rei) {

    if (rei == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlDelete: input rei is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }

    if (msp_in_str_url == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlDelete: msp_in_str_url is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }
    if (msp_in_str_url->type == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlDelete: msp_in_str_url->type is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }
    if (strcmp(msp_in_str_url->type, STR_MS_T)) {
        rodsLog(LOG_ERROR, "msiCurlDelete: first argument should be STR_MS_T, was [%s]", msp_in_str_url->type);
        return USER_PARAM_TYPE_ERR;
    }

    if (msp_out_str_response == nullptr) {
        rodsLog(LOG_ERROR, "msiCurlDelete: msp_out_str_response is NULL");
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }

    irodsCurl myCurl(rei->rsComm);
    msp_out_str_response->type = strdup(STR_MS_T);
    irods::error res = myCurl.del(static_cast<char*>(msp_in_str_url->inOutStruct), reinterpret_cast<char**>(&msp_out_str_response->inOutStruct));
    return res.code();
}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(2);
    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>("msiCurlDelete",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(msiCurlDelete));
    return msvc;
}
