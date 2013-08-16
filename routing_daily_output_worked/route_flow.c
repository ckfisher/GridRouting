#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <netcdf.h>
#include "route.h"


int route_flow(int *startdate, int endtime, float *alpha, float *beta, 
				float ***runoff, float *dx, int ngrid, int nheadwaters, float *dist, float *nlinks, float **back_index, 
				float *vic_factor, int *vic_row, int *vic_col, int **vic_ts, float ***flow_out) {	
	FILE *fout;
	
	int t;
	int time;
	int n, x;
	int vic_current=0;
	int vic_old=0;
	int back;
	int row, col;
	int vic_tidx;
	int nt;
	int ndays;
	int year, month, day;
	int dayflag;
	
	float dt;
	float c_k;
	float v_old[ngrid], v[ngrid], Q_old[ngrid], Q[ngrid];
	float xarea[ngrid];
	float term1, term2, term3, denom;
	float dt_total;

	float Q_ts[4];
	float Q_ts_old[4];
	float Q_mean[ngrid];
	float Q_mean_old[ngrid];
	
	// Open up output file
/*	fout = fopen(output_file,"w");
	if (fout==NULL) {
		printf("Couldn't open output file %s, exiting.\n", output_file);
		exit(4);
	}*/

	// TJT For now just does one dimension for Q_outlet - fix once can read multiple points. Also time needs fixing
	//time = (int*)calloc(1024*10, sizeof(int));
	
	time = 0;
	t=0;
	ndays = endtime / 86400;
	printf("ngrid %d\n", ngrid);

	
	// TJT - need to set up for initial conditions
	for (n=0; n<ngrid; n++) {
		Q[n] = 5;
		v[n] = 2;
	}

	// Put a check that not using missing VIC data 
	for (n=0; n<ngrid; n++) {
		row = vic_row[n];
		col = vic_col[n];
		if (runoff[row][col][0]==-999000000) {
			printf("Missing VIC data at grid %d\n", n);
			exit(4);
		}
	}
		
	
	// Set up array for time - year, month, day, second
	Q_ts_old[0] = startdate[0];
	Q_ts_old[1] = startdate[1];
	Q_ts_old[2] = startdate[2];
	Q_ts_old[3] = 1;
	vic_current = 0;
	vic_tidx = 0;
	
	// Initialize Q_mean
	for (n=0; n<ngrid; n++) {
		Q_mean[n] = 0;
		Q_mean_old[n] = 0;
	}
	
	
	// TJT: I forced an hour less because for some reason was looping through till the next day after when it was supposed to end
	dt=0;
	dt_total = 0;
	dayflag = 1;
	while(time < endtime-3600) {
		t++;

		
		/* Save last time step */
		for (n=0; n<=ngrid; n++) {
			v_old[n] = v[n];
			Q_old[n] = Q[n];
			
			v[n] = 0;
			Q[n] = 0;
		}
			
		/* Calculate the Courant condition */
		// TJT should fix it so that have dx's at junctions
		dt = 3600;
		for (n=nheadwaters; n<=ngrid; n++) {
			if (nlinks[n]==1) {
				c_k = 5. / 3. * v_old[n];
				if ( (dx[n] / c_k) < dt) {
					dt = dx[n] / c_k;
				}
			}
		}
		dt = floor(dt);
		
		// Keep track of time
		time = time + (int)dt;
		if (time>endtime) {
			break;
		}
	
		// Handle the time series
		// See if we are still the same day
		if( Q_ts_old[3]+dt <86400) {
			//printf("in here!\n");
			Q_ts[0] = Q_ts_old[0];
			Q_ts[1] = Q_ts_old[1];
			Q_ts[2] = Q_ts_old[2];
			Q_ts[3] = Q_ts_old[3] + dt;
			
			dayflag = 1;
			dt_total = dt_total + dt;

		}
		// See if same month
		else if( Q_ts_old[2]+1<=eomday(Q_ts_old[0], Q_ts_old[1]) ) {
			Q_ts[0] = Q_ts_old[0];
			Q_ts[1] = Q_ts_old[1];
			Q_ts[2] = Q_ts_old[2]+ 1;
			Q_ts[3] = Q_ts_old[3] + dt - 86400;
			
			dayflag = 0;
		}
		// See if in same year
		else if( Q_ts_old[1] + 1 <= 12) {
			Q_ts[0] = Q_ts_old[0];
			Q_ts[1] = Q_ts_old[1]+1;
			Q_ts[2] = 1;
			Q_ts[3] = Q_ts_old[3] + dt - 86400;
			
			dayflag = 0;
		}
		// If not increment everything
		else {
			Q_ts[0] = Q_ts_old[0] + 1;
			Q_ts[1] = 1;
			Q_ts[2] = 1;
			Q_ts[3] = Q_ts_old[3] + dt - 86400;
			
			dayflag = 0;
		}
		//printf("Flag %d %.0f\n", dayflag, dt_total);
		
		// Initialize some stuff if dayflag==0
		if (dayflag==0) {
			for (n=0; n<ngrid; n++) {
				Q_mean_old[n] = Q_mean[n] / dt_total;
				Q_mean[n] = 0;
			}
			dt_total = dt;
		}
		
		// Save Q_ts in old
		Q_ts_old[0] = Q_ts[0];
		Q_ts_old[1] = Q_ts[1];
		Q_ts_old[2] = Q_ts[2];
		Q_ts_old[3] = Q_ts[3];
	
		// Find the vic index for where we are - this assumes daily runoff
		vic_old = vic_current;
		vic_tidx = vic_old;
 
/*		while (1){
			year=(int)Q_ts[0];
			month=(int)Q_ts[1];
			day=(int)Q_ts[2];
                        
			if(vic_ts[vic_tidx][0]==Q_ts[0] && vic_ts[vic_tidx][1]==Q_ts[1] && vic_ts[vic_tidx][2]==Q_ts[2]) {
				vic_current = vic_tidx;
				break;
			}
			else {
				vic_tidx++;
			}
		}
	
		if (vic_tidx>=ndays) {
			break;
		} */

	
		for(n=0; n<ngrid; n++) {
		
			row = vic_row[n];
			col = vic_col[n];
			
			
			// If it's a headwater, the boundary condition is just VIC flow
			if(nlinks[n]==0) {
				Q[n] = runoff[row][col][vic_current] * vic_factor[n] / 86400 * dx[n]; 

				xarea[n] = alpha[n] * pow( Q[n], beta[n]);
				v[n] = Q[n] / xarea[n];
			}
				
			// If it's not a junction, we route the flow
			else if(nlinks[n]==1) {
			
				// Equation 9.6.7 in Chow 1988
				back = back_index[n][0];
				term1 = (dt/dx[n]) * Q[back];
				term2 = alpha[n] * beta[n] * Q_old[n] * pow( 0.5 * ( Q_old[n] + Q[back] ), (beta[n]-1));
				term3 = dt * 0.5 * ( runoff[row][col][vic_current] + runoff[row][col][vic_old] ) * vic_factor[n] /86400;
				
				denom = dt / dx[n] + alpha[n] * beta[n] * pow(0.5 * ( Q_old[n]+Q[back] ), (beta[n]-1));
				
				// Calculate streamflow
				Q[n] = (term1 + term2 + term3) / denom;
			
				// Calculate x-sectional area from that
				xarea[n] = alpha[n] * pow(Q[n], beta[n]);
				v[n] = Q[n] / xarea[n];
			}
			else {

				// We do continuity
				for (x=0; x<nlinks[n]; x++) {
					back = back_index[n][x];
					Q[n] = Q[n] + Q[back];
				}
				// And add in runoff contribution
				Q[n] = Q[n] + runoff[row][col][vic_current]* vic_factor[n] /86400 * dx[n];
			
				// Calculate x-sectional area from that
				xarea[n] = alpha[n] * pow(Q[n], beta[n]);
				v[n] = Q[n] / xarea[n];
			}
			
			// If it's a new day, set Q_mean to Q*dt and save the old, else add it up
			if (dayflag==0) {
				/*if (n==0) {
					//printf("New Day %.0f %.0f\n ",Q_mean[n] / dt_total, dt_total);
					fprintf(fout, "%.0f/%.0f/%.0f ", Q_ts_old[0], Q_ts_old[1], Q_ts_old[2]);
									//printf("dt total %f\n",dt_total);
				}

				fprintf(fout,"%.2f ",Q_mean_old[n]);

				
				if (n==ngrid-1) {
					fprintf(fout,"\n");
				}*/
				
				flow_out[row][col][vic_current] = Q_mean_old[n];				
						
				dt_total = dt;
				Q_mean[n] = Q[n] * dt;
			}
			else {
				Q_mean[n] = Q_mean[n] + Q[n]*dt;

			}		

		}
		
		if (dayflag==0) { 
				vic_ts[vic_current][0]=Q_ts_old[0];
				vic_ts[vic_current][1]=Q_ts_old[1]; 
				vic_ts[vic_current][2]=Q_ts_old[2];
				vic_current++;
		}

	} /* end of time while loop */
	
	//fclose(fout);
	
	return nt;
	

} /* End of route_flow */

