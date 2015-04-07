////////////////////////////////////////////////////////////////////////
// Solution to assignment #1 for ECE1733.
// This program implements functions on BDDs
// Authors: Julie Hsiao & Joy Chen
////////////////////////////////////////////////////////////////////////

/**********************************************************************/
/*** HEADER FILES *****************************************************/
/**********************************************************************/

#include <stdlib.h>
//#include <conio.h>
//#include <curses.h> //replaces conio.h
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "common_types.h"
#include "blif_common.h"
#include "cubical_function_representation.h"
#include "findPI.h"
#include "assign2.h"

/**********************************************************************/
/*** DATA STRUCTURES DECLARATIONS *************************************/
/**********************************************************************/
typedef struct _TRow {
    int var;
    int low;
    int high;
} TRow;

/**********************************************************************/
/*** DEFINE STATEMENTS ************************************************/
/**********************************************************************/
#define INIT_SIZE 1024
#define AND  0x100
#define OR   0x101
#define XOR  0x102
#define NAND 0x103
#define NOR  0x104
#define XNOR 0x105

/**********************************************************************/
/*** GLOBAL VARIABLES *************************************************/
/**********************************************************************/
int numInputs = 0;
TRow *T;
int TRowThreshold = INIT_SIZE;
int numTRows = 0;
int **G;
int GThreshold = INIT_SIZE;



/**********************************************************************/
/*** Functions for printing BDD graph *********************************/
/**********************************************************************/
void printSubGraph(t_blif_cubical_function *f, t_blif_signal **inputs, FILE *fp,
        int u, bool *printed)
{
    int nameIdx = f->inputs[T[u].var]->data.index;
    //set the name of this node
    printf("\t%d [label=\"%s\"];\n", u, inputs[nameIdx]->data.name);
    fprintf(fp, "\t%d [label=\"%s\"];\n", u, inputs[nameIdx]->data.name);

    //--------------------------
    // print low subgraph
    //--------------------------
    //print 0-path if it points to terminal node
    if( (T[u].low == 0)||(T[u].low == 1) ) 
    {
        printf("\t%d -> %d [label=\" 0\"];\n", u, T[u].low);
        fprintf(fp, "\t%d -> %d [label=\" 0\"];\n", u, T[u].low);
    }
    else
    {
        // print the subgraph if it hasn't been printed
        if(false == printed[T[u].low])
            printSubGraph(f, inputs, fp, T[u].low, printed);

        // print the 0-path of this node
        printf("\t%d -> %d [label=\" 0\"];\n", u, T[u].low);
        fprintf(fp, "\t%d -> %d [label=\" 0\"];\n", u, T[u].low);
    }

    //--------------------------
    // print high subgraph
    //--------------------------
    //print 1-path if it points to terminal node
    if( (T[u].high == 0)||(T[u].high == 1) )
    {
        printf("\t%d -> %d [label=\" 1\" fontcolor=blue] [color=blue];\n", 
                u, T[u].high);
        fprintf(fp, "\t%d -> %d [label=\" 1\" fontcolor=blue] [color=blue];\n", 
                u, T[u].high);
    }
    else
    {
        // print the subgraph if it hasn't been printed
        if(false == printed[T[u].high])
            printSubGraph(f, inputs, fp, T[u].high, printed);

        // print the 1-path of this node
        printf("\t%d -> %d [label=\" 1\" fontcolor=blue] [color=blue];\n", 
                u, T[u].high);
        fprintf(fp, "\t%d -> %d [label=\" 1\" fontcolor=blue] [color=blue];\n", 
                u, T[u].high);
    }

    //set node as printed
    printed[u] = true;
}

