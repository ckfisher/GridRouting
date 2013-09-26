#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "setup_input_files.h"

void setup_output_data(float **order, float ***back_network, float **nlinks, int ngrid, char *fname_output, 
					   int min_row, int max_row, int min_col, int max_col, float *ll_limits, int nheadwaters, float **link,
					   float **mask_minflow, float **slope, float **width) {

	FILE *fout;
	
	int idx, tmp_idx, n;
	int newrow, newcol;
	int row, col;
	int P;
	int mod;
	int sub_nx, sub_ny;
	int x,y;
	int count;
	
	float lat1, lat2, lon1,lon2;
	float **index;
	
	sub_nx = max_col - min_col+1;
	sub_ny = max_row - min_row+1;

	
	index = (float**)calloc(sub_ny,sizeof(float*));
	for(col=0;col<sub_ny;col++)
		index[col] = (float*)calloc(sub_nx,sizeof(float));
	
	// first set up index array to get the number
	n=0;
	for (row=min_row; row<=max_row; row++) {
		for (col=min_col; col<max_col; col++) {
			index[row-min_row][col-min_col]=n;
			n=n+1;
		}
	}
	/*printf("Limits %d %d %d %d %d %.0f %d %d\n", min_row, max_row, min_col, max_col, n, index[1924-min_row][2020-min_col],
		   1924-min_row, 2020-min_col);
	exit; */
	
	// Set up new arrays that are just the size of the grid
	
	float **tmp_back_network;
	float *tmp_nlinks;
	float *tmp_lat;
	float *tmp_lon;
	float *tmp_dx;
	float *tmp_alpha;
	float *tmp_beta;
	
	// Allocate memory
	tmp_nlinks = (float*)calloc(ngrid,sizeof(float));
	tmp_alpha = (float*)calloc(ngrid,sizeof(float));
	tmp_beta = (float*)calloc(ngrid,sizeof(float));
	tmp_lat = (float*)calloc(ngrid,sizeof(float));
	tmp_lon = (float*)calloc(ngrid,sizeof(float));
	tmp_dx = (float*)calloc(ngrid,sizeof(float));
	tmp_back_network = (float**)calloc(ngrid,sizeof(float*));
	for(row=0;row<ngrid;row++)
        tmp_back_network[row] = (float*)calloc(8,sizeof(float));	

	
	// Organize into new arrays
	for (row=min_row+1; row<max_row-1; row++) {
		for (col=min_col+1; col<max_col-1; col++) {
			//printf("%d %d %.0f\n",row,col,order[row][col]);
			if (order[row][col]>=0) {
				idx = order[row][col];
				
				// Number of links
				tmp_nlinks[idx] = nlinks[row][col];
				
				// Latitude & longitude
				tmp_lat[idx] = 24.00416666666667 + row * RESN;
				tmp_lon[idx] = -137.995833333333 + col * RESN;
				
				// alpha
				// TJT Manning's n
				// TJT the hydraulic radius is wrong!
				P = width[row][col];
				tmp_alpha[idx] = pow ( (0.035 * pow(P, 2./3.)) / (pow(slope[row][col], 0.5)), 0.6);
				
				// beta
				tmp_beta[idx] = 0.6;
											
			}
		}
	}
	printf("Organized data into output array\n");

	for (row=min_row+1; row<max_row-1; row++) {
		for (col=min_col+1; col<max_col-1; col++) {
			//printf("%d %d %.0f\n",row,col,order[row][col]);
			if (order[row][col]>=0) {
				idx = order[row][col];
				
				// Calculate back index again
				count = 0;
				
				for (y=row-1;y<=row+1; y++) {
					for(x=col-1; x<=col+1;x++) {
						if(link[y][x] == index[row-min_row][col-min_col]) {
							// This finds the index of the back network
							tmp_back_network[idx][count] = order[y][x];
							count++;
						}
						
					}
				}
				
			}
		}
	}
	printf("Re-organized the back network array\n");
	
	// Calculate distance for each link  
	for (idx=0; idx<ngrid; idx++) {
		// If not a junction or headwater
		if (tmp_nlinks[idx]==1) {
			
			lat1 = tmp_lat[idx];
			lon1 = tmp_lon[idx];
			
			tmp_idx = tmp_back_network[idx][0];
			lat2 = tmp_lat[tmp_idx];
			lon2 = tmp_lon[tmp_idx];
			
			tmp_dx[idx] = calc_distance(lat1, lat2, lon1,lon2);
						
		}
		else if (tmp_nlinks[idx]>1) {
			// Just use the first link -- TJT think if this matters
			lat1 = tmp_lat[idx];
			lon1 = tmp_lon[idx];
			
			tmp_idx = tmp_back_network[idx][0];
			lat2 = tmp_lat[tmp_idx];
			lon2 = tmp_lon[tmp_idx];
			
			tmp_dx[idx] = calc_distance(lat1, lat2, lon1,lon2);
			
		}
		else {
			// We go forward
			lat1 = tmp_lat[idx];
			lon1 = tmp_lon[idx];
			
			tmp_idx = idx+1;
			lat2 = tmp_lat[tmp_idx];
			lon2 = tmp_lon[tmp_idx];
			
			tmp_dx[idx] = calc_distance(lat1, lat2, lon1,lon2);
			
		}
	}
	
	
	printf("Writing the output!\n");
	
	printf("%d %d\n",ngrid, nheadwaters);
	
	// Put it in the output array
	fout = fopen(fname_output, "wb");
	fwrite(&ngrid, sizeof(int),1,fout);
	for(n=0; n<4;n++) {
		fwrite(&ll_limits[n], sizeof(float),1,fout);
	}
	fwrite(&nheadwaters, sizeof(int),1,fout);
	for (n=0; n<ngrid; n++) {
		fwrite(&tmp_lat[n], sizeof(float), 1, fout);
	}
	for (n=0; n<ngrid; n++) {
		fwrite(&tmp_lon[n], sizeof(float), 1, fout);
	}
	for (n=0; n<ngrid; n++) {
		fwrite(&tmp_dx[n], sizeof(float), 1, fout);
	}
	for (n=0; n<ngrid; n++) {
		fwrite(&tmp_nlinks[n], sizeof(float), 1, fout);
	}				
	for (n=0; n<ngrid; n++) {
		fwrite(&tmp_alpha[n], sizeof(float), 1, fout);
	}
	for (n=0; n<ngrid; n++) {
		fwrite(&tmp_beta[n], sizeof(float), 1, fout);
	}
	for (idx=0; idx<ngrid; idx++) {
		for (n=0; n<8; n++) {
			fwrite(&tmp_back_network[idx][n], sizeof(float),1,fout);
		}
	}

	fclose(fout);
	
	printf("Done writing output!\n");

	/*for(col=0; col<sub_ny; col++)
		free(index[col]);
	free(index); */
	// TJT Free memory!
}