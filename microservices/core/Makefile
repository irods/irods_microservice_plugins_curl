GCC = g++ 

INC_DIR = ./include
SRC_DIR = ./src
OBJ_DIR = ./obj

INC=-I/usr/include/irods/ -I${INC_DIR}

all: lib

lib:
	${GCC} ${INC} -c -DRODS_SERVER -fPIC -g -Wno-deprecated -o ${OBJ_DIR}/irods_ms_plugin_curl.o ${SRC_DIR}/irods_ms_plugin_curl.cpp
	
clean:
	@rm -f ${OBJ_DIR}/*.o