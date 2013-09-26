#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "setup_input_files.h"

#define PI 3.1415265 

void read_flowaccum(char *filename, float **data) {
	FILE *fin;
	int row, col;

	unsigned int temp;
	fin = fopen(filename,"r");
	if(fin==NULL) {
		printf("Couldn't open %s!\n",filename);
		exit(4);
	}

	for (row=NY-1;row>=0; row--) {
	//for(row=0; row<NY; row++) {
		for (col=0; col<NX; col++) {
			fread(&temp, sizeof(unsigned int), 1, fin);
			data[row][col] = (float)temp;
		}
	}

	fclose(fin);
}

void read_direction(char *filename, float **data) {
	FILE *fin;
	int row, col;
	
	unsigned char temp;
	
	fin = fopen(filename,"r");
	if(fin==NULL) {
		printf("Couldn't open %s!\n",filename);
		exit(4);
	}
	
	for (row=NY-1;row>=0; row--) {
	//for(row=0; row<NY; row++) {
		for (col=0; col<NX; col++) {
			fread(&temp, sizeof(unsigned char), 1, fin);
			if(temp>128)
				data[row][col] = (float)temp - 256;
			else
				data[row][col] = (float)temp;
			
		}
	}
	
	fclose(fin);
}

void read_elevation(char *filename, float **data) {
	FILE *fin;
	int row, col;
	
	int temp;
	
	fin = fopen(filename,"r");
	if(fin==NULL) {
		printf("Couldn't open %s!\n",filename);
		exit(4);
	}
	
	for (row=NY-1;row>=0; row--) {
		//for(row=0; row<NY; row++) {
		for (col=0; col<NX; col++) {
			//fread(&temp, sizeof(int), 1, fin);
			fread(&data[row][col], sizeof(float), 1, fin);
			//data[row][col] = (float)temp;
			//printf("%d %d %.4f\n",row,col, data[row][col]);	 
			
		}
	}
	
	fclose(fin);
}


void reshape_matrix(float **direc, float *direc_vec) {

  int n;
  int row, col;

  /* reshape direction matrix */
  n=0;
  for(row=0; row<NY; row++) {
    for(col=0; col<NX; col++) {
      direc_vec[n] = direc[row][col];
      n++;
    }
  }

}



int subset_basin(float **mask, float **mask_minflow, float **flowacc, float *ll_limits, float **direc, int flow_thresh, int mouth_row, int mouth_col) {
	
	// TJT - this needs to be screened for flow accumulation too
	
	int flag;
	int row, col;
	int loop;
	int ngrid;
	
	int x,y;
	
	float lat, lon;
	
	// Here's how it's going to go - the ones that are in that haven't been checked yet will be a 2, 
	// once we've looked for all the cells around, change it to a 1 and it's done. When all 1's, change the flag
	// to break the loop
	
	// Start at the mouth 
	mask[mouth_row][mouth_col]=3;
	loop = 0;
	while(1) {
		flag=0;
		loop++;
		
		for(row=0; row<NY; row++) {
			for (col=0; col<NX; col++) {
				if(mask[row][col]==3) {
					flag = flag+1;
					
					if(direc[row-1][col] == 64) 
						mask[row-1][col] = 2;
					if(direc[row-1][col-1] == 128)
						mask[row-1][col-1] = 2;
					if(direc[row][col-1] ==1) 
						mask[row][col-1] = 2;
					if(direc[row+1][col-1] == 2)
						mask[row+1][col-1] = 2;
					if(direc[row+1][col] == 4)
						mask[row+1][col] = 2;
					if(direc[row+1][col+1]==8)
						mask[row+1][col+1] = 2;
					if(direc[row][col+1]==16)
						mask[row][col+1] = 2;
					if(direc[row-1][col+1]==32)
						mask[row-1][col+1] = 2; 
				
					mask[row][col] = 1;
					
				}
			}
		}

		
		// Now set all the new flags (2) to (3) so that it does it next round		
		for(row=0; row<NY; row++) {
			for (col=0; col<NX; col++) {
				if(mask[row][col]==2) {
					mask[row][col]=3;
				}
			}
		}
		
		// If the flag is zero, then that means all the grid cells are done, break the while loop
		if(flag==0) break;
		
	}

	// Set up lat/lon limits (0 min lat 1 max lat 2 min lon 3 max lon)
	ll_limits[0] = 90;
	ll_limits[1] = -90;
	ll_limits[2] = 180;
	ll_limits[3] = -180;
	for(row=0; row<NY; row++) {
		for (col=0; col<NX; col++) {
			if(mask[row][col]==1) {
				lat = 24.00416666667 + row * RESN;
				lon = -137.99583333333 + col * RESN;
				
				if(lat<ll_limits[0])
					ll_limits[0] = lat;
				else if(lat > ll_limits[1])
					ll_limits[1] = lat;
				
				if(lon<ll_limits[2])
					ll_limits[2] = lon;
				else if(lon > ll_limits[3])
					ll_limits[3] = lon;
				
			}
		}
	}
	
	// Calculate those placesin the basin with a minimum flow accumulation to count as a stream reach
	ngrid = 0;
	for(row=0; row<NY; row++) {
		for (col=0; col<NX; col++) {
			if(mask[row][col]==1 && flowacc[row][col]>=flow_thresh ) {
				mask_minflow[row][col] = 1.;
				ngrid++;
			}
		}
	}
				
	return(ngrid);
	
}  /* end subset_basin */

