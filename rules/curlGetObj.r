curlGetObj {
# as per rodsKeyWdDef.hpp
*options."objPath" = *path;
*options."forceFlag" = "1";
msiCurlGetObj(*url, *options, *written);
writeLine("stdout", str(*written)++" bytes written");
}
INPUT *url="http://www.textfiles.com/art/ferrari.art",*path="/tempZone/home/public/ferrari.art"
OUTPUT ruleExecOut
