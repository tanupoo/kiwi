#!/bin/sh

need_test=1

case $1 in
test_xbuf|test_chunk)
	;;
test_config)
	PARAMS=sample_save_to_file.conf
	;;
test_sqlite3)
	export USE_KIWI_DB_SQLITE3=1
	;;
test_submit)
	export USE_KIWI_DB_SQLITE3=1
	export USE_KIWI_SUBMIT_JSON=1
	;;
test_mhd)
	export USE_KIWI_DB_SQLITE3=1
	export KIWI_MHD_DIR=./libmicrohttpd-0.9.38/src
	LDLIBS="DYLD_LIBRARY_PATH=${KIWI_MHD_DIR}/microhttpd/.libs"
	;;
test_ieee1888)
	export USE_KIWI_CLIENT=1
	;;
help)
	echo "Usage: test.sh (test_name)"
	exit 1
	;;
*)
	need_test=0
	export USE_KIWI_CONFIG=1
	export USE_KIWI_DB_SQLITE3=1
	export USE_KIWI_CLIENT=1
	export USE_KIWI_CODEC_JSON=1
	export USE_KIWI_TRANSPORT_HTTP=1
	export KIWI_MHD_DIR=./libmicrohttpd-0.9.38/src
	;;
esac

make $*

echo ${LDLIBS} ./$* ${PARAMS}

if [ $need_test -eq 1 ] ; then
	${LDLIBS} ./$* ${PARAMS}
fi
