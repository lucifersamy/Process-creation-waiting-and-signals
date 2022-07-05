hellomake: processP.c processR.c
	gcc -o processP processP.c -lm -I.
	gcc -o processR processR.c -lm -I.
