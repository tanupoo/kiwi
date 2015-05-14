#USE_KIWI=1 USE_KIWI_DB=1 USE_KIWI_SERVER=1 make a801n07v
#DYLD_LIBRARY_PATH=/Users/sakane/work/tools/kiwi/libmicrohttpd-0.9.38/src/microhttpd/.libs ./a801n07v -ddddd

setval()
{
	export USE_KIWI=1
	export USE_KIWI_DB=1
	export USE_KIWI_DB_SQLITE3=1
	export USE_KIWI_CODEC_XML=1
	export USE_KIWI_SUBMIT=1
	export USE_KIWI_IEEE1888=1
	export USE_KIWI_SIO=1
	export USE_KIWI_SERVER=1
	export USE_KIWI_SERVER_MHD=1
	export KIWI_MHD_DIR=../libmicrohttpd-0.9.38/src
	LDLIBS="DYLD_LIBRARY_PATH=${KIWI_MHD_DIR}/microhttpd/.libs"
}

case $1 in
a801n07v)
	PARAM="-ddd /dev/tty.usbserial-A801N07V"
	setval
	;;
sin_quick_submit)
	setval
	;;
help)
	echo "target is either:"
	echo "    a801n07v"
	echo "    sin_quick_submit"
	echo "    sin_batch_submit"
	exit 0
	;;
*)
	;;
esac

echo "Environment Variables:"
set | grep KIWI

echo "Start making:"
make $*

cmd="${LDLIBS} ./$* ${PARAM}"
echo $cmd
$cmd
