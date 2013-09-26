#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "setup_input_files.h"


// TJT: This is for the north america data - should be modified down the road for any continent. 
// Or just make five of these files ;)


int main(int argc, char *argv[]) {

	// Define variables
	FILE *fdir, *facc, *fout, *fout2;
	
	char *fname_dir, *fname_flowacc, *fname_elev, *fname_output, *fname_output2;
	
	int x, y;
	int row, col;
	int count;
	int ngrid;
	
	int mouth_row, mouth_col, temp_row, temp_col;
	int min_row, max_row, min_col, max_col;
	int nheadwaters;
	int flow_thresh;
	
	float mouth_lat;
	float mouth_lon;
	float *ll_limits;
	float max_accum;
	float **direc;
	float **link;
	float **flowacc;
	float **mask;
	float **mask_minflow;
	float **nlinks;
	float ***back_network;
	float **order;
	float **dx;
	float lat, lon;
	float **elev;
	float **slope;
	float **width;
	
	/* Read in arguments */	// TJT fix the usage error statement
	if(argc!=9) {
		printf("Bad usage!\n");
		exit(4);
	}
	fname_dir = argv[1];
	fname_flowacc = argv[2];
	fname_elev = argv[3];
	fname_output = argv[4];
	fname_output2 = argv[5];
	flow_thresh = atoi(argv[6]);
	mouth_lat = atof(argv[7]);
	mouth_lon = atof(argv[8]);
	
	// Check that can open output file
	fout = fopen(fname_output, "w");
	if(fout==NULL) {
		printf("Couldn't open output file!\n");
		exit(4);
	}
	else {
		fclose(fout);
	}

	
	// Allocate memory
	direc= (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        direc[row] = (float*)calloc(NX,sizeof(float));
	link = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        link[row] = (float*)calloc(NX,sizeof(float)); 
	elev = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        elev[row] = (float*)calloc(NX,sizeof(float));
	slope = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        slope[row] = (float*)calloc(NX,sizeof(float));
	width = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        width[row] = (float*)calloc(NX,sizeof(float));
	flowacc = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        flowacc[row] = (float*)calloc(NX,sizeof(float));	
	mask = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        mask[row] = (float*)calloc(NX,sizeof(float));	
	mask_minflow = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        mask_minflow[row] = (float*)calloc(NX,sizeof(float));	
	ll_limits = (float*)calloc(4,sizeof(float));
	nlinks = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        nlinks[row] = (float*)calloc(NX,sizeof(float));
	order = (float**)calloc(NY,sizeof(float*));
	for(row=0;row<NY;row++)
        order[row] = (float*)calloc(NX,sizeof(float));
	back_network = (float***)calloc(NY,sizeof(float**));
	for(row=0;row<NY;row++) {
        back_network[row] = (float**)calloc(NX,sizeof(float*));	
		for(col=0; col<NX;col++) {
			back_network[row][col] = (float*)calloc(8,sizeof(float));
		}
	}			
	printf("Memory allocated\n");
	
	// Read in direction file
	read_direction(fname_dir, direc);
	printf("Direction file read\n");
	
	// Read in flow accumulation
	read_flowaccum(fname_flowacc, flowacc);
	printf("Flow accumulation file read\n");
	
	// Read in elevation data
	read_elevation(fname_elev, elev);
	printf("Elevation file read\n");
	
	
	// Find basin outlet
	mouth_row = (int)((mouth_lat - 24.00416666667)/RESN ); // TJT fix so uses set values
	mouth_col = (int)((mouth_lon - (-137.99583333333))/RESN); 
	max_accum = flowacc[mouth_row][mouth_col];
		
	temp_row = mouth_row;
	temp_col = mouth_col;
	for (y=mouth_row-5; y<= mouth_row+5; y++) {
		for (x=mouth_col-5; x<= mouth_col+5; x++) {
			if(flowacc[y][x]>max_accum) {
				temp_row = y;
				temp_col = x;
				max_accum = flowacc[y][x];
			}
		}
	}
	mouth_row = temp_row;
	mouth_col = temp_col; 
	
	// Subset basin
	ngrid = subset_basin(mask, mask_minflow, flowacc, ll_limits, direc, flow_thresh, mouth_row, mouth_col);
	for (x=0;x<4;x++)
		printf("%f\n", ll_limits[x]);
	printf("Basin subset: Number of points %d\n", ngrid);

	// Identify the rows we're going to use (so not working on the whole continent for a subbasin
	min_row = (int)((ll_limits[0] - 24.00416666666667) / RESN)-5;
	max_row = (int)((ll_limits[1] - 24.00416666666667) / RESN)+5;
	min_col = (int)((ll_limits[2] - (-137.995833333333)) / RESN)-5;
	max_col = (int)((ll_limits[3] - (-137.995833333333)) / RESN)+5;
	
	// Make links TJT Think about moving this to after we subset the basin
	assemble_network(direc,link, mask_minflow, min_row, max_row, min_col, max_col);
	printf("Network assembled\n");	
	
	// Calculate the back_netowrk
	calc_back_network(link, back_network, mask_minflow, min_row, max_row, min_col, max_col);
	printf("Back network assembled\n");
	
	// Set up the order we're going to solve everything 
	nheadwaters = calc_solving_order(order, link, nlinks, mask_minflow, back_network, min_row, max_row, min_col, max_col);
	printf("Calculated the order to solve everything\n");
	printf("%d\n",nheadwaters);
	
	// Calculate the slope of each stream pixel
	calc_slope(slope, elev, link, nlinks, mask_minflow, back_network, min_row, max_row, min_col, max_col);
	
	// Calculate river width
	calc_width(width, flowacc, mask_minflow, min_row, max_row, min_col, max_col);

	// Identify inflow points (if it's a downstream basin)
	// TJT ignore this for now - this should be included once we can subset out downstream basins, as the code 
	// is now can only do everything upstream.
	
					  
	// Save everything into one gridded, binary file
	setup_output_data(order, back_network, nlinks, ngrid, fname_output,min_row, max_row, min_col, max_col, 
					  ll_limits, nheadwaters, link, mask_minflow, slope, width);
	
	// Save another file with the lat/long info of all points in the river basin (not just those of a certain flow accumulation).
	// This will be used to calculate the area of each VIC grid cell
	fout2 = fopen(fname_output2, "w");
	if(fout==NULL) {
		printf("Couldn't open %s!\n", fname_output2);
		exit(4);
	}
	for (row=min_row; row<=max_row; row++) {
		for (col=min_col; col<=max_col; col++) {
			if (mask[row][col]==1) {
				lat = 24.00416666667 + row * RESN;
				lon = -137.99583333333 + col * RESN;
				fprintf(fout2,"%12.4f %12.4f %8.0f %12.8f\n", lat,lon, mask_minflow[row][col],slope[row][col]);
			}
		}
	}
	fclose(fout2);
	
	// TJT Free memory
	/* for(row=0;row<NY;row++) {
		free(direc[row]);
		free(link[row]);
		free(flowacc[row]);
		free(mask[row]);
		free(mask_minflow[row]);
		free(nlinks[row]);
		free(order[row]);
		for(col=0; col<NX;col++) {
			free(back_network[row][col]);
		}
		free(back_network[row]);
	}
	free(direc);
	free(link);
	free(flowacc);
	free(mask);
	free(mask_minflow);
	free(nlinks);
	free(order);
	free(back_network);
	free(ll_limits);	*/
	
	
	return(1);


}