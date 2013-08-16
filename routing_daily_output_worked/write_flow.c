#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include "route.h"

#define UNITS "units"
#define DEGREES_EAST "degrees_east"
#define DEGREES_NORTH "degrees_north"

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERR(e) {printf("Error: %s\n", nc_strerror(e));}

void write_flow(float ***flow_out, int nlats, int nlons, int ndays, float vic_min_lat, float vic_min_lon, float vic_resn, int **vic_ts, char *out_path){


int ncid, lon_dimid, lat_dimid, flow_varid;
int lat_varid, lon_varid, ndims;
int dimids[3];
char filename[1024];
int retval;
char flow_units[] = "";
int i, j, k;

float lats[nlats], lons[nlons];
float temp_flow[nlats][nlons];


//Create arrays of lats and lons

for (i = 0; i < nlats; i++)
	lats[i] = vic_min_lat + vic_resn*i;
for (j = 0; j < nlons; j++)
	lons[j] = vic_min_lon + vic_resn*j;
	
//Cycle through days and write each day as separate netCDF file
for (k = 0; k<ndays; k++) {

	//Create filename for daily file
	sprintf(filename, "%s/results_%d%d%d.nc", out_path, vic_ts[k][0],vic_ts[k][1],vic_ts[k][2]);
	printf("%s\n",filename);
	
	/* Create the file. */
	if ((retval = nc_create(filename, NC_CLOBBER, &ncid)))
		ERR(retval);
	printf("%d",1);	
	/* Define the dimensions. */
	if ((retval = nc_def_dim(ncid, "lat", nlats, &lat_dimid)))
		ERR(retval);
	if ((retval = nc_def_dim(ncid, "lon", nlons, &lon_dimid)))
		ERR(retval);
	printf("%d",2);	
	/* Define coordinate netCDF variables.*/
	if ((retval = nc_def_var(ncid, "lat", NC_FLOAT, 1, &lat_dimid,&lat_varid)))
		ERR(retval);
	if ((retval = nc_def_var(ncid, "lon", NC_FLOAT, 1, &lon_dimid,&lon_varid)))
		ERR(retval);
	printf("%d",3);	
	/* Define units attributes for coordinate vars. */
	if ((retval = nc_put_att_text(ncid, lat_varid, UNITS,strlen(DEGREES_NORTH), DEGREES_NORTH)))
		ERR(retval);
	if ((retval = nc_put_att_text(ncid, lon_varid, UNITS,strlen(DEGREES_EAST), DEGREES_EAST)))
		ERR(retval);
	printf("%d",4);	
	/* Define the netCDF variables. The dimids array is used to pass
	the dimids of the dimensions of the variables.*/
	dimids[0] = lat_dimid;
	dimids[1] = lon_dimid;
	ndims = 2;  //2d data
	if ((retval = nc_def_var(ncid, "discharge", NC_FLOAT, ndims,dimids, &flow_varid)))
		ERR(retval);
	printf("%d",5);	
	/* Define units attributes for vars. */
	if ((retval = nc_put_att_text(ncid, flow_varid, UNITS,strlen(flow_units), flow_units)))
		ERR(retval);
		
	/* End define mode. */
	if ((retval = nc_enddef(ncid)))
		ERR(retval);
	printf("%d",6);	
	/* Write the coordinate variable data. This will put the latitudes
	and longitudes of our data grid into the netCDF file. */
	if ((retval = nc_put_var_float(ncid, lat_varid, &lats[0])))
		ERR(retval);
	if ((retval = nc_put_var_float(ncid, lon_varid, &lons[0])))
		ERR(retval);
	printf("%d",7);	
	
	for (i = 0; i < nlats; i++) {
		for (j = 0; j < nlons; j++) {
			temp_flow[i][j] = flow_out[i][j][k];
			}
		}
	
	/* Write the data */
	if ((retval = nc_put_var_float(ncid, flow_varid, &temp_flow[0][0])))
		ERR(retval);
		
	/* Close the file. */
	if ((retval = nc_close(ncid)))
		ERR(retval);

}

}