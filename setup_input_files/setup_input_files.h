#define NX 10320
#define NY 4440
#define DLYMAP = 24.0041666666666667
#define DLXMAP = -137.9958333333333333
#define RESN 0.0083333333333333

// Some constants that need to be revisited TJT
#define MANNING_N 0.035;
#define WIDTH 100;


void read_flowaccum(char *, float **);
void read_direction(char *, float **);
void read_elevation(char *, float **);
void reshape_matrix(float **, float *);
void assemble_network(float **, float **, float **, int, int, int, int);
int subset_basin(float **, float **, float **, float *, float **, int, int, int);
int calc_solving_order(float **, float **, float **, float **, float ***, int, int, int, int);
void calc_back_network(float **, float ***, float **, int, int, int, int);
void setup_output_data(float **, float ***, float **, int, char *, int, int, int, int, float *, int, float **, float **, float **, float **);
float calc_distance(float, float, float, float);
void calc_slope(float **, float **, float **, float **, float **, float ***, int, int, int, int);
void calc_width(float **, float **, float **, int, int, int, int);

