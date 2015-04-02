#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "findPI.h"
#include "assign2.h"


// Orders the set_of_cubes by cardinality of 1's
// - this function assumes that ordered has been allocated enough (t_blif_cube *)s
//   and assigns the address of f->set_of_cubes to ordered[i] (doesn't allocate t_blif_cube)
void orderSetOfCubes (t_blif_cubical_function *f, t_blif_cube **ordered)
{
    int index = 0;
    assert(f->set_of_cubes);
    int i;
    for (i = 0; i <= f->input_count; i++) {
        int j;
        for (j = 0; j < f->cube_count; j++) {
        	int cardinality = findCardinality(f->set_of_cubes[j], f->input_count);
			if (cardinality == i) {
                ordered[index++] = f->set_of_cubes[j];
            }
        }
    }
	if (index != f->cube_count) {
		printf("%sIn orderSetOfCubes: f->cube_count (%d) != index (%d)%s\n", BRED, f->cube_count, index, KEND);
	    assert(0);
	}
}

// Returns cardinality of a cube cube, size is the number of inputs for the function
int findCardinality (t_blif_cube *cube, int size)
{
    int i;
    int ret = 0;
    for (i = 0; i < size; i++) {
        if (read_cube_variable(cube->signal_status, i) == LITERAL_1) ret++;
    }
    return ret;
}

// Takes a set of cubes and returns a set of PIs
void findPI(t_blif_cubical_function *f, t_blif_cube **PIs)
{
    t_blif_cube ** tmpPIs, ** orderedSetOfCubes;

    bool mergedCubes = true;
    int PIIndex = 0;
	
	printf("All Implicants\n");
    printSetOfCubes(f->set_of_cubes, f->input_count, f->cube_count);
	printf("\n");

    // Loop to find the PIs
    while (mergedCubes) {
        mergedCubes = false;

    	orderedSetOfCubes = (t_blif_cube **) malloc (f->cube_count * sizeof(t_blif_cube *));

		// Order the set of cubes based on cardinality of 1's
    	orderSetOfCubes(f, orderedSetOfCubes); // orderedSetOfCubes[i] now points to original t_blif_cube structs from f->set_of_cubes

        if (debug) printSetOfCubes(orderedSetOfCubes, f->input_count, f->cube_count);

        bool *used = (bool *) malloc (f->cube_count * sizeof(bool));
        memset(used, false, f->cube_count * sizeof(bool));

        int newCubeCount = 0;

        // Malloc a "set_of_cubes" that is square of the number of cubes in the set
        // This is the largest possible number of cubes we can have
        tmpPIs = (t_blif_cube **) malloc (f->cube_count * f->cube_count * sizeof (t_blif_cube *));

        int i, j;
        for (i = 0; i < f->cube_count-1; i++) { //for every cube
            for (j = i+1; j < f->cube_count; j++) { //compare with subsequent cubes
                if (findCardinality(orderedSetOfCubes[i], f->input_count)+1 != findCardinality(orderedSetOfCubes[j], f->input_count)) continue;
                t_blif_cube *mergedImplicant = mergeImplicants(orderedSetOfCubes[i], orderedSetOfCubes[j], f->input_count);
                if (mergedImplicant != NULL) { //if merge was successful
                    used[i] = true;
                    used[j] = true;
                    tmpPIs[newCubeCount++] = mergedImplicant;
                    mergedCubes = true;
                }
            }
        }

		// unused cubes should be added to the tmpPIs list to be reused for combine
		if (!mergedCubes) {
	        for(i = 0; i < f->cube_count; i++) { //for every cube
    	        if ((used[i] == false) &&  // if it wasn't used for merge and it is not already in the list
        	            !isRedundantPI(PIs, f->input_count, PIIndex, orderedSetOfCubes[i])) {
            	    //allocate t_blif_cube and add it in the list
                	PIs[PIIndex] = (t_blif_cube *) malloc(sizeof(t_blif_cube));
                	PIs[PIIndex++][0] = orderedSetOfCubes[i][0]; 
            	}
        	}
		} else {
	        for(i = 0; i < f->cube_count; i++) { //for every cube
    	        if ((used[i] == false) &&  // if it wasn't used for merge and it is not already in the list
        	            !isRedundantPI(PIs, f->input_count, PIIndex, orderedSetOfCubes[i])) {
            	    //allocate t_blif_cube and add it in the list
                	tmpPIs[newCubeCount] = (t_blif_cube *) malloc(sizeof(t_blif_cube));
                	tmpPIs[newCubeCount++][0] = orderedSetOfCubes[i][0]; 
            	}
        	}
		}
        
        if (newCubeCount != 0) { //if there is some merging happening in this iteration
            freeSetOfCubes(orderedSetOfCubes, f->cube_count); //free the old list
            orderedSetOfCubes = tmpPIs; //make the old list point to the new list
			f->set_of_cubes = tmpPIs;
            f->cube_count = newCubeCount; // update cube count
        }

        free(used);
    }

	printf("All Prime Implicants\n");
    printSetOfCubes(orderedSetOfCubes, f->input_count, f->cube_count);
	printf("\n");

    //update the final cube count
    f->cube_count = PIIndex;
    
    free(tmpPIs); //when execution gets here, there are no newly allocated t_blif_cube so just free pointer array
}

