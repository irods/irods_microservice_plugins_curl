GCC = g++ 

INC_DIR = ../core/include
OBJ_DIR = ../core/obj

INC=-I/usr/include/irods/ -I/usr/include/irods/boost/ -I${INC_DIR}

all: libmsiCurlGetStr

libmsiCurlGetStr:
	${GCC} ${INC} -DRODS_SERVER -fPIC -shared -g -Wno-deprecated -o $@.so $@.cpp ${OBJ_DIR}/irods_ms_plugin_curl.o `curl-config --libs` /usr/lib/libirods_client.a
	
clean:
	@rm -f ./*.so
