cwd="`pwd`"
xdir="`dirname $0`"

cd $xdir

common_files="network.c audio.c common.c"
tinyalsa_files="mixer.c pcm.c"
alsalib_libs="-lasound"

case $1 in
"alsa-lib")
	# alsa-lib
	gcc $alsalib_libs -o client $common_files client.c
	gcc $alsalib_libs -o server $common_files server.c
;;
"tinyalsa")
	# tinyalsa
	gcc -D_TINYALSA -o servertiny $common_files $tinyalsa_files server.c
	gcc -D_TINYALSA -o clienttiny $common_files $tinyalsa_files client.c 
;;
*)
	echo
	echo "  usage: cmd [alsa-lib | tinyalsa]"
	echo
;;
esac

cd $cwd
