becsmos: becsmos.c
	gcc -o becsmos becsmos.c -lm -I/usr/include/gdal -L/usr/lib -lgdal

clean:
	rm -rf becsmos