void assemble_network(float **direc, float **link, float **mask_minflow, int min_row, int max_row, int min_col, int max_col) {
	
	int n, d; 
	int row, col;
	int newrow, newcol;
	
	int sub_nx, sub_ny;
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
	//printf("just allocated the index...");
	
	// then we run through the directions
	for (row=min_row+1; row<max_row-1; row++) {
		for (col=min_col+1; col<max_col-1; col++) {
			// Only solve for those above the threshold
			if (mask_minflow[row][col]==1) {
				d = direc[row][col];
				
				newrow = row - min_row;
				newcol = col - min_col;
				
				if(d==64)
					link[row][col] = index[newrow+1][newcol];
				else if(d==128)
					link[row][col] = index[newrow+1][newcol+1];
				else if(d==1)
					link[row][col] = index[newrow][newcol+1];
				else if(d==2)
					link[row][col] = index[newrow-1][newcol+1];
				else if(d==4)
					link[row][col] = index[newrow-1][newcol];
				else if(d==8)
					link[row][col] = index[newrow-1][newcol-1];
				else if(d==16)
					link[row][col] = index[newrow][newcol-1];
				else if(d==32)
					link[row][col] = index[newrow+1][newcol-1];
				else if(d==0)
					link[row][col] = 0;
				
			}
			
		}
	}
	
	free(index);
} /* end assemble_network */



