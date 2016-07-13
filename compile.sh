cwd="`pwd`"
xdir="`dirname $0`"

cd $xdir

common_files="network.c audio.c common.c"
tinyalsa_files="mixer.c pcm.c"
alsalib_libs="-lasound"

print_usage() {
	echo
	echo "  usage: cmd [alsalib | tinyalsa |   (blank for both) ]"
	echo
}

compile_alsalib() {
	# alsa-lib
	gcc $alsalib_libs -o client $common_files client.c
	gcc $alsalib_libs -o server $common_files server.c
}

compile_tinyalsa() {
	# tinyalsa
	gcc -D_TINYALSA -o servertiny $common_files $tinyalsa_files server.c
	gcc -D_TINYALSA -o clienttiny $common_files $tinyalsa_files client.c 
}

case $1 in
"")
	compile_alsalib
	compile_tinyalsa
;;
"alsalib")
	compile_alsalib
;;
"tinyalsa")
	compile_tinyalsa
;;
*)
	print_usage
;;
esac

cd $cwd
