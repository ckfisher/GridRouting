#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "route.h"

int eomday(int year, int month) {

	int DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int i;
	int mod;
		
	i=DaysInMonth[month-1];
	
	/* Check for leap year */
	if (month==2) {
		mod = year % 4;
		if (mod==0) {
			i = 29;
		}
	}
		
	return i;
		
} /* End of eomday function */

int calculate_nsecs(int *startdate, int *enddate) {

	int year, month, day;
	int ndays;
	int nsecs;
	/* Do checks that the enddates are after the startdates! */
	if (enddate[0] < startdate[0]) {
		printf("End year of simulation is earlier than the start year!\n");
		exit(4);
	}

	if (enddate[0] == startdate[0]) {
		if (enddate[1]<startdate[1]) {
			printf("Month of simulation start is after the end month!\n");
			exit(4);
		}
	}

	if (enddate[0] == startdate[0] && enddate[1] == startdate[1]) {
		if (enddate[2]<startdate[2]) {
			printf("Day of simulation start is after the end month!\n");
			exit(4);
		}
	}

	/* If less than a year of simulation */
	if (startdate[0]== enddate[0]) {
		
		if (startdate[1]==enddate[1]) {
			ndays = enddate[2] - startdate[2] + 1;
		}
		else {
			ndays = eomday(startdate[0], startdate[1]) - startdate[2]+1;
			for (month=startdate[1]+1; month<=enddate[1]-1; month++) {
				ndays = ndays + eomday(startdate[0], month);
			}
			ndays = ndays + enddate[2];
		}
	}

	else {

		// Do first year
		ndays = eomday(startdate[0], startdate[1]) - startdate[2]+1;
		for (month=startdate[1]+1; month<=12; month++) {
			ndays = ndays + eomday(startdate[0], month);
		}		
		// Do middle years
		for (year=startdate[0]+1; year<=enddate[0]-1; year++) {
			for (month=1; month<=12; month++) {
				ndays = ndays + eomday(year, month);
			}							
		}
		// Do last year
		for (month=1; month<=enddate[1]-1; month++) {
			ndays = ndays + eomday(enddate[0], month);
		}		
		ndays = ndays + enddate[2];
	}
	

	nsecs = ndays * 86400;
	
	return(nsecs);
	
} /* End of calculate_nsecs */

int read_network_data(float *ll_limits, float *lat, 
					   float *lon, float *dx, float *nlinks, float *alpha, float *beta, 
					   float **back_index, char *fname_input ) {
	
	FILE *finput;
	
	int n, idx;
	int ngrid;
	int nheadwaters;
	
	
	// Open up file with network properties
	finput = fopen(fname_input, "r");
	if (finput==NULL) {
		printf("Couldn't open %s!\n", fname_input);
		exit(4);
	}
	
	fread(&ngrid, sizeof(int),1,finput);

	// Read in the data
	for(n=0; n<4;n++) {
		fread(&ll_limits[n], sizeof(float),1,finput);
	}
	fread(&nheadwaters, sizeof(int),1,finput);
	for (n=0; n<ngrid; n++) {
		fread(&lat[n], sizeof(float), 1, finput);
	}
	for (n=0; n<ngrid; n++) {
		fread(&lon[n], sizeof(float), 1, finput);
	}
	for (n=0; n<ngrid; n++) {
		fread(&dx[n], sizeof(float), 1, finput);
	}
	for (n=0; n<ngrid; n++) {
		fread(&nlinks[n], sizeof(float), 1, finput);
	}				
	for (n=0; n<ngrid; n++) {
		fread(&alpha[n], sizeof(float), 1, finput);
	}
	for (n=0; n<ngrid; n++) {
		fread(&beta[n], sizeof(float), 1, finput);
	}
	for (idx=0; idx<ngrid; idx++) {
		for (n=0; n<8; n++) {
			fread(&back_index[idx][n], sizeof(float),1,finput);
		}
	}

	fclose(finput);
	
	return nheadwaters;
	
} /* end of read_network_data */

void read_vic_network_data(float *vic_factor, float *vic_lat, float *vic_lon, int ngrid, char *fname_vic_input) {
	
	FILE *fvic_input;
	
	int n;
	
	fvic_input = fopen(fname_vic_input, "r");
	if (fvic_input==NULL) {
		printf("Couldn't open %s!\n", fname_vic_input);
		exit(4);
	}


	for (n=0; n<ngrid; n++) {
		fread(&vic_factor[n], sizeof(float),1, fvic_input);
	}
	for (n=0; n<ngrid; n++) {
		fread(&vic_lat[n], sizeof(float),1, fvic_input);
	}
	for (n=0; n<ngrid; n++) {
		fread(&vic_lon[n], sizeof(float),1, fvic_input);
	}
	fclose(fvic_input);
	
	/*for(n=0; n<ngrid; n++) {
		printf("%d %.4f %.4f %.4f\n", n, vic_lat[n], vic_lon[n], vic_factor[n]);
	} */

	
} /* end of read_vic_network_data */

void associate_vic_network(int *vic_row, int *vic_col, float *lat, float *lon, float *ll_limits, int ngrid, float vic_resn) {

	
	// TJT this whole thing can be done in teh vic setup routines. 
	int n;
	int nrows, ncols;
	int row, col;
	
	float vic_min_lat, vic_max_lat, vic_min_lon, vic_max_lon;
	float lat1,lat2, lon1,lon2;


	// Set up the lat/lon limits and grid size
	vic_min_lat = floor(ll_limits[0]);
	vic_max_lat = ceil(ll_limits[1]);
	vic_min_lon = floor(ll_limits[2]);
	vic_max_lon = ceil(ll_limits[3]);
	
	nrows = (vic_max_lat - vic_min_lat) / vic_resn;
	ncols = (vic_max_lon - vic_min_lon) / vic_resn;
	
	for (n=0; n<ngrid; n++) {
		for (row=0; row<nrows; row++) {
			for (col=0; col<ncols; col++) {
			
				lat1 = vic_min_lat + (row+1) * vic_resn;
				lat2 = vic_min_lat + row * vic_resn;
			
				lon1 = vic_min_lon + (col+1) * vic_resn;
				lon2 = vic_min_lon + col * vic_resn;
				//printf("%.4f %.4f %.4f\t\t%.4f %.4f %.4f\n",lat1,lat2,lat[n],lon1,lon2, lon[n]);	
				if (lat[n] >= lat2 && lat[n]<=lat1 && lon[n]>=lon2 && lon[n]<=lon1) {
					vic_row[n] = row;
					vic_col[n] = col;
					//printf("%d %d\n", row, col);
				}
			}			
			
		}
	}

} /* end of associate_vic_network */

