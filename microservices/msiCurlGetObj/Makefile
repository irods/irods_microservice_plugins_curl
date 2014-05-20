GCC = g++ 
INC=-I/usr/include/irods/

all: msiCurlGetObj

msiCurlGetObj:
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -g -Wno-deprecated -o libmsiCurlGetObj.so libmsiCurlGetObj.cpp `curl-config --libs` /usr/lib/irods/libirods_client.a
	
clean:
	@rm -f ./*.so