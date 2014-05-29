curlPost {
*postFields."data" = *data
msiCurlPost(*url, *postFields, *response);
writeLine("stdout", "server response: "++*response);
}
INPUT *url="http://requestb.in/vf9fl2vf",*data="blah"
OUTPUT ruleExecOut
