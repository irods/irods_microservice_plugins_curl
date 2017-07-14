curlDelete {
*options.'timeout' = '1000';
msiCurlDelete(*url, *options, *outStr);
writeLine("stdout", *outStr);
}
INPUT *url=$"http://www.textfiles.com/art/dragon.txt"
OUTPUT ruleExecOut
