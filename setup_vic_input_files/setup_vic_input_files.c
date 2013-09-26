#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "setup_vic_input_files.h"


// TJT: This is for the north america data - should be modified down the road for any continent. 
// Or just make five of these files ;) ALSO NOT SURE THIS DOESN'T ONLY WORK IN THE WESTERN HALF OF THE HEMISPHERE
// BECAUSE OF NEGATIVE LONGITUDES


int main(int argc, char *argv[]) {

	// Define variables
	FILE *finfo, *finfo2, *fout;
	
	char *fname_info, *fname_info2, *fname_output;
	
	int ngrid;
	int nrows, ncols;
	int row, col;
	int n;
	int nlines;
	
	float dummy;
	float ll_limits[4];
	float *lat, *lon, *dx;
	float vic_resn;
	float *vic_factor, *vic_lat, *vic_lon;
	float *basin_lat, *basin_lon;
	float basin_pixel;
	
	float vic_min_lat, vic_max_lat, vic_min_lon, vic_max_lon;
	float **density;
	float **area;
	float lat1, lat2,lat3, lon1,lon2,lon3;
	float dist1, dist2;
	float total_area;
	
	float npixels;
	
	/* Read in arguments */	// TJT improve the usage error statement
	if(argc!=6) {
		printf("Bad usage!\n");
		exit(4);
	}
	fname_info = argv[1];
	fname_info2	=argv[2];
	fname_output = argv[3];
	vic_resn = atof(argv[4]);	
	nlines = atoi(argv[5]);
	
	// Read in the network info file
	finfo = fopen(fname_info, "r");
	if (finfo==NULL) {
		printf("Couldn't open %s\n", fname_info);
		exit(4);
	}
	fread(&ngrid, sizeof(int),1,finfo);	
	for(n=0; n<4;n++) {
		fread(&ll_limits[n], sizeof(float),1,finfo);
	}	
	fread(&dummy, sizeof(float),1, finfo);
	printf("ngrid %d\n", ngrid);
	
	// Calculate the maximum number of pixels in a grid cell
	npixels = pow( floor(vic_resn / 0.0083333333333333), 2);
	
	// Allocate memory needed
	lat = (float*)calloc(ngrid,sizeof(float));
	lon = (float*)calloc(ngrid,sizeof(float));	
	dx = (float*)calloc(ngrid,sizeof(float));	
	
	// Read in the lat/lon info
	for (n=0; n<ngrid; n++) {
		fread(&lat[n], sizeof(float), 1, finfo);
	}
	for (n=0; n<ngrid; n++) {
		fread(&lon[n], sizeof(float), 1, finfo);
	}
	for (n=0; n<ngrid; n++) {
		fread(&dx[n], sizeof(float), 1, finfo);
	}
	fclose(finfo);
	
	// Read in the list of lat/longs in the basin - this will be used to calculate the grid cell area
	basin_lat = (float*)calloc(nlines,sizeof(float));
	basin_lon = (float*)calloc(nlines,sizeof(float));	
	finfo2 = fopen(fname_info2,"r");
	if(finfo2==NULL) {
		printf("Couldn't open %s\n", fname_info);
		exit(4);
	}
	
	for (n=0; n<nlines; n++) {
		fscanf(finfo2, "%f %f %f %f", &basin_lat[n], &basin_lon[n], &dummy, &dummy);
		//printf("%d %.4f %.4f\n", n, basin_lat[n],basin_lon[n]);
	}
	fclose(finfo2);
	
	// Calculate the VIC limits - this could be more precise, will have some added rows/columns
	vic_min_lat = floor(ll_limits[0]);
	vic_max_lat = ceil(ll_limits[1]);
	vic_min_lon = floor(ll_limits[2]);
	vic_max_lon = ceil(ll_limits[3]);
	
	nrows = (vic_max_lat - vic_min_lat) / vic_resn;
	ncols = (vic_max_lon - vic_min_lon) / vic_resn;
	
	// Allocate density and area arrays
	density= (float**)calloc(nrows,sizeof(float*));
	for(row=0;row<nrows;row++)
        density[row] = (float*)calloc(ncols,sizeof(float));
	area= (float**)calloc(nrows,sizeof(float*));
	for(row=0;row<nrows;row++)
        area[row] = (float*)calloc(ncols,sizeof(float));
	
	// Calculate area & density for each VIC grid cell
	total_area = 0;
	for (row=0; row<nrows; row++) {
		for (col=0; col<ncols; col++) {
			
			lat1 = vic_min_lat + (row+1) * vic_resn;
			lat2 = vic_min_lat + row * vic_resn;
			lat3 = vic_min_lat + row * vic_resn + vic_resn / 2;
			
			lon1 = vic_min_lon + (col+1) * vic_resn;
			lon2 = vic_min_lon + col * vic_resn;
			lon3 = vic_min_lon + col * vic_resn + vic_resn / 2;
						
			// Calculate area
			dist1 = calc_distance(lat1, lat2, lon3, lon3);
			dist2 = calc_distance(lat3, lat3, lon1, lon2);
			area[row][col] = dist1 * dist2;
			
			// Calculate density
			for (n=0; n<ngrid; n++) {
				if (lat[n] >= lat2 && lat[n]<=lat1 && lon[n]>=lon2 && lon[n]<=lon1) {
					density[row][col]++;
				}
			}

			// Calculate the fractional area 
			basin_pixel = 0;
			for (n=0; n<nlines; n++) {
				if (basin_lat[n] >= lat2 && basin_lat[n]<=lat1 && basin_lon[n]>=lon2 && basin_lon[n]<=lon1) {
					basin_pixel++;
					
					//printf("%.0f %d %.4f %.4f %.4f %.4f %.4f %.4f\n", basin_pixel, n, lat2, basin_lat[n],lat1,lon2,basin_lon[n],lon1);
				}
			}
			// Adjust by the number of basin pixels in the grid cell 
			// Ensure that have at least 1% in the basin
			//printf(" %d  %d  %.4f %.1f %.1f\n", row, col, area[row][col], basin_pixel, npixels);
			area[row][col] = area[row][col] * basin_pixel / npixels;
			total_area = total_area + area[row][col];
				//printf("%.4f %.4f %.4f %.0f %.0f\n", lat3, lon3, area[row][col], basin_pixel, npixels);

			
			
		}
	}
	printf("Area: %.0f\n", total_area/ 1000 /1000 /1.7/1.7);


	// Calculate the factor for each stream reach/link
	vic_factor = (float*)calloc(ngrid,sizeof(float));	
	vic_lat = (float*)calloc(ngrid,sizeof(float));	
	vic_lon = (float*)calloc(ngrid,sizeof(float));	
	
	for (n=0; n<ngrid; n++) {
		vic_lat[n] = -999.;
		vic_lon[n] = -999.;
	}
		
	for (row=0; row<nrows; row++) {
		for (col=0; col<ncols; col++) {
			lat1 = vic_min_lat + (row+1) * vic_resn;
			lat2 = vic_min_lat + row * vic_resn;
			
			lon1 = vic_min_lon + (col+1) * vic_resn;
			lon2 = vic_min_lon + col * vic_resn;
			for (n=0; n<ngrid; n++) {
				//printf("%d %.4f %.4f %.4f %.4f %.4f %.4f\n", n, lat[n], lat2, lat1, lon[n],lon2,lon1);
				if (lat[n] >= lat2 && lat[n]<=lat1 && lon[n]>=lon2 && lon[n]<=lon1) {
					vic_factor[n] = area[row][col] / 1000 / density[row][col] / dx[n];
					//printf("%d %.4f %.4f %.4f %.4f %.4f %.4f\n",n,lat[n],lon[n], area[row][col], density[row][col], dx[n], vic_factor[n]);
					vic_lat[n] = lat2 + vic_resn/2;
					vic_lon[n] = lon2 + vic_resn/2;
				}
			
			}
		}
	}

	
	// Output the data
	fout = fopen(fname_output,"w");
	if(fout==NULL) {
		printf("Couldn't open %s!\n", fname_output);
		exit(4);
	}
	for (n=0; n<ngrid; n++) {
		fwrite(&vic_factor[n], sizeof(float),1, fout);
	}
	for (n=0; n<ngrid; n++) {
		fwrite(&vic_lat[n], sizeof(float),1, fout);
	}
	for (n=0; n<ngrid; n++) {
		fwrite(&vic_lon[n], sizeof(float),1, fout);
	}
	fclose(fout);
	/*for (n=0; n<ngrid; n++) {
		printf("%d %.4f %.4f %.4f %.4f %.4f\n", n, vic_factor[n], vic_lat[n], vic_lon[n], lat[n], lon[n]);
	} */
	
	// Free memory
	free(lat);
	free(lon);
	free(dx);
	for(row=0;row<nrows;row++) {
		free(density[row]);
		free(area[row]);
	}
	free(density);
	free(area);
	free(vic_factor);
	free(vic_lat);
	free(vic_lon);
	
	
	return(1);


}
