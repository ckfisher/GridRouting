#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <netcdf.h>
#include "route.h"

void read_vic_runoff(float ***vic_runoff, int **vic_ts, float vic_resn, int endtime, int *startdate, int *enddate, 
					 float *ll_limits, char *vic_path) {
	
	FILE *fvic;
	
	char filename[1024];
	char dummy[1024];
	
	int year, month, day;
	int nrows, ncols;
	int ndays;
	int row, col;
	int t;
	int start_idx, end_idx;
	int flag;
	
	float runoff;
	float vic_min_lat, vic_max_lat, vic_min_lon, vic_max_lon;
	float lat, lon;
	
	//exit(4);
	
	// Set up the lat/lon limits and grid size
	vic_min_lat = floor(ll_limits[0]);
	vic_max_lat = ceil(ll_limits[1]);
	vic_min_lon = floor(ll_limits[2]);
	vic_max_lon = ceil(ll_limits[3]);
	
	nrows = (int)((vic_max_lat - vic_min_lat) / vic_resn);
	ncols = (int)((vic_max_lon - vic_min_lon) / vic_resn);
        
	// Read in the VIC data
	// TJT This all assumes it's daily for now
	flag=1;
	for (row=0; row<nrows; row++) {		
		for (col=0; col<ncols; col++) {
			//printf("%d %d\n", row, col);
			
			// Only do it if the vic_factor is greater than zero (otherwise it's not in the basin)
			
			lat = vic_min_lat + row * vic_resn + vic_resn / 2;
			lon = vic_min_lon + col * vic_resn + vic_resn / 2;
			//printf("%d %d %.4f %.4f\n", row, col, lat, lon);
			
			// Make filename
			sprintf(filename, "%s/dailyflux_%.4f_%.4f", vic_path, lat, lon);
			
			// Open file - this assumes vic has been post-processed into year, month, day, total runoff
			fvic = fopen(filename, "r");
			if (fvic==NULL) {
				vic_runoff[row][col][0]=-999;
				printf("Couldn't open %s\n", filename);
				/* exit(4); */
			}
			else {
			
			// Read it in - for first file, get the start and end index. For others skip to the start and verify it's when we want
				if (flag==1) {
					flag = 0;
					start_idx = 0;
					while (1) {
						fscanf(fvic,"%d %d %d %f", &year, &month, &day, &runoff);
						if ( year==startdate[0] && month==startdate[1] && day==startdate[2] ) {
							vic_runoff[row][col][0] = runoff;
							vic_ts[0][0] = year;
							vic_ts[0][1] = month;
							vic_ts[0][2] = day;
							break;
						}
						else {
							start_idx++;
						}
					
						if (year==EOF) {
							// TJT I don't think this'll catch it properly - should test
							printf("Couldn't find the start date in VIC before EOF\n");
						
							exit(4);
						}
					}
					
					t=1;
					end_idx = start_idx;
					while (1) {
						fscanf(fvic,"%d %d %d %f", &year, &month, &day, &runoff);
						vic_runoff[row][col][t] = runoff;
						vic_ts[t][0] = year;
						vic_ts[t][1] = month;
						vic_ts[t][2] = day;		
						t++;
					
					// Read in the rest of the data
						if ( year==enddate[0] && month==enddate[1] && day==enddate[2] ) {
				
							break;
						}
						else {
							end_idx++;
						}
						if (year==EOF) {
							printf("Couldn't find the end date in VIC before EOF\n");
							exit(4);
						}					
					}

				}	/* End of if statement if first row & col */
				else {
				
					// Skip to first day
					for (t=0; t<start_idx; t++) {
						fgets(dummy, 1024, fvic);
					}
					// Check it's the right start time
					fscanf(fvic,"%d %d %d %f", &year, &month, &day, &runoff);
					//printf("%d %d %d %d\n", start_idx, year, month,day);
					if ( year!=startdate[0] && month!=startdate[1] && day!=startdate[2] ) {
						printf("Wrong start time for %s\n", filename);
						exit(4);
					}
					else {
						vic_runoff[row][col][0] = runoff;
					}

					for (t=start_idx+1; t<=end_idx; t++) {
						fscanf(fvic,"%d %d %d %f", &year, &month, &day, &runoff);
						vic_runoff[row][col][t-start_idx] = runoff;

					}
				}
				fclose(fvic);
 
			}
		}
	}
	


} /* End of read_vic_runoff */

