#include <time.h>
#include "cubical_function_representation.h"

#define KEND  "\033[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define BRED  "\x1B[1;31m"
#define BGRN  "\x1B[1;32m"
#define BYEL  "\x1B[1;33m"
#define BBLU  "\x1B[1;34m"
#define BMAG  "\x1B[1;35m"
#define BCYN  "\x1B[1;36m"
#define BWHT  "\x1B[1;37m"

bool debug;
int doSift;
int cube_cost(t_blif_cube *cube, int num_inputs);
int function_cost(t_blif_cubical_function *f);
int cover_cost(t_blif_cube **cover, int num_cubes, int num_inputs);
void simplify_function(t_blif_cubical_function *f);
void printValidCoverTable(bool **coverTable, int numRows, int numCols, 
bool *validPIs, bool*validMinterms, int *minterms, t_blif_cube **set_of_cubes, int numInputs);
