#include <upc.h>
#include <stdio.h>

/* ---------   Defines    ---------- */// Shared matrixes
shared  float A[THREADS];
shared  float B[THREADS]; 
shared  float C[THREADS];


// Index of master id
#define MASTER 0


// Variable to print logs 
// 0: logs invisible
// 1: logs visible
bool VERBOSE = 0;

/* ---------    Function definitions    ---------- */

/*
 * Function:  print_matrix 
 * --------------------
 * prints out given matrix:
 *
 *  mat: Matrix to print out
 *  dim: dimention of the matrix
 *  returns: void
 */
void print_matrix(shared float mat[], const int dim);

/*
 * Function: initialize_matrix
 * ----------------------------
 *   One dimentional matrix created from csv file (comma delivered).
 *
 *   file_pointer: Pointer to *.csv file from which will be imported matrix
 *   matrix_data: Pointer to matrix which have to be filled.
 *
 *   returns: Void
 */
void initialize_matrix(FILE *file_pointer, shared float mat[]);


/*
 * Function:  save_matrix 
 * --------------------
 * saves matrix:
 *
 *  file_pointer: Pointer to *.csv file from which will be imported matrix
 *  mat: Matrix to print out
 *  dim: dimention of the matrix
 *
 *  returns: void
 */
void save_matrix(FILE *file_pointer,
                const int dim,
                shared float mat[]);


/* ---------    Function declarations    ---------- */

int main (int argc, char *argv[])
{
	int myid;
	int dim = (int)sqrt(THREADS);

	FILE* file_pointer;

    // ---------    Set verbose    --------- */
    if(argc > 1 && strcmp(argv[1], "-v") == 0)
    {
        VERBOSE = 1;
    }

	if(MYTHREAD == MASTER)
	{
		/* ---------    Initialize matrices    ---------- */
		file_pointer = fopen("A.csv", "r");
		initialize_matrix(file_pointer, A);
		if(VERBOSE)
		{
			fprintf(stdout, "Matrix A: \n");
			print_matrix(A, dim);
		}
		fclose(file_pointer);

		file_pointer = fopen("B.csv", "r");
		initialize_matrix(file_pointer, B);
		if(VERBOSE)
		{
			fprintf(stdout, "Matrix B: \n");
			print_matrix(B, dim); 
		}
		fclose(file_pointer);

		fflush(stdout);
	}

	upc_barrier;

	upc_forall(myid = 0; myid < THREADS; myid++; &C[myid])
	{
		//dim
		int i = myid / dim;
		//column
		int j = myid % dim;
	
		//initial skewing
		int Ai = i;
		int Aj = (j + i) % dim;
		int Bi = (i + j) % dim;
		int Bj = j;
	
		for(int shift = 0; shift < dim; shift++) 
		{
			Aj = (Aj + 1) % dim;
			Bi = (Bi + 1) % dim;

			int indexA = Ai * dim + Aj;
			int indexB = Bi * dim + Bj;
			C[myid] += A[indexA] * B[indexB];
		}
	}

	upc_barrier;

	if(MYTHREAD == MASTER)
	{
		if(VERBOSE)
		{
			fprintf(stdout, "Output matrix: \n");
			print_matrix(C, dim); 
		}
		file_pointer = fopen("C.csv","w");
		save_matrix(file_pointer, dim, C);
		fclose(file_pointer);

	}
	return 0;
}


void initialize_matrix(FILE *file_pointer, shared float mat[])
{
    char buffer[1024];
    char *line,
         *record;
    int row = 0,
        col = 0;
    while((line = fgets(buffer,sizeof(buffer), file_pointer)) != NULL)
    {
        if(row == 0)
        {
            for(int i = 0; i < strlen(line); i++)
            {  
                if(line[i] == ',')
                {
                    col++;  
                }
            }  
        }
        row++;
    }

    fseek(file_pointer, 0, SEEK_SET);

    int k=0;
    int i = 0;
    while((line = fgets(buffer,sizeof(buffer), file_pointer)) != NULL && i++ < row)
    {
        record = strtok(line, ",");
        int j = 0;
        while(record != NULL && j++ < col)
        {
            mat[k] = atof(record);
            record = strtok(NULL,",");
            k++;
        }
    }
}

void print_matrix(shared float mat[], const int dim)
{
    int cnt = 0;
    for(int i = 0; i < dim; i++) 
    {
        for(int j = 0; j < dim; j++)
        {
            if(VERBOSE)
            {
                fprintf(stdout, "%1.2f\t", mat[cnt]);
            }
            cnt++; 
        }
        if(VERBOSE)
        {
            fprintf(stdout, "\n");
        }
    }
}

void save_matrix(FILE *file_pointer,
                const int dim,
                shared float mat[])
{
    int cnt = 0;
    if(VERBOSE)
    {
        fprintf(stdout, "Saving matrix to output file.\n");
    }
    for(int i = 0; i < dim; i++) 
    {
        for(int j = 0; j < dim; j++)
        {
            fprintf(file_pointer, "%1.2f,", mat[cnt]);
            cnt++; 
        }
        fprintf(file_pointer, "\n");
    }
}