void printDOTfromNode(t_blif_cubical_function *f, t_blif_signal **inputs, 
        int BDDnum, int rootNode)
{
    printf("there are %d inputs and %d nodes in total\n", numInputs, numTRows);

    FILE *fp;
    char fileName[50]; // should be more than enough
    sprintf(fileName, "BDD%d.dot", BDDnum);
    fp = fopen(fileName, "w+");

    //temp bool to keep track of whether a node has been printed or not
    bool *printed = (bool*) malloc (numTRows * sizeof(bool));
    memset(printed, false, numTRows * sizeof(bool));

    printf("digraph BDD {\n");
    fprintf(fp, "digraph BDD {\n");
    
    if(rootNode != 0)
    {
        printf("\t1 [shape=box];\n");
        fprintf(fp, "\t1 [shape=box];\n");
    }
    if(rootNode != 1)
    {
        printf("\t0 [shape=box];\n");
        fprintf(fp, "\t0 [shape=box];\n");
    }
    
    printSubGraph(f, inputs, fp, rootNode, printed);

    // free temp bool
    free(printed);

    printf("}\n");
    fprintf(fp, "}\n");
    fclose(fp);
}


void printDOT(t_blif_cubical_function *f, t_blif_signal **inputs, int BDDnum, bool isAllX)
{
    printf("there are %d inputs and %d nodes in total\n", numInputs, numTRows);

    FILE *fp;
    char fileName[50]; // should be more than enough
    sprintf(fileName, "BDD%d.dot", BDDnum);
    fp = fopen(fileName, "w+");

    printf("digraph G {\n");
    fprintf(fp, "digraph G {\n");
    // 0 should be printed as long as it's not all DC
    if(!isAllX)
    {
        printf("\t0 [shape=box];\n");
        fprintf(fp, "\t0 [shape=box];\n");
    }
    if(f->cube_count > 0)
    {
        // 1 should always be there
        printf("\t1 [shape=box];\n");
        fprintf(fp, "\t1 [shape=box];\n");

        int i;
        //for(i=0; i < f->input_count; i++)
        for(i=2; i<numTRows; i++)
        {
            int var = T[i].var;
            int idx = f->inputs[var]->data.index;
            printf("input name = %s\n", inputs[idx]->data.name);
            printf("\t%d [label=\"%s\"];\n", i, inputs[idx]->data.name);
            fprintf(fp, "\t%d [label=\"%s\"];\n", i, inputs[idx]->data.name);
        }
        for(i=2; i<numTRows; i++)
        {
            // 0-path
            printf("\t%d -> %d [label=\" 0\"];\n", i, T[i].low);
            fprintf(fp, "\t%d -> %d [label=\" 0\"];\n", i, T[i].low);
            // 1-path
            printf("\t%d -> %d [label=\" 1\" fontcolor=blue] [color=blue];\n", 
                    i, T[i].high);
            fprintf(fp, "\t%d -> %d [label=\" 1\" fontcolor=blue] [color=blue];\n", 
                    i, T[i].high);
        }
    }
    printf("}\n");
    fprintf(fp, "}\n");
    fclose(fp);
}

void printTTable (TRow *T, t_blif_cubical_function *f)
{
    int i;
    printf("=====================================\n");
    printf("|u\t|var\t|low\t|high\n");
    printf("|0\t|\n");
    printf("|1\t|\n");
    for (i = 2; i < numTRows; i++) {
        printf("|%d\t|%d\t|%d\t|%d\n", i, T[i].var, T[i].low, T[i].high);
    }
    printf("=====================================\n");
}

/**********************************************************************/
/*** Functions for G **************************************************/
/**********************************************************************/

//---------------------------------------------------------------------
// Initialize the table G with size INIT_SIZE * INIT_SIZE
// each entry is a previously computed result of APPLY
//---------------------------------------------------------------------
void initG()
{
    G = (int **) malloc(INIT_SIZE * INIT_SIZE * sizeof(int));
    // initialize all value to -1 (empty)
    memset(G, -1, INIT_SIZE * INIT_SIZE * sizeof(int));
}

//---------------------------------------------------------------------
// increase the size of table G
//---------------------------------------------------------------------
void updateGSize(int size)
{
    G = (int **) realloc(G, size * size * sizeof(int));
    // initialize all value to -1 (empty)
    int i, j;
    for(i=0; i<size; i++) {
        for(j=0; j<size; j++) {
            if((i>=GThreshold) || (j>=GThreshold))
                G[i][j] = -1;
        }
    }
    GThreshold = size;
}


