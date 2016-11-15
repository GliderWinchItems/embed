if [ -f "$1" ]; then
	export pc="pod_compress"
	rm -f $pc
	gcc -o $pc $pc.c ./libz.a
	./$pc "$1"
	exit 0
else
    echo "Can't open input file: \"$1\""
    exit 1
fi
