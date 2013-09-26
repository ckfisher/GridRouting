#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "setup_vic_input_files.h"

#define PI 3.1415265 




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