/**********************************************************************/
/*** Functions for T **************************************************/
/**********************************************************************/

//---------------------------------------------------------------------
// Initialize the table T with the two terminal nodes, 0 and 1
// each row of table T is one node
//---------------------------------------------------------------------
void initT()
{
    // reserve space for terminal nodes only
    T = (TRow*) malloc(INIT_SIZE * sizeof(TRow));
    // initialize value for the terminal nodes
    T[0].var = numInputs; // terminal node 0
    T[1].var = numInputs; // terminal node 1
    numTRows = 2;
}

//---------------------------------------------------------------------
// add a row to table T. This corresponds to the creation of a new node in BDD
// returns the position(index) of the node in the table
//---------------------------------------------------------------------
int add(int i, int l, int h)
{
    int idx = numTRows;
    numTRows++;

    if(numTRows > TRowThreshold)
    {
        TRowThreshold += INIT_SIZE;
        T = (TRow*) realloc(T, TRowThreshold);
        updateGSize(TRowThreshold);
    }

    T[idx].var = i;
    T[idx].low = l;
    T[idx].high = h;
    return idx;
}

//---------------------------------------------------------------------
// delete a row to table T
//---------------------------------------------------------------------
void removeNode(int i, int l, int h)
{
//    int u = lookup(i, l, h);

}

//---------------------------------------------------------------------
// remove a variable from the BDD
// returns the position(index) of the node in the table
//---------------------------------------------------------------------
void removeVar(int i)
{
}

/**********************************************************************/
/*** Functions for H **************************************************/
/**********************************************************************/

//---------------------------------------------------------------------
// returns whether a node with varIdx=i, 0-path=l and 1-path=h exists in T
//---------------------------------------------------------------------
bool member(int i, int l, int h)
{
    bool found = false;
    int idx;
    for(idx=0; (idx < numTRows) && (!found); idx++)
    {
        if( (T[idx].var == i) && (T[idx].low == l) && (T[idx].high == h) )
        {
            found = true;
            break;
        }
    }
    return found;
}

//---------------------------------------------------------------------
// returns position of a node with varIdx=i, 0-path=l and 1-path=h exists in T
//---------------------------------------------------------------------
int lookup(int i, int l, int h)
{
    bool found = false;
    int idx;
    for(idx=0; (idx < numTRows) && (!found); idx++)
    {
        if( (T[idx].var == i) && (T[idx].low == l) && (T[idx].high == h) )
        {
            found = true;
            break;
        }
    }
    if(found)
        return idx;
    else
        return -1; //always call member() before this, so should never reach here
}

/**********************************************************************/
/*** BDD CODE *********************************************************/
/**********************************************************************/

//---------------------------------------------------------------------
// searches the table for a node with varIdx=i, 0-branch=l, 1-branch=h.
// returns position of the matching node if one exists.
// Otherwise it creates a new node u and returns the identity of it.
//---------------------------------------------------------------------
int MK(int i, int l, int h)
{
    if(l==h)
        return l;
    else if (member(i, l, h))
        return lookup(i, l, h);
    else
    {
        int u = add(i, l, h);
        return u;
    }
}

//---------------------------------------------------------------------
// create an updated set_of_cubes based on setting an input (idx) to values val
// returns a newly allocated set_of_cubes
//      or a LITERAL 1 or 0 vslue if it has no cofactors
//---------------------------------------------------------------------
int createCofactor(t_blif_cube **oldCubes, int oldCubeCount, int idx, int val,
        t_blif_cube **newCubes, int *newCubeCount)
{
    int i;
    int cnt = 0;
    // find out how many cubes should be in newCubes
    for(i=0; i < oldCubeCount; i++)
    {
        if((read_cube_variable(oldCubes[i]->signal_status, idx) == val) || 
           (read_cube_variable(oldCubes[i]->signal_status, idx) == LITERAL_DC))
        {
            cnt++;
        }
    }
    *newCubeCount = cnt;

    // allocate the newCubes
    int j = 0;
    for(i=0; (i<oldCubeCount) && (j<cnt); i++)
    {
        if((read_cube_variable(oldCubes[i]->signal_status, idx) == val) || 
           (read_cube_variable(oldCubes[i]->signal_status, idx) == LITERAL_DC))
        {
            newCubes[j] = (t_blif_cube *) malloc(sizeof(t_blif_cube)); 
            assert(newCubes[j]);
            memcpy(newCubes[j++], oldCubes[i], sizeof(t_blif_cube));
            //newCubes[j++][0] = oldCubes[i][0]; 
        }
    }
    assert(j==cnt);

    if(idx == numInputs-1)
    {
        // after setting xi to val, if there are no more matching, then f=false
        if(cnt == 0)
        {
            printf("no matching cubes for x%d\n", idx);
            return LITERAL_0;
        }
        // after setting the last variable x_(n-1), if there are matching ones,
        // then f=true
        else
        {
            printf("assigning last input and still found a match\n");
            return LITERAL_1;
        }
    }


    // if there are more xi to assign, then this should not be examined.
    // just assign a garbage value for now
    return -1;
}

//---------------------------------------------------------------------
// return the node index
//---------------------------------------------------------------------
int build(t_blif_cube **setOfCubes, int cubeCount, int i, int literalVal)
{
    if(i == numInputs)
    {
        if(literalVal == LITERAL_0)
            return 0;
        else
            return 1;
    }
    else
    {
        // this allocs more space, but that's ok
        t_blif_cube **v0NewCubes = (cubeCount==0)?(NULL):
            ((t_blif_cube **) malloc(cubeCount * sizeof(t_blif_cube *))); 
        int v0NewCubeCount = 0;
        //printf("\n***** 0-cofactor on x%d\n", i);
        //printf("old cube count=%d\n", cubeCount);
        //printf("old cube addr=%p\n", setOfCubes);
        //if(cubeCount > 0)
        //    printf("old cube[0] addr=%p\n", setOfCubes[0]);
        //printSetOfCubes(setOfCubes, numInputs, cubeCount);
        int v0Literal = createCofactor(setOfCubes, cubeCount, i, LITERAL_0,
                            v0NewCubes, &v0NewCubeCount);
        //printf("new cube count=%d\n", v0NewCubeCount);
        //if(v0Literal==-1)
        //{
        //    printSetOfCubes(v0NewCubes, numInputs, v0NewCubeCount);
        //}
        int v0 = build(v0NewCubes, v0NewCubeCount, i+1, v0Literal);

        if(v0NewCubes)
            freeSetOfCubes(v0NewCubes, v0NewCubeCount);



        // this allocs more space, but that's ok
        t_blif_cube **v1NewCubes = (cubeCount==0)?(NULL):
            ((t_blif_cube **) malloc(cubeCount * sizeof(t_blif_cube *))); 
        int v1NewCubeCount = 0;
        //printf("\n***** 1-cofactor on x%d\n", i);
        //printf("old cube count=%d\n", cubeCount);
        //printf("old cube addr=%p\n", setOfCubes);
        //if(cubeCount > 0)
        //    printf("old cube[0] addr=%p\n", setOfCubes[0]);
        //printSetOfCubes(setOfCubes, numInputs, cubeCount);
        int v1Literal = createCofactor(setOfCubes, cubeCount, i, LITERAL_1,
                            v1NewCubes, &v1NewCubeCount);
        //printf("new cube count=%d\n", v1NewCubeCount);
        //if(v1Literal==-1)
        //{
        //    printSetOfCubes(v1NewCubes, numInputs, v1NewCubeCount);
        //}
        int v1 = build(v1NewCubes, v1NewCubeCount, i+1, v1Literal);

        if(v1NewCubes)
            freeSetOfCubes(v1NewCubes, v1NewCubeCount);

        return MK(i, v0, v1);
    }
}

int res(int u, int j, int b) 
{
    if(T[u].var > j)
        return u;
    else if (T[u].var < j)
        return MK(T[u].var, res(T[u].low, j, b), res(T[u].high, j, b));
    else
    {
        if(LITERAL_0 == b)
            return res(T[u].low, j, b);
        else
            return res(T[u].high, j, b);
    }
}


int perfOp(int op, int u1, int u2)
{
    if(AND == op)
        return u1 && u2;
    else if(OR == op)
        return u1 || u2;
    else if(XOR == op)
        return u1 != u2;
    else if(NAND == op)
        return !(u1 && u2);
    else if(NOR == op)
        return !(u1 || u2);
    else if(XNOR == op)
        return !(u1 != u2);
    //shouldn't get here...
    return 0;
}

//---------------------------------------------------------------------
// return the node index of the root of u1<op>u2
//---------------------------------------------------------------------
int APP(int op, int u1, int u2)
{
    int u;

    if(G[u1][u2] != -1)
        return G[u1][u2];
    else if ( ((u1 == 0)||(u1 == 1)) && ((u2 == 0)||(u2 == 1)) )
        u = perfOp(op, u1, u2);
    else if (T[u1].var == T[u2].var)
        u = MK(T[u1].var, APP(op, T[u1].low, T[u2].low), APP(op, T[u1].high, T[u2].high));
    else if (T[u1].var < T[u2].var)
        u = MK(T[u1].var, APP(op, T[u1].low, u2), APP(op, T[u1].high, u2));
    else 
        u = MK(T[u2].var, APP(op, u1, T[u2].low), APP(op, u1, T[u2].high));

    G[u1][u2] = u;

    return u;
}


//---------------------------------------------------------------------
// Given a list of t_blif_signals, return the reference to the idx-th
// element/signal
//---------------------------------------------------------------------
/*
t_blif_signal *findVar (t_blif_signal **inputs, int idx, int size)
{
    int i;
    t_blif_signal **signal = inputs;
    for (i = 0; i < idx; i++) {
        assert(i < size);
        signal++;
    }

    return *signal;
}
*/


// Remove the idx-th entry from the T table
void removeTableEntry (TRow *T, int idx, int newIdx) {
    int i;
    for (i = 0; i < numTRows; i++) {
        if (T[i].low == idx) {
            T[i].low = newIdx;
        }
        if (T[i].high == idx) {
            T[i].high = newIdx;
        }
    }
    return;
}

// Simplify the entries in the T table
// An entry can be simplified if both the low and high entries are the same
void simplifyTable (TRow *T) {
    int i;
    for (i = 0; i < numTRows; i++) {
        if (T[i].low == T[i].high && T[i].low > 0) {
            printf("Redundant entry: %d|%d|%d|%d|\n", i, T[i].var, T[i].low, T[i].high);
            removeTableEntry(T, i, T[i].low);

            // Set low and high entries to -1 to signify that it is invalid
            T[i].low = -1;
            T[i].high = -1;
        }
    }
    return;
}

int findVarOrderIdx (int *varOrder, int idx, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (varOrder[i] == idx) return i;
    }
    printf("Error, out of range!\n");
    assert(0);
}

