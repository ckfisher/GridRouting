#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <netcdf.h>
#include "route.h"

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERR(e) {printf("Error: %s\n", nc_strerror(e));}

void read_vic_runoff(float ***vic_runoff, int **vic_ts, float vic_resn, int endtime, int *startdate, int *enddate, 
					 float *ll_limits, char *vic_path) {
	
	FILE *fvic;
	
	char filename[1024];
	char dummy[1024];
	
	int year, month, day;
	int nrows, ncols;
	int ndays;
	int index;
	int t, i, j, ind_lat, ind_lon;
	int start_idx, end_idx;
	int flag;
	
	float vic_min_lat, vic_max_lat, vic_min_lon, vic_max_lon;
	float lat, lon;
	int NLAT = 113;
	int NLON = 233;
	float lats_in[NLAT], lons_in[NLON];
	float runoff_in[NLAT][NLON]; 
	int count = 0;
	
	//netCDF vars
	int ncid, ndims_in, nvars_in, ngatts_in, unlimid_in;
	int lat_varid, lon_varid, retval, runoff_varid;
	
	
	// Set up the lat/lon limits and grid size
	vic_min_lat = floor(ll_limits[0]);
	vic_max_lat = ceil(ll_limits[1]);
	vic_min_lon = floor(ll_limits[2]);
	vic_max_lon = ceil(ll_limits[3]);
	
	nrows = (int)((vic_max_lat - vic_min_lat) / vic_resn);
	ncols = (int)((vic_max_lon - vic_min_lon) / vic_resn);
	
	endtime = calculate_nsecs(startdate, enddate);
	ndays = endtime / 86400;	
        
	// Read in the VIC data
	flag=1;
	for (index=1462; index<=2557; index++) {	
			
			//Make filename
			sprintf(filename, "%s/data_%d.nc", vic_path, index);
			//Open the file
			nc_open(filename, NC_NOWRITE, &ncid);
			//Read in file info
			nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in, &unlimid_in);
						
			//printf("%d %d %d %d %d %d \n", index, ncid, ndims_in, nvars_in, ngatts_in, unlimid_in);
			
			
			  if ((retval = nc_inq_varid(ncid, "lat", &lat_varid)))
			  	ERR(retval);
			  if ((retval = nc_inq_varid(ncid, "lon", &lon_varid)))
				ERR(retval);
			  /* Read the coordinate variable data. */
		     	 if ((retval = nc_get_var_float(ncid, lat_varid, &lats_in[0])))
		      		ERR(retval);
			  if ((retval = nc_get_var_float(ncid, lon_varid, &lons_in[0])))
			  	ERR(retval);
			  //printf("%f %f\n",lats_in[0], lats_in[112]);
			  //printf("%f %f\n",lons_in[0], lons_in[232]);
			  
			  if ((retval = nc_inq_varid(ncid, "crunoff", &runoff_varid)))
				ERR(retval);
			  if ((retval = nc_get_var_float(ncid, runoff_varid, &runoff_in[0][0])))
				ERR(retval);
				
			int y = sizeof(runoff_in)/sizeof(float)/(sizeof(runoff_in[0])/sizeof(float)); /* lat size */
			int x = sizeof(runoff_in[0])/sizeof(float); /* lon size */
			
			for (i=0; i<(y-1); i++) { 
				if (lats_in[i] == vic_min_lat) {
					ind_lat = i;
					break;
				} 
			}
			
			for (i=0; i<(x-1); i++) {
				if(lons_in[i] == vic_min_lon) { 
					ind_lon = i;
					break;
				}
			}
			
			for (i=ind_lat; i<(ind_lat+nrows); i++) {
				for (j=ind_lon; j<(ind_lon+ncols); j++) {				
					vic_runoff[i-ind_lat][j-ind_lon][count] = runoff_in[i][j];
					//printf("%f %f\n",vic_runoff[i-ind_lat][j-ind_lon][count],runoff_in[i][j]);
				}
			}
			
			nc_close(ncid);
			count++;
			
					
	}
	
	//system("read -n 1 -s -p \"Press any key to continue...\"");
} /* End of read_vic_runoff */