int calc_solving_order(float **order, float **link, float **nlinks,float **mask_minflow,float ***back_network, int min_row, int max_row, int min_col, int max_col) {

	int count;
	int sub_nx, sub_ny;
	int row, col;
	int n;
	int x,y;
	int nheadwaters;
	int newrow, newcol;
	int nrows, ncols;
	int oldrow, oldcol;
	int mod;
	int jcount, jflag;
	int loop; // TJT can get rid of after debugging
	
	float idx;
	float **index;
	
	sub_nx = max_col - min_col + 1;
	sub_ny = max_row - min_row + 1;
	
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
	
	// Initialize order
	for (row=0; row<NY; row++) {
		for (col=0; col<NX; col++) {
			order[row][col]=-999;
		}
	}

   printf("%d %d %d %d\n",min_row,max_row,min_col,max_col);	
	// Look for headwaters and junctions
        nheadwaters=0;
	for (row=min_row; row<=max_row; row++) {
		for (col=min_col; col<max_col; col++) {
			
			if (mask_minflow[row][col]==1) {
				count = 0;
				
				for (y=row-1;y<=row+1; y++) {
					for(x=col-1; x<=col+1;x++) {
						
						if(link[y][x] == index[row-min_row][col-min_col]) count++;
					}
				}
				nlinks[row][col] = count;
				if(count==0) {
					nheadwaters++;
				}
			}
		}
	}
	printf("nheadwaters %d\n", nheadwaters);

	
	// Loop through, if it's a headwater and has no order yet, then assign the order, and trace it to a junction
	count = 0;
	ncols = max_col-min_col + 1;
	nrows = max_row-min_row +1;
	for (row=min_row+1; row<=max_row-1; row++) {
		for (col=min_col+1; col<max_col-1; col++) {
			if (nlinks[row][col] ==0 && mask_minflow[row][col]==1 && order[row][col] ==-999) {
				
				// Set the order for the headwater
				order[row][col] = count;
				count++;
				//printf("%d %d %d\n", count, row, col);
				
				// Trace it until the next junction
				oldrow = row;
				oldcol = col;
				loop=0;
				jflag=0;
				while (jflag==0) {

					// See where it goes - calculate it based on the index
					// TJT check htis - could be wrong
					idx = link[oldrow][oldcol];
					for (y=oldrow-1; y<=oldrow+1; y++) {
						for (x=oldcol-1; x<=oldcol+1; x++) {
							//printf("\t%d %d %d %.0f %.0f\n",loop, y,x, idx,index[y-min_row][x-min_col]);
							
							if (idx==index[y-min_row][x-min_col]) {
								newrow = y;
								newcol = x;
							}
						}
					}
					loop++;
					
					
					if (nlinks[newrow][newcol]==1 ) { //&& order[newrow][newcol]==0) {
						order[newrow][newcol] = count;
						count++;
						oldrow = newrow;
						oldcol = newcol;
						//printf("a %d %d %d %.0f\n", count, newrow, newcol, order[newrow][newcol]);
					}
					else {
						//printf("in here!\t %.0f\n", order[newrow][newcol]);
						jflag=1;
						break;
					}
				}
				
			}
		}
	}
	
	// Now let's loop through the junctions, see if everything that flows in is
	// ordered. If it is, number the junction and then trace the streamflow down
	// to the next junction
	
	while (1) {
		
		// Find junctions and see if they're ordered. If all are ordered, then break the loop
		jcount = 0;
		for (row=min_row+1; row<=max_row-1; row++) {
			for (col=min_col+1; col<max_col-1; col++) {
				if(nlinks[row][col]>1 && mask_minflow[row][col]==1) {
					if (order[row][col]==-999) {
						jcount++;
					}
				}
			}
		}
		//printf("Jcount a %d\n", jcount);
		if(jcount==0) {
			break;
		}
		
		// If we haven't broken the loop, there's still more to do
		for (row=min_row+1; row<=max_row-1; row++) {
			for (col=min_col+1; col<max_col-1; col++) {

				// See if we're at a junction and it's not ordered
				if (nlinks[row][col]>1 && mask_minflow[row][col]==1 && order[row][col]==-999) {
					//printf("here\n");
					// See if all the upstream segments have been ordered
					jcount = 0;
					for	(n=0; n<nlinks[row][col]; n++) {
						idx = back_network[row][col][n];
						
						for (y=row-1; y<=row+1; y++) {
							for (x=col-1; x<=col+1; x++) {
								//printf("a %d %d %d %.0f %.0f\n",loop, y,x, idx,index[y-min_row][x-min_col]);
								
								if (idx==index[y-min_row][x-min_col]) {
									newrow = y;
									newcol = x;
								}
							}
						}
						
						//printf("b %d %d %.0f %.0f\n", newrow, newcol, nlinks[row][col], order[newrow][newcol]);
						if (order[newrow][newcol]!=-999) {
							jcount++;
						}
					}
					if (jcount==nlinks[row][col]) {
						//printf("in here!!!");
						order[row][col] = count;
						count++;
						oldrow = row;
						oldcol = col;
						while (1) {
							
							// See where it goes - calculate it based on the index
							// TJT check htis - could be wrong
							idx = link[oldrow][oldcol];

							for (y=oldrow-1; y<=oldrow+1; y++) {
								for (x=oldcol-1; x<=oldcol+1; x++) {
									//printf("%d %d %d %.0f %.0f\n",loop, y,x, idx,index[y-min_row][x-min_col]);
									
									if (idx==index[y-min_row][x-min_col]) {
										newrow = y;
										newcol = x;
									}
								}
							}
														
							if (nlinks[newrow][newcol]==1) {
								order[newrow][newcol] = count;
								count++;
								oldrow = newrow;
								oldcol = newcol;
							}
							else {
								break;
							}
						}
					}
				}
			}
		}
	}
	
	free(index);

	return(nheadwaters);

} /* end of calc_solving_order */

void calc_back_network(float **link, float ***back_network, float **mask_minflow, int min_row, int max_row, int min_col, int max_col) {
	
	int count;
	int sub_nx, sub_ny;
	int row, col;
	int n;
	int x,y;

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
	//printf("Limits %d %d %d %d %d %.0f %d %d\n", min_row, max_row, min_col, max_col, n, index[1924-min_row][2020-min_col],
	//	   1924-min_row, 2020-min_col);
	

	// Look for headwaters and junctions
	for (row=min_row+1; row<=max_row-1; row++) {
		for (col=min_col+1; col<max_col-1; col++) {
			if (mask_minflow[row][col]==1) {
				count = 0;
				
				for (y=row-1;y<=row+1; y++) {
					for(x=col-1; x<=col+1;x++) {
						/*if(row==1931 && col==6586) {
							printf("bn1 %d %d %.0f %.0f %.0f %d\n", row, col, link[y][x], 
								   index[row-min_row][col-min_col], back_network[row][col][count], count );
						}*/
						if(link[y][x] == index[row-min_row][col-min_col]) {
							back_network[row][col][count] = index[y-min_row][x-min_col];
							count++;
						}
						
					}
				}
			
			}

		}
	} 
	row=1935;
	col=6582;
	printf("back_network %d %d %.0f %.0f\n", row, col, link[row][col], back_network[row][col][0]);
		
	free(index);
} /* end of calc_back_network */