void swap (int var1, int var2, TRow *localT, TRow *T) {
    printf("swap: %d <=> %d\n", var1, var2);

    int i;
    int table[4];
    // Modify only the first encountered pair for now!
    for (i = 0; i < numTRows; i++) {
        if (T[i].var == var1) {
            printf("|%d\t|%d\t|%d\t|%d\t|\n", i, T[i].var, T[i].low, T[i].high);
            // Here are the possibilities:
            // a) low points to an entry representing var2
            // b) low points to an entry representing a valid variable not var2
            // c) low points to an entry representing 0/1
            // that's all folks, assert otherwise
            if (T[i].low < numTRows && T[i].low > 0) {
                if (T[T[i].low].var == var2) { // a
                    table[0] = T[T[i].low].low;
                } else if (T[i].low < numTRows && T[i].low > 0) { // b, c treated the same
                    table[0] = T[i].low;
                } else {
                    assert(0);
                }
            }

            if (T[i].low < numTRows && T[i].low > 0) {
                if (T[T[i].low].var == var2) { // a
                    table[1] = T[T[i].low].high;
                } else if (T[i].low < numTRows && T[i].low > 0) { // b, c treated the same
                    table[1] = T[i].low;
                } else {
                    assert(0);
                }
            }

            if (T[i].high < numTRows && T[i].high > 0) {
                if (T[T[i].high].var == var2) { // a
                    table[2] = T[T[i].high].low;
                } else if (T[i].high < numTRows && T[i].high > 0) { // b, c treated the same
                    table[2] = T[i].high;
                } else {
                    assert(0);
                }
            }

            if (T[i].high < numTRows && T[i].high > 0) {
                if (T[T[i].high].var == var2) { // a
                    table[3] = T[T[i].high].high;
                } else if (T[i].high < numTRows && T[i].high > 0) { // b, c treated the same
                    table[3] = T[i].high;
                } else {
                    assert(0);
                }
            }

            break; // TODO change this if you want to handle more than 1 pattern
        }
    }

    printf("Table %d %d %d %d\n", table[0], table[1], table[2], table[3]);

    // Swap the orders
    T[i].var = var2;
    if (T[i].low < numTRows && T[i].low > 1 && T[T[i].low].var == var2) {
        T[T[i].low].var = var1;
        //T[T[i].low].high = T[T[i].high].low;
        T[T[i].low].high = table[2];
        printf("mod - T[%d].high <= %d\n", T[i].low, table[2]);
    }

    if (T[i].high < numTRows && T[i].high > 1 && T[T[i].high].var == var2) {
        T[T[i].high].var = var1;
        //T[T[i].high].low = T[T[i].low].high;
        T[T[i].high].low = table[1];
        printf("mod - T[%d].low <= %d\n", T[i].high, table[1]);
    }

    simplifyTable(T);

    //=====================================================
    // [1] Set the table to reflect the variable swap
    //     and remove redundant rows
    //     x_i <=> x_j
    //=====================================================

    // Modify the table such that:
    // 1) Entries before the cut line need to point to the correct entry

    // 2) Entries i and j need to be modified to point to the correct entry

    // 3) Entries after the cut line don't need to be modified


    //=====================================================
    // [2] Keep track of the optimal position for current
    //     variable
    //=====================================================

    //=====================================================
    // [3] Modify the local T table after finding optimal pos
    //=====================================================

}

void printVarOrder (int *varOrder, int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("%d\t", varOrder[i]);
    }
    printf("\n");
}


