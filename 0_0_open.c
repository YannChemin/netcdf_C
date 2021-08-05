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

int main( int argc, char *argv[] )
{
	if( argc < 1 ) {
		usage();
		return(EXIT_FAILURE);
	}
	int row,col;
	float smosx;
	//Register all GDAL image drivers to memory
	GDALAllRegister();
	//Loading the input files names
	GDALDatasetH hD[MAXFILES];
	GDALDatasetH hDQC[MAXFILES];
	//for ( int x = 1; x < (argc/2); x++ )
	for ( int x = 1; x < argc; x++ )
    	{
		hD[x] = GDALOpen(argv[x],GA_ReadOnly); //Read Band
		//Fail the application if unable to load any input file
		if(hD[x]==NULL){
			printf("Unable to load one or more input file\n");
			exit(EXIT_FAILURE);
		}
    	}
	//for ( int x = (argc/2); x < argc; x++ )
    	//{
	//	hDQC[x] = GDALOpen(argv[x],GA_ReadOnly); //Read QC
	//	//Fail the application if unable to load any input file
	//	if(hDQC[x]==NULL){
	//		printf("Unable to load one or more input file\n");
	//		exit(EXIT_FAILURE);
	//	}
    	//}
	//Loading the files drivers
	GDALDriverH hDr = GDALGetDatasetDriver(hD[0]);
	//GDALDriverH hDrQC = GDALGetDatasetDriver(hDQC[0]);

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
	//Defining the file bands and loading them
	GDALRasterBandH hB[MAXFILES];
	GDALRasterBandH hBQC[MAXFILES];
	for ( int x = 1; x < argc; x++ )
    	{
		hB[x] = GDALGetRasterBand(hD[x],1);
		//hBQC[x] = GDALGetRasterBand(hDQC[x],1);
		//Loading Input file number 1 rows and columns
		int nX = GDALGetRasterBandXSize(hB[x]);
		int nY = GDALGetRasterBandYSize(hB[x]);
		//Preparing processing
		float *lineB;
		//SWIR3 Band row
		int *lineQ;
		//Defining the data rows
		lineB = (float *) malloc(sizeof(float)*nX);
		lineQ = (int *) malloc(sizeof(int)*nX);
		//Accessing the data rowxrow
		for(row=0 ; row<nY ; row++){
			GDALRasterIO(hB[x],GF_Read,0,row,nX,1,lineB,nX,1,GDT_Float32,0,0);
			//GDALRasterIO(hBQC[x],GF_Read,0,row,nX,1,lineQ,nX,1,GDT_Int32,0,0);
			//Processing the data cellxcell
			for(col=0 ; col<nX ; col++){
				//if no data or quality flag fail 
				//then assign fail values
				if(lineB[col]==-28768){ /*skip it*/
					smosx = -28768;
				}
				// if pixel are correct, process our algorithms
				else {
					//Create Meteo array
					smosx = lineB[col];
				}
			}
			//Write to meteo array
			printf("Write to meteo array %f\n", smosx );
		}
		if( lineB != NULL ) free( lineB );
		if( lineQ != NULL ) free( lineQ );
		GDALClose(hD);
		GDALClose(hDQC);
	}
}
