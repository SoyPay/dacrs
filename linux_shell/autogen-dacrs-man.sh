#!/bin/sh
set -e

if [ $# = 0 ]; then	
  	echo -e "\033[40;33m"
		echo warming  your had not inputed assemble model
		echo "$PWD"
		echo autogen-dacrs-man [MODEL NAME]
		echo
		echo	EXAMPLE:
		echo
		echo	autogen-dacrs-man ["dacrs-d|dacrs-cli|dacrs-test|dacrs-ptest"]
	echo -e "\033[40;37m"
		exit 1
elif [ $# = 1 ]; then
	case $1 in 
		dacrs-d)
		flag1=--with-daemon
		;;
		dacrs-cli)
		flag1=--with-cli
		;;
		dacrs-test)
		flag1=--enable-tests
		;;
		dacrs-ptest)
		flag1=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
elif [ $# = 2 ]; then
	case $1 in 
		dacrs-d)
		flag1=--with-daemon
		;;
		dacrs-cli)
		flag1=--with-cli
		;;
		dacrs-test)
		flag1=--enable-tests
		;;
		dacrs-ptest)
		flag1=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
	case $2 in 
		dacrs-d)
		flag2=--with-daemon
		;;
		dacrs-cli)
		flag2=--with-cli
		;;
		dacrs-test)
		flag2=--enable-tests
		;;
		dacrs-ptest)
		flag2=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
elif [ $# = 3 ]; then
	case $1 in 
		dacrs-d)
		flag1=--with-daemon
		;;
		dacrs-cli)
		flag1=--with-cli
		;;
		dacrs-test)
		flag1=--enable-tests
		;;
		dacrs-ptest)
		flag1=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
	case $2 in 
		dacrs-d)
		flag2=--with-daemon
		;;
		dacrs-cli)
		flag2=--with-cli
		;;
		dacrs-test)
		flag2=--enable-tests
		;;
		dacrs-ptest)
		flag2=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
	case $3 in 
		dacrs-d)
		flag3=--with-daemon
		;;
		dacrs-cli)
		flag3=--with-cli
		;;
		dacrs-test)
		flag3=--enable-tests
		;;
		dacrs-ptest)
		flag3=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
elif [ $# = 4 ]; then
	case $1 in 
		dacrs-d)
		flag1=--with-daemon
		;;
		dacrs-cli)
		flag1=--with-cli
		;;
		dacrs-test)
		flag1=--enable-tests
		;;
		dacrs-ptest)
		flag1=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
	case $2 in 
		dacrs-d)
		flag2=--with-daemon
		;;
		dacrs-cli)
		flag2=--with-cli
		;;
		dacrs-test)
		flag2=--enable-tests
		;;
		dacrs-ptest)
		flag2=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
	case $3 in 
		dacrs-d)
		flag3=--with-daemon
		;;
		dacrs-cli)
		flag3=--with-cli
		;;
		dacrs-test)
		flag3=--enable-tests
		;;
		dacrs-ptest)
		flag3=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
	case $4 in 
		dacrs-d)
		flag4=--with-daemon
		;;
		dacrs-cli)
		flag4=--with-cli
		;;
		dacrs-test)
		flag4=--enable-tests
		;;
		dacrs-ptest)
		flag4=--enable-ptests
		;;
		*)
		echo -e "\033[40;32m"
		echo warming:error para!
		echo -e "\033[40;37m"
		exit 1
		;;
	esac
else
	echo -e "\033[40;32m"
	echo warming  your had inputed illegal params
   	echo please insure the params in [dacrs-d|dacrs-cli|dacrs-test|dacrs-ptest]
	echo -e "\033[40;37m" 
	exit 1
fi

srcdir="$(dirname $0)"
cd "$srcdir"
autoreconf --install --force

CPPFLAGS="-I/c/deps/boost_1_55_0 \
-I/c/deps/db-4.8.30.NC/build_unix \
-I/c/deps/openssl-1.0.1g/include \
-I/c/deps \
-I/c/deps/protobuf-2.5.0/src \
-I/c/deps/libpng-1.6.9 \
-I/c/deps/qrencode-3.4.3 \
-std=c++11 \
-DHAVE_CONFIG_H" \
LDFLAGS="-L/c/deps/boost_1_55_0/stage/lib \
-L/c/deps/db-4.8.30.NC/build_unix \
-L/c/deps/openssl-1.0.1g \
-L/c/deps/miniupnpc \
-L/c/deps/protobuf-2.5.0/src/.libs \
-L/c/deps/libpng-1.6.9/.libs \
-L/c/deps/qrencode-3.4.3/.libs" \
./configure \
--disable-upnp-default \
--enable-debug \
--without-gui \
$flag1 \
$flag2 \
$flag3 \
$flag4 \
--with-protoc-bindir=/c/deps/protobuf-2.5.0/src \
--with-incompatible-bdb
