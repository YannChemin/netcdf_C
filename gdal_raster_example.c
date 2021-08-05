#include "gdal.h"
#include <stdio.h>
#include <gdal.h>
#include "cpl_conv.h" /* for CPLMalloc() */

void usage()
{
	printf("--Modis Processing chain--Serial code----\n");
	printf("-----------------------------------------\n");
	printf("./wm inB1 inB2 inB7\n");
	printf("in250QC in500stateQA\n");
	printf("outWATER\n");
	printf("-----------------------------------------\n");
	printf("inB[1-7] files are surface reflectance files (250&500)\n");
	printf("inB1 and inB2 are Modis 250m\n");
	printf("inB3-7 are Modis 500m, they will be split to 250m\n");
	printf("in250QC is Modis 250/500m Quality Assessment\n");
	printf("in500stateQA is Modis 500m State Quality Assessment\n");
	printf("outWATER is the Water mask output [0-1]\n");
}

int main( int argc, char *argv[] )
{
	if( argc < 6 ) {
		usage();
		return 1;
	}
	int row,col;
	float ndvix, waterx;
	float temp, tempval;
	char *inB1, *inB2, *inB7;
	char *in250QC, *in500QC;
	char *waterF;
	//Loading the input files names
	inB1 = argv[1];
	inB2 = argv[2];
	inB7 = argv[3];
	in250QC = argv[4];
	in500QC = argv[5];
	waterF = argv[6];
	//Register all GDAL image drivers to memory
	GDALAllRegister();
	//Open the raster image datasets through GDAL
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly); //Red Band
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly); //NIR Band
	GDALDatasetH hD7 = GDALOpen(inB7,GA_ReadOnly); //SWIR3 Band
	GDALDatasetH hD8 = GDALOpen(in250QC,GA_ReadOnly); //QA 250m Band
	GDALDatasetH hD9 = GDALOpen(in500QC,GA_ReadOnly); //QA 500m Band
	//Fail the application if unable to load any input file
	if(hD1==NULL||hD2==NULL||hD7==NULL||hD8==NULL||hD9==NULL){
		printf("Unable to load one or more input file\n");
		exit(EXIT_FAILURE);
	}
	//Loading the file
	GDALDriverH hDr1 = GDALGetDatasetDriver(hD1);
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	GDALDriverH hDr7 = GDALGetDatasetDriver(hD7);
	GDALDriverH hDr8 = GDALGetDatasetDriver(hD8);
	GDALDriverH hDr9 = GDALGetDatasetDriver(hD9);
	//Creating output file
	//Water Mask
	GDALDatasetH hDOut0 =GDALCreateCopy(hDr1,waterF,hD1,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut0 = GDALGetRasterBand(hDOut0,1);
	//Defining the file bands and loading them
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);
	GDALRasterBandH hB7 = GDALGetRasterBand(hD7,1);
	GDALRasterBandH hB8 = GDALGetRasterBand(hD8,1);
	GDALRasterBandH hB9 = GDALGetRasterBand(hD9,1);

	//Loading Input file number 1 rows and columns
	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	//Preparing processing
	float *line1;
	//Red Band row
	float *line2;
	//NIRed Band row
	float *line7;
	//SWIR3 Band row
	int *line8;
	//Quality Assessment 250m Band row
	int *line9;
	//Quality Assessment 500m band row
	float *lineOut0; //Output Band row
	//Defining the data rows
	line1 = (float *) malloc(sizeof(float)*nX);
	line2 = (float *) malloc(sizeof(float)*nX);
	line7 = (float *) malloc(sizeof(float)*nX/2);
	line8 = (int *) malloc(sizeof(int)*nX);
	line9 = (int *) malloc(sizeof(int)*nX/2);
	lineOut0 = (float *) malloc(sizeof(float)*nX);

	//Accessing the data rowxrow
	for(row=0;row<nY;row++){
		GDALRasterIO(hB1,GF_Read,0,row,nX,1,line1,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB2,GF_Read,0,row,nX,1,line2,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB7,GF_Read,0,row/2,nX/2,1,line7,nX/2,1,GDT_Float32,0,0);
		GDALRasterIO(hB8,GF_Read,0,row,nX,1,line8,nX,1,GDT_Int32,0,0);
		GDALRasterIO(hB9,GF_Read,0,row/2,nX/2,1,line9,nX/2,1,GDT_Int32,0,0);
		//Processing the data cellxcell
		for(col=0;col<nX;col++){
			//Load temporary values with quality flags
			tempval = stateqa500a(line9[col/2]);
			temp = qc250a(line8[col]);
			//if no data or quality flag fail then assign fail values
			if(line1[col]==-28768||temp>1.0||tempval>=1.0){ /*skip it*/
				if(temp>1.0)
					lineOut0[col]=10.0;
				else if (tempval>=1.0)
					lineOut0[col]=100.0;
				else
					lineOut0[col]=-28768;
			}
			// if pixel are correct, process our algorithms
			else {
				//NDVI calculation
				ndvix = ndvi(line1[col],line2[col]);
				//Water calculation
				waterx = water_modis(line7[col/2]*0.0001,ndvix);
				lineOut0[col] = waterx;
			}
		}
		//Write image to disk through GDAL
		GDALRasterIO(hBOut0,GF_Write,0,row,nX,1,lineOut0,nX,1,GDT_Float32,0,0);
	}
	if( line1 != NULL ) free( line1 );
	if( line2 != NULL ) free( line2 );
	if( line7 != NULL ) free( line7 );
	if( line8 != NULL ) free( line8 );
	if( line9 != NULL ) free( line9 );
	if( lineOut0 != NULL ) free( lineOut0 );
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD7);
	GDALClose(hD8);
	GDALClose(hD9);
	GDALClose(hDOut0);
}
