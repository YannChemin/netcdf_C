becsmos: 0_0_open.c
	gcc -o becsmos 0_0_open.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp

clean:
	rm -rf becsmos
