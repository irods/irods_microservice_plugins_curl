curlDelete {
*curlOptions.'timeout' = '3000';
msiCurlDelete(*url, *curlOptions, *outStr);
writeLine("stdout", *outStr);
}
INPUT *url=$"http://www.textfiles.com/art/dragon.txt"
OUTPUT ruleExecOut