float calc_distance(float lat1, float lat2, float lon1,float lon2) {
	
	float d1;
	float Re=6.37122e6;
	float a1, b1, a2, b2;
	
	a1=PI/180 * lat1;
	b1=PI/180 * lon1;
	a2=PI/180 * lat2;
	b2=PI/180 * lon2;
	
	d1=acos(cos(a1)*cos(b1)*cos(a2)*cos(b2) + cos(a1)*sin(b1)*cos(a2)*sin(b2) + sin(a1)*sin(a2))*Re;
	
	return(d1);
}


void calc_slope(float **slope, float **elev, float **link, float **nlinks, float **mask_minflow, float ***back_network, 
		   int min_row, int max_row, int min_col, int max_col) {

	int n;
	int row, col;
	int sub_nx, sub_ny;
	int y, x;
	int flag1, flag2; // To check that there is an upstream and downstream cell to average, otherwise just take one or the other
	
	float **lat; 
	float **lon;
	float **index;
	
	float upslope, downslope, dist;
	float idx;
	
	sub_ny = max_row-min_row+1;
	sub_nx = max_col-min_col+1;
	
	// Allocate memory
	index = (float**)calloc(sub_ny,sizeof(float*));
	for(col=0;col<sub_ny;col++)
		index[col] = (float*)calloc(sub_nx,sizeof(float));
	lat = (float**)calloc(sub_ny,sizeof(float*));
	for(col=0;col<sub_ny;col++)
		lat[col] = (float*)calloc(sub_nx,sizeof(float));
	lon = (float**)calloc(sub_ny,sizeof(float*));
	for(col=0;col<sub_ny;col++)
		lon[col] = (float*)calloc(sub_nx,sizeof(float));
	
	
	// first set up index array to get the index, and do the lat & long
	n=0;
	for (row=min_row; row<=max_row; row++) {
		for (col=min_col; col<max_col; col++) {
			index[row-min_row][col-min_col]=n;
			lat[row-min_row][col-min_col] = 24.00416666666667 + row * RESN;
			lon[row-min_row][col-min_col] = -137.995833333333 + col * RESN;
			n=n+1;
		}
	}
	printf("Set up the lat/long network\n");
	
	// loop through rows and columns, if the pixel is in the mask, then we calculate the upstream slope,
	// the downstream slope, and then the average (if two streams coming in, take average of them
	
	for (row=min_row+1; row<=max_row-1; row++) {
		for (col=min_col+1; col<max_col-1; col++) {
			if (mask_minflow[row][col]==1) {
				
				flag1 = 0;
				flag2 = 0;
				
				downslope = 0.;
				upslope = 0.;
				for (y=row-1;y<=row+1; y++) {
					for(x=col-1; x<=col+1;x++) {
						
						// Downstream
						if (link[row][col]==index[y-min_row][x-min_col]) {
							dist = calc_distance(lat[row-min_row][col-min_col],lat[y-min_row][x-min_col], lon[row-min_row][col-min_col],lon[y-min_row][x-min_col]);
							downslope = (elev[row][col] - elev[y][x]) / dist;
							flag1 = 1;
						}
						
						// Upstream
						for	(n=0; n<nlinks[row][col]; n++) {
							idx = back_network[row][col][n];
							if (idx==index[y-min_row][x-min_col]) {
								dist = calc_distance(lat[row-min_row][col-min_col],lat[y-min_row][x-min_col], lon[row-min_row][col-min_col],lon[y-min_row][x-min_col]);
								upslope = upslope + (elev[y][x]-elev[row][col]) / dist / nlinks[row][col];
							}
							flag2 = 1;
						} 
						
					}
				}
				if (flag1==1 && flag2==1) {
					slope[row][col] = 0.5 * (downslope + upslope);
				}
				else if (flag1==1 && flag2==0) {
					slope[row][col] = downslope;
				}
				else if (flag2==1 && flag1==0) {
					slope[row][col] = upslope;
				}
				else {
					slope[row][col] = -999;
				}
				if (slope[row][col]<=0.) {
					slope[row][col] = 0.0001;
				} 
				
			}
			
		}
	} 
	
	
	
} // End calc_slope

void calc_width(float **width, float **flowacc, float **mask_minflow, 
				int min_row, int max_row, int min_col, int max_col) {

	int row, col;
	float area;
	
	for (row=min_row+1; row<=max_row-1; row++) {
		for (col=min_col+1; col<max_col-1; col++) {
			if (mask_minflow[row][col]==1) {
				area = flowacc[row][col];
				
				// Width relationship to drainage area from Ohio streamflow USGS stream rating measurements
				width[row][col] = 1.8723 * pow(area, 0.44417);
			}
		}
	}
	
} // End of calc_width
