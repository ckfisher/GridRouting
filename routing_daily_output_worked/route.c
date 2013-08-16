#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <netcdf.h>
#include "route.h"

int main(int argc, char *argv[]) {
	
	FILE *fctl, *fvic_input, *fout, *finput;

	char *fname_ctl, *fname_input, *fname_vic_input;
	char vic_path[1024], out_path[1024];
	
	int *startdate, *enddate;
	int endtime;
	int ngrid, nheadwaters;
	int *vic_row, *vic_col;
	int **vic_ts;
	int nt;
	int t, n;
	int ndays;
	int nrows, ncols, row, col;
	int max_t = 1024*2000; // TJT arbitrary - fix this
	int count;
	
	float *ll_limits;
	float *lat, *lon, *nlinks;
	float *alpha, *beta;
	float *dx;
	float *dist;
	float **back_index;
	float *vic_factor, *vic_lat, *vic_lon;
	float vic_resn;
	float ***vic_runoff;
	float ***flow_out;
	float vic_min_lat, vic_max_lat, vic_min_lon, vic_max_lon;


	/* Read in arguments */	
	if(argc!=4) {
		printf("Bad usage!\n");
		exit(4);
	}
	fname_ctl = argv[1];
	fname_input = argv[2];
	fname_vic_input = argv[3];
	
	startdate = (int*)calloc(3,sizeof(int));
	enddate = (int*)calloc(3,sizeof(int));

	// Read in control file data
	// TJT add check ot make sure control file has right number of inputs
	fctl = fopen(fname_ctl,"r");
	if (fctl==NULL) {
		printf("Couldn't open %s!\n", fname_ctl);
		exit(4);
	}
	fscanf(fctl, "%s", vic_path);
	fscanf(fctl,"%s", out_path);
	fscanf(fctl, "%d %d %d", &startdate[0], &startdate[1], &startdate[2]);
	fscanf(fctl, "%d %d %d", &enddate[0], &enddate[1], &enddate[2]);
	fscanf(fctl, "%f", &vic_resn);
	fclose(fctl);
	
	printf("Vic path: %s\n", vic_path);
	printf("Output file: %s\n", out_path);
	
	/* Get the number of grids */
	finput = fopen(fname_input, "r");
	if (finput==NULL) {
		printf("Couldn't open %s!\n", fname_input);
		exit(4);
	}
	
	fread(&ngrid, sizeof(int),1,finput);
	fclose(finput);
	//printf("Ngrids %d\n", ngrid);
	
	// Now allocate
	ll_limits = (float*)calloc(4,sizeof(float));
	lat = (float*)calloc(ngrid,sizeof(float));
	lon = (float*)calloc(ngrid,sizeof(float));	
	dx = (float*)calloc(ngrid,sizeof(float));	
	nlinks = (float*)calloc(ngrid,sizeof(float));	
	alpha = (float*)calloc(ngrid,sizeof(float));	
	beta = (float*)calloc(ngrid,sizeof(float));	
	back_index = (float**)calloc(ngrid,sizeof(float*));	
	for (n=0; n<ngrid; n++) {
		back_index[n] = (float*)calloc(8,sizeof(float));
	} 
	vic_lat = (float*)calloc(ngrid,sizeof(float));
	vic_lon = (float*)calloc(ngrid,sizeof(float));	
	vic_factor = (float*)calloc(ngrid,sizeof(float));
	vic_row = (int*)calloc(ngrid, sizeof(int));
	vic_col = (int*)calloc(ngrid, sizeof(int));
	
	
	// Read in input data
	nheadwaters = read_network_data(ll_limits, lat, lon, dx, nlinks, alpha, beta, back_index, fname_input);
	printf("network data read\n");
	
	// Read in vic input data
	read_vic_network_data(vic_factor, vic_lat, vic_lon, ngrid, fname_vic_input);
	//printf("557 %.4f %.4f %.4f\n", vic_lat[557], vic_lon[557], vic_factor[557]);
	printf("vic network info read\n");
	
	// Calculate endtime - number of seconds of the simulation
	endtime = calculate_nsecs(startdate, enddate);

	// Do some more allocating for vic data
	ndays = endtime / 86400;
	vic_min_lat = floor(ll_limits[0]);
	vic_max_lat = ceil(ll_limits[1]);
	vic_min_lon = floor(ll_limits[2]);
	vic_max_lon = ceil(ll_limits[3]);

	nrows = (int)((vic_max_lat - vic_min_lat) / vic_resn);
	ncols = (int)((vic_max_lon - vic_min_lon) / vic_resn);

	vic_runoff= (float***)calloc(nrows,sizeof(float**));
	for(row=0;row<nrows;row++) {
        vic_runoff[row] = (float**)calloc(ncols,sizeof(float*));
		for (col=0; col<ncols; col++) {
			vic_runoff[row][col] = (float*)calloc(ndays, sizeof(float));
		}
	}
	vic_ts= (int**)calloc(ndays,sizeof(int*));
	for(row=0;row<ndays;row++) {
        vic_ts[row] = (int*)calloc(4,sizeof(int));
	}
	
	// Read in VIC generated runoff
	printf("Starting to read in the VIC runoff data\n");
	read_vic_runoff(vic_runoff, vic_ts, vic_resn, endtime, startdate, enddate, ll_limits, vic_path);	
	printf("Read in the VIC runoff data\n");
	
	// Set up the associated vic rows/cols for each node
	associate_vic_network(vic_row, vic_col, lat, lon, ll_limits, ngrid, vic_resn);
	printf("Associated the vic network with links\n");
	
	flow_out= (float***)calloc(nrows,sizeof(float**));
	for(row=0;row<nrows;row++) {
        flow_out[row] = (float**)calloc(ncols,sizeof(float*));
		for (col=0; col<ncols; col++) {
			flow_out[row][col] = (float*)calloc(ndays, sizeof(float));
		}
	}
	
	
	// TJT can stop passing lat and lon this was just to debug
	printf("Routing the flow...\n");
	nt = route_flow(startdate,  endtime, alpha, beta, vic_runoff, dx, ngrid, 
			   nheadwaters, dist, nlinks, back_index, vic_factor, vic_row, vic_col, vic_ts, flow_out);
	printf("Routing done!\n");
	
	printf("Saving the output...\n");
	write_flow(flow_out, nrows, ncols, ndays, vic_min_lat, vic_min_lon, vic_resn, vic_ts, out_path);
	printf("Saving complete!\n");
	
	return(1);
	
} /* end of main function */

