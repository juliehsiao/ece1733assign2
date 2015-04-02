/*
 * Description:
 *
 * This file contains functions that manipulate logic function representation.
 * The idea is to have a unified representation for the function in its logic form,
 * not necessarily how it is implemented. For this purpose each function will be represented
 * in cubical form. This representation will be derived from the physical implementation,
 * so that new mapping possibility can be derived.
 *
 * Author:
 * Tomasz Czajkowski
 * University of Toronto
 * 2002.
 *
 *
 * REVISION HISTORY:
 * June 10, 2003: The following functions have been made available though the header file:
 *					create_empty_cubical_function_with_variable_list(...);
 *					translate_equation_to_cubical_function_representation(...);
 *					create_variable_index_list_for_function(...);
 *					process_single_LUT_function(...);
 *				  As it turns out this code is reusable in carry chain shortening,
 *				  but with different initial conditions. (TC)
 * October 2, 2006: Changed the logic function representation to use 2-bits per symbol versus 8. (TC)
 */

/*******************************************************************************************/
/****************************          INCLUDE FILES            ****************************/
/*******************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
//#include <malloc.h>
#include <string.h>

#include "common_types.h"
#include "cubical_function_representation.h"

/*******************************************************************************************/
/****************************         DEFINE STATEMENTS         ****************************/
/*******************************************************************************************/


/*******************************************************************************************/
/****************************         GLOBAL VARIABLES          ****************************/
/*******************************************************************************************/


/*******************************************************************************************/
/****************************    LOCAL FUNCTION DECLARATIONS    ****************************/
/*******************************************************************************************/


/*******************************************************************************************/
/****************************           IMPLEMENTATION          ****************************/
/*******************************************************************************************/


int read_cube_variable(long int *cube, int var_index)
{
	return (cube[var_index / (4*sizeof(long int))] & (LITERAL_DC << (2*(var_index & (4*sizeof(long int) - 1))))) >> (2*var_index);
}


void write_cube_variable(long int *cube, int var_index, int value)
{
	int mask = (-1) ^ (LITERAL_DC << (2*(var_index & (4*sizeof(long int) - 1))));

	assert((value == LITERAL_0) | (value == LITERAL_1) | (value == LITERAL_DC));

	cube[var_index / (4*sizeof(long int))] = (cube[var_index / (4*sizeof(long int))] & (mask)) |
											 (value << (2*(var_index & (4*sizeof(long int) - 1))));
}


void free_cubical_function(t_blif_cubical_function *f)
/* This function releases the memory allocated for a cubical function.
 */
{
	int index;

	if (f != NULL)
	{
		if (f->inputs > 0)
		{
			for(index = 0; index < f->input_count; index++)
			{
				free(f->inputs[index]);
			}
			free(f->inputs);
		}
		if (f->output != NULL)
		{
			free(f->output);
		}
		for(index = 0; index < f->cube_count; index++)
		{
			free(f->set_of_cubes[index]);
		}
		if (f->set_of_cubes != NULL)
		{
			free(f->set_of_cubes);
		}
		free(f);
	}
}

// takes two lists of PIs and return whether the lists are the same
bool isRedundantSetOfCubes(t_blif_cube **PIs1, int inputCount, int listSize1, t_blif_cube **PIs2, int listSize2 )
{
    int i, j;
    if(listSize1 != listSize2) return false;

    int sameCount = 0;
    for(i = 0; i < listSize1; i++)
    {
        for(j = 0; j < listSize2; j++) {
            if(isRedundantPI(&(PIs1[i]), inputCount, 1, PIs2[j])) {
                sameCount++;
            }
        }
    }

    if(sameCount != listSize1) return false;
    else return true;
}

// takes a list of PI and a newPI, and return whether the newPI exists in the list already
bool isRedundantPI(t_blif_cube **PIs, int inputCount, int listSize, t_blif_cube *newPI)
{
    int i, j;

    for(i = 0; i < listSize; i++)
    {
        bool isRedundant = true;
        for(j = 0; j < inputCount; j++) {
            if(read_cube_variable(PIs[i]->signal_status, j) != read_cube_variable(newPI->signal_status, j)) {
                isRedundant = false;
                break;
            }
        
        }
        if((j==inputCount) && isRedundant) return true;
    }
    return false;
}

// takes 2 cubes and try to merge them
// - returns a newly allocated cube if merge was successful, otherwise NULL
t_blif_cube *mergeImplicants(t_blif_cube *c1, t_blif_cube *c2, int size)
{
    t_blif_cube *ret = NULL;
    int i, pos;
    int numDiff = 0;
    for (i = 0; i < size; i++) {
//        printf("c1[%d] = %d, c2[%d] = %d\n", i, read_cube_variable(c1->signal_status, i), i,  read_cube_variable(c2->signal_status, i));
        if (read_cube_variable(c1->signal_status, i) != read_cube_variable(c2->signal_status, i)) {
            numDiff++;
            pos = i;
        }
        if (numDiff > 1) return NULL;
    }

    ret = (t_blif_cube *) malloc (sizeof(t_blif_cube));
    memcpy(ret, c1, sizeof(t_blif_cube));

    // change the one bit that's diff
    write_cube_variable(ret->signal_status, pos, LITERAL_DC);
    return ret;
}

void freeSetOfCubes (t_blif_cube **cubes, int cube_count)
{
    int i;
    for (i = 0; i < cube_count; i++) {
        free(cubes[i]);
    }
    free(cubes);
}

char translateLiterals(int literal)
{
    char ret = ' ';
    if(literal == LITERAL_0) ret = '0';
    else if(literal == LITERAL_1) ret = '1';
    else if(literal == LITERAL_DC) ret = 'X';
    return ret;
}

void printCube(t_blif_cube *cube, int numInputs)
{
    int j;
    for(j = 0; j < numInputs; j++) {
        printf("%c ", translateLiterals(read_cube_variable(cube->signal_status, j)));
    }
}

void printSetOfCubes(t_blif_cube **cubes, int numInputs, int numCubes)
{
    int i, j;
	// Print a border
	for (i = 0; i < numInputs; i++) {
		printf("==");
	}
	printf("\n");
    for(i = 0; i < numCubes; i++) {
        for(j = 0; j < numInputs; j++) {
            printf("%c ", translateLiterals(read_cube_variable(cubes[i]->signal_status, j)));
        }
        printf("\n");
    }
	// Print a border
	for (i = 0; i < numInputs; i++) {
		printf("==");
	}
	printf("\n");
}

// given a cube, returns all cubes represented by that cube in an int array
// - returns index of the next free spot
int enumerateAllMinterms(t_blif_cube * cube, int *mintermArray, int startIndex, int input_count)
{
    int numX = 0;
    int val = 0;
    int pos[64] = {0};
    int posIndex = 0;
    int j, i;
    for(j = 0; j < input_count; j++) {  // for each bit of the cube
        if(read_cube_variable(cube->signal_status, j) == LITERAL_DC) { //if it is a DC
            numX++;
            pos[posIndex++] = input_count - 1 - j;
        }
        else {
           val += (read_cube_variable(cube->signal_status, j) == LITERAL_1) << (input_count - 1 - j);
        }
    }

    for(j = 0; j < (1 << numX); j++) {
        mintermArray[startIndex] = val;
        for(i = 0; i < numX; i++) {
            mintermArray[startIndex] += (j & (1 << i)) >> i << pos[i];
        }
        // check if this newly added term is present in array already
        startIndex++;
    }
    //printf("\n");
    return startIndex;
}