//---------------------------------------------------------------------
// Sifting function
// Performs the sifting procedure to try to find the minimal sized BDD
//---------------------------------------------------------------------
void sift(t_blif_cubical_function *f)
{
    // Given the T table containing the current BDD, perform sifting
    // to find the BDD which contains the minimal number of nodes
    int i, j;

    //=====================================================
    // [1] Need a local copy of the T table
    //=====================================================
    TRow *localT = (TRow*) malloc (INIT_SIZE * sizeof(TRow));
    int numlocalTRows = numTRows;

/*
    printf("=====================================\n");
    printf("Original T Table:\n");
    printf("|u\t|var\t|low\t|high\n");
    printf("|0\t|\n");
    printf("|1\t|\n");
    for (i = 2; i < numTRows; i++) {
        //localT[i].var = T[i].var;
        //localT[i].low = T[i].low;
        //localT[i].high = T[i].high;
        printf("|%d\t|x%d\t|%d\t|%d\n", i, T[i].var, T[i].low, T[i].high);
    }
    printf("=====================================\n");
*/
    printTTable(T, f);

    //=====================================================
    // [2] For each variable, swap with every other
    //     variable to find the optimal placement
    //=====================================================
    int *varOrder = (int *) malloc (numInputs * sizeof(int));
    for (i = 0; i < numInputs; i++) {
        varOrder[i] = f->inputs[i]->data.index; // initial ordering
    }

    printVarOrder(varOrder, numInputs);

    int varIidx, varJidx;
    for (i = 0; i < numInputs; i++) {
        varIidx = findVarOrderIdx(varOrder, i, numInputs);
        bool reverse = (varIidx == 0) ? false : true; // Need to reverse to the beginning if did not start there
        // i is the variable number

        //=====================================================
        // [3] Perform swap for each input to test all possible
        //     positions to find the position of minimum cost
        //=====================================================
        for (varJidx = varIidx + 1; varJidx < numInputs; varJidx++) { // swap downward
            swap(varOrder[varIidx], varOrder[varJidx], localT, T);
            // update varOrder array
            int tmp = varOrder[varIidx];
            varOrder[varIidx] = varOrder[varJidx];
            varOrder[varJidx] = tmp;
            printVarOrder(varOrder, numInputs);
            varIidx++;
            printTTable(T, f);
            return; // TODO remove
        }

        if (reverse) {
            for (; varJidx >= 0; varJidx--) { // swap upward back to beginning
                swap(varOrder[varIidx], varOrder[varJidx], localT, T);
                // update varOrder array
                int tmp = varOrder[varIidx];
                varOrder[varIidx] = varOrder[varJidx];
                varOrder[varJidx] = tmp;
                printVarOrder(varOrder, numInputs);
                varIidx--;
            }
        }

        //=====================================================
        // [4] Move variable i to optimal position
        //=====================================================
        //for (j = 0; j < optPos; j++) { // Start with var i at pos 0
        //    varI = findVar(varOrder, i, numInputs);
        //    varJ = findVar(varOrder, j, numInputs);
        //    swap(varI, varJ, localT, varOrder);
        //}


    }

    //=====================================================
    // [5] Copy the local T table back into the T table
    //=====================================================

}


/**********************************************************************/
/*** MAIN FUNCTION ****************************************************/
/**********************************************************************/


int main(int argc, char* argv[])
{
	debug = false;
	t_blif_logic_circuit *circuit = NULL;

	if (argc < 2)
	{
		printf("Usage: %s <source BLIF file> <options>\r\n", argv[0]);
		return 0;
	}

	/* Read BLIF circuit. */
	printf("Reading file %s...\n",argv[1]);
	circuit = ReadBLIFCircuit(argv[1]);
    doSift = 0;
    if (argc == 3) {
        doSift = atoi(argv[2]);
    }

	if (circuit != NULL)
	{
		int index;

		/* build BDD for each function, one at a time. */
		printf("Building BDD for logic functions\n\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

            int i;
            for(i=0; i<function->input_count; i++)
            {
                int idx = function->inputs[i]->data.index;
                printf("input name = %s\n", circuit->primary_inputs[idx]->data.name);
            }
            //=====================================================
            // [1] set initial parameters
            //=====================================================
            numInputs = function->input_count;
            initT();
            initG();

            if(function->cube_count > 0)
            {
                //=====================================================
                // [2] merge cubes to set of PIs
                //=====================================================
                t_blif_cube ** PIs = (t_blif_cube **) malloc (function->cube_count * 
                        sizeof(t_blif_cube *));
                findPI(function, PIs); //f->set_of_cubes will be freed in findPI, PIs is the only valid list
                function->set_of_cubes = PIs;

                //=====================================================
                // [3] build ROBDD
                //=====================================================
                build(function->set_of_cubes, function->cube_count, 0, 
                    function->value);

                if (doSift) sift(function);
            }

            //=====================================================
            // [4] output to dot file
            //=====================================================
            printDOTfromNode(function, circuit->primary_inputs, index, numTRows-1);
            if(function->cube_count == 0)
               printf("%sWarning: No cubes in function! Skipping...\n%s", BYEL, KEND); 

            //=====================================================
            // [5] perform Apply ops
            //=====================================================

		}

		/* Finish. */
		printf("Done.\r\n");
		DeleteBLIFCircuit(blif_circuit);
	}
	else
	{
		printf("Error reading BLIF file. Terminating.\n");
	}
	return 0;
}

