#include "gdal.h"
#include <stdio.h>
#include <gdal.h>
#include "cpl_conv.h" /* for CPLMalloc() */

#define MAXFILES 1000

void usage()
{
	printf("--BECSMOS Reading data --Serial code----\n");
	printf("-----------------------------------------\n");
	printf("./becsmos out inB1,[inB2,...] \n");
	printf("inQC1,[inQC2,...]\n");
	printf("-----------------------------------------\n");
	printf("inB files are BECSMOS files (1000)\n");
	printf("inQC is BECSMOS Quality Assessment\n");
}

//double pixel_to_world(geo_matrix, x, y){
//	double ul_x = geo_matrix[0];
//	double ul_y = geo_matrix[3];
//	double x_dist = geo_matrix[1];
//	double y_dist = geo_matrix[5];
//	double lon = col * x_dist + ul_x;
//	double lat = row * y_dist + ul_y;
//	return (lon, lat);
//}
int main( int argc, char *argv[] )
{
	if( argc < 1 ) {
		usage();
		return(EXIT_FAILURE);
	}
	int row,col;
	float smosx;
	double scale_factor=9.9999997e-05;
	//Register all GDAL image drivers to memory
	GDALAllRegister();
	//Loading the input files names
	GDALDatasetH hD[MAXFILES];
	GDALDatasetH hDQC[MAXFILES];
	for ( int x = 0; x < ((argc-1)/2)+1; x++ )
    	{
		printf("%d/%d : %s\n", x, argc, argv[x]);
	}
	for ( int x = ((argc-1)/2)+1; x < argc; x++ )
    	{
		printf("%d/%d : %s\n", x, argc, argv[x]);
	}
	for ( int x = 1; x < ((argc-1)/2)+1; x++ )
    	{
		printf("%d : %s\n", x, argv[x]);
		hD[x-1] = GDALOpen(argv[x],GA_ReadOnly); //Read Band
		//Fail the application if unable to load any input file
		if(hD[x-1]==NULL){
			printf("Unable to load input file %s\n",argv[x]);
			exit(EXIT_FAILURE);
		}
    	}
	for ( int x = ((argc-1)/2)+1; x < argc; x++ )
    	{
		printf("%d : %s\n", x, argv[x]);
		int xc = x-((argc-1)/2)-1;
		printf("Counter for hDQC is at %d\n",xc);
		hDQC[xc] = GDALOpen(argv[x],GA_ReadOnly); //Read QC
		//Fail the application if unable to load any input file
		if(hDQC[xc]==NULL){
			printf("Unable to load input file %s\n",argv[x]);
			exit(EXIT_FAILURE);
		}
    	}
	//Loading the files drivers
	GDALDriverH hDr = GDALGetDatasetDriver(hD[0]);
	printf("Passed 1\n");

	//Creating output file
	double        adfGeoT[6];
	printf( "Driver: %s/%s\n",
	        GDALGetDriverShortName( hDr ),
	        GDALGetDriverLongName( hDr ) );
	printf( "Size is %dx%dx%d\n",
	        GDALGetRasterXSize( hD[0] ),
	        GDALGetRasterYSize( hD[0] ),
	        GDALGetRasterCount( hD[0] ) );
	if( GDALGetProjectionRef( hD[0] ) != NULL )
	    printf( "Projection is `%s'\n", GDALGetProjectionRef( hD[0] ) );
	if( GDALGetGeoTransform( hD[0], adfGeoT ) == CE_None )
	{
	    printf( "Origin = (%.6f,%.6f)\n",adfGeoT[0], adfGeoT[3] );
	    printf( "Pixel Size = (%.6f,%.6f)\n",adfGeoT[1], adfGeoT[5] );
	}
	double ul_x = adfGeoT[0];
	double ul_y = adfGeoT[3];
	double x_dist = adfGeoT[1];
	double y_dist = adfGeoT[5];
	//double lon = col * x_dist + ul_x;
	//double lat = row * y_dist + ul_y;

	//Loading the files drivers
	GDALDriverH hDrQC = GDALGetDatasetDriver(hDQC[0]);
	printf("Passed 2\n");

	//Creating output file
	double        adfGeoTQC[6];
	printf( "Driver: %s/%s\n",
	        GDALGetDriverShortName( hDrQC ),
	        GDALGetDriverLongName( hDrQC ) );
	printf( "Size is %dx%dx%d\n",
	        GDALGetRasterXSize( hDQC[0] ),
	        GDALGetRasterYSize( hDQC[0] ),
	        GDALGetRasterCount( hDQC[0] ) );
	if( GDALGetProjectionRef( hDQC[0] ) != NULL )
	    printf( "Projection is `%s'\n", GDALGetProjectionRef( hDQC[0] ) );
	if( GDALGetGeoTransform( hDQC[0], adfGeoTQC ) == CE_None )
	{
	    printf( "Origin = (%.6f,%.6f)\n",adfGeoTQC[0], adfGeoTQC[3] );
	    printf( "Pixel Size = (%.6f,%.6f)\n",adfGeoTQC[1], adfGeoTQC[5] );
	}

	//Defining the file bands and loading them
	GDALRasterBandH hB[MAXFILES];
	GDALRasterBandH hBQC[MAXFILES];
	printf("<><><><><>%d",(argc/2));
	for ( int x = 0; x < (argc/2); x++ )
    	{
		hB[x] = GDALGetRasterBand(hD[x],1);
		hBQC[x] = GDALGetRasterBand(hDQC[x],1);
		//Loading Input file number 1 rows and columns
		int nX = GDALGetRasterBandXSize(hB[x]);
		int nY = GDALGetRasterBandYSize(hB[x]);
		//Preparing processing
		float *lineB;
		//QC Band row
		int *lineQ;
		//Defining the data rows
		lineB = (float *) malloc(sizeof(float)*nX);
		lineQ = (int *) malloc(sizeof(int)*nX);
		//Accessing the data rowxrow
		for(row = 0 ; row < nY ; row++){
			GDALRasterIO(hB[x],GF_Read,0,row,nX,1,lineB,nX,1,GDT_Float32,0,0);
			GDALRasterIO(hBQC[x],GF_Read,0,row,nX,1,lineQ,nX,1,GDT_Int32,0,0);
			//Processing the data cellxcell
			for(col = 0 ; col < nX ; col++){
				double lon = col * x_dist + ul_x;
				double lat = row * y_dist + ul_y;
				printf("\nWrite Long/Lat: %f %f \t",lon,lat);
				//if no data or quality flag fail skip it
				if(lineB[col]!=-999.0 && lineQ[col]==0){
					// if pixel are correct, process our algorithms
					// process the correction to m3/m3
					smosx = lineB[col] * scale_factor;
					// Write to meteo array
					printf(" %0.4f\t", smosx );
				}
			}
		}
		if( lineB != NULL ) free( lineB );
		if( lineQ != NULL ) free( lineQ );
		GDALClose(hD);
		GDALClose(hDQC);
	}
}
