curlPut {
*postFields."data" = *data
msiCurlPut(*url, *postFields, *response);
writeLine("stdout", "server response: "++*response);
}
INPUT *url="http://httpbin.org/post",*data="Sent from iRODS"
OUTPUT ruleExecOut
# See also http://requestb.in/ for quick testing of POST requests
