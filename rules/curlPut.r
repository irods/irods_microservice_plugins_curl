curlPut {
*postFields."data" = *data;
*curlOptions."timeout" = '3000'; 
msiCurlPut(*url, *postFields, *curlOptions, *response);
writeLine("stdout", "server response: "++*response);
}
INPUT *url=$"http://httpbin.org/post",*data="Sent from iRODS"
OUTPUT ruleExecOut
# See also http://requestb.in/ for quick testing of POST requests
