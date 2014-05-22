GCC = g++ 
INC=-I/usr/include/irods/

all: msiCurlGetStr

msiCurlGetStr:
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -g -Wno-deprecated -o libmsiCurlGetStr.so libmsiCurlGetStr.cpp `curl-config --libs` /usr/lib/irods/libirods_client.a
	
clean:
	@rm -f ./*.so