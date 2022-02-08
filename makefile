executables = tarc tarx

all: clean tarc tarx

tarc: tarc.c
	gcc -g -Werror -Wall -MD -std=gnu99 -I/home/cosc360/libfdr/include/ -o tarc tarc.c /home/cosc360/libfdr/lib/libfdr.a

tarx: tarx.c
	gcc -g -Werror -Wall -MD -std=gnu99 -I/home/cosc360/libfdr/include/ -o tarx tarx.c /home/cosc360/libfdr/lib/libfdr.a

clean: 
	rm -f ${executables} *.o