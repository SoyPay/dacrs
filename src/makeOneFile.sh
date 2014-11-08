#!/bin/sh

if [ $# == 0 ]; then	
  	echo -e "\033[40;33m"
		echo warming  your had not inputed assemble file
   		echo useing the deflaut folder $Folder 
		echo "$PWD"
		echo MakeShell [FILE NAME]
		echo
		echo	EXAMPLE:
		echo
		echo	MakeShell main.cpp
	echo -e "\033[40;37m"
		exit 1
	
	
else
	echo -e "\033[40;32m"
	if [ -f $1 ]; then	  
		echo compile Start ...
		echo compiling $1.
		gcc -c -std=c++11 $1 -I/c/deps/boost_1_55_0 -I/c/deps/db-4.8.30.NC/build_unix -I/c/deps/openssl-1.0.1g/include \
			-I"$PWD"/leveldb/include -I"$PWD"/leveldb/helpers/memenv  -I"$PWD"/lotto -I"$PWD" \
			-I/c/deps/boost_1_55_0/boost/thread -DHAVE_CONFIG_H\
			-I/c/deps -I/c/deps/protobuf-2.5.0/src -I/c/deps/libpng-1.6.9 -I/c/deps/qrencode-3.4.3 -DBOOST_SPIRIT_THREADSAFE -DHAVE_BUILD_INFO \
			-D__STDC_FORMAT_MACROS -D_MT -DWIN32 -D_WINDOWS -DBOOST_THREAD_USE_LIB -D_FILE_OFFSET_BITS=64  -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -DSTATICLIB \
			-g3 -O0 -DDEBUG -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter -w  -Wstack-protector -fstack-protector-all -fPIE

		temp_var=$?
			if [ $temp_var != 0 ]; then
			echo -e "\033[40;31m"
			echo "MAKE ERROR" 
			echo -e "\033[40;37m"	
        fi		
		echo compiling End.
	else
		echo File $1 does not exists
	fi
	echo -e "\033[40;37m" 
fi





