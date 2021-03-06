/**
 * CITS3402 Project 1
 *
 * Written by Nicholas Cannon (22241579)
 */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "logResult.h"
#include "matrixOp.h"
#include "parseMatrix.h"

int main(int argc, char **argv) {
    int lflag = 0;
    int loadFlag = 0;
    int opFlag = 0;
    int threadNum = 0;  // non zero if set by CLA
    int optIndex;
    char *mat1Path, *mat2Path;
    char matOp[3];  // two chars + null byte
    double loadTime = 0.0;
    double opTime = 0.0;
    int status = 0;
    clock_t start, end;

    // quick error check
    if (argc < 2) {
        printf("Invalid comand line args!\n");
        return EXIT_FAILURE;
    }

    /* Parse CLA's */
    strncpy(matOp, (argv[1] + 2), 3);  // matrix operation always first CLA!
    for (optIndex = 0; optIndex < argc; optIndex++) {
        if (argv[optIndex][0] == '-') {
            switch (argv[optIndex][1]) {
                case 'l':
                    lflag = 1;
                    break;
                case 't':
                    threadNum = atoi(argv[optIndex + 1]);
                    break;
                case 'L':
                    loadFlag = 1;
                    break;
                case 'O':
                    opFlag = 1;
                    break;
                case 'f':
                    mat1Path = argv[optIndex + 1];
                    if (strcmp(matOp, "ad") || strcmp(matOp, "mm")) {
                        // two matrices provided
                        mat2Path = argv[optIndex + 2];
                    }
            }
        }
    }

    // check if number of threads has been specified
    if (threadNum != 0) omp_set_num_threads(threadNum);

    /* MATRIX OPERATION SWITCH */
    if (strcmp(matOp, "sm") == 0) {
        /* SCALAR MULTIPLICATION */
        float scalar = atof(argv[2]);  // argument just after --sm
        COO *mat = malloc(sizeof(COO));
        COO *ans = malloc(sizeof(COO));
        
				// read in matrix
        start = clock();
        readCOO(mat1Path, mat);
        end = clock();
        loadTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (status == -1) {
            // failed to open file!
            free(mat);
            free(ans);
            printf("Failed to open matrix: %s\n", mat1Path);
            return EXIT_FAILURE;
        }

        // Do opertation
        start = clock();
        scalarMultiplication(mat, ans, scalar);
        end = clock();
        opTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (lflag) {
            logCOO(matOp, mat1Path, NULL, threadNum, ans, loadTime, opTime,
                   argv[2]);
        }

        // clean up our allocations
        free(mat);
        free(ans);
    } else if (strcmp(matOp, "tr") == 0) {
        /* MATRIX TRACE */
        COO *mat = malloc(sizeof(COO));

        // read in matrix
        start = clock();
        status = readCOO(mat1Path, mat);
        end = clock();
        loadTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (status == -1) {
            // failed to open file!
            free(mat);
            printf("Failed to open matrix: %s\n", mat1Path);
            return EXIT_FAILURE;
        }

        // trace is only defined for square matrices (rows = cols)
        if (mat->rows != mat->cols) {
            printf("Invalid operation: Trace on non square matrix\n");
            free(mat);
            return EXIT_FAILURE;
        }

        // Do opertation
        if (mat->type == MAT_FLOAT) {
            float ans;

            start = clock();
            trace_FLOAT(mat, &ans);
            end = clock();
            opTime = (double)(end - start) / CLOCKS_PER_SEC;

						if (lflag) {
								logInt(matOp, mat1Path, threadNum, ans, loadTime, opTime);
						}
        } else {
            int ans;

            start = clock();
            trace_INT(mat, &ans);
            end = clock();
            opTime = (double)(end - start) / CLOCKS_PER_SEC;

						if (lflag) {
								logInt(matOp, mat1Path, threadNum, ans, loadTime, opTime);
						}
        }

        free(mat);
    } else if (strcmp(matOp, "ad") == 0) {
        /* MATRIX ADDITION */
        int status2 = 0;
        COO *mat1 = (COO *)malloc(sizeof(COO));
        COO *mat2 = (COO *)malloc(sizeof(COO));
        COO *ans = (COO *)malloc(sizeof(COO));

        if (strcmp(mat1Path, mat2Path) == 0) {
            // no point reading same file twice!
            start = clock();
            status = readCOO(mat1Path, mat1);
            *mat2 = *mat1;
            end = clock();
        } else {
            // read in both matrices
            start = clock();
            status = readCOO(mat1Path, mat1);
            status2 = readCOO(mat2Path, mat2);
            end = clock();
        }
        loadTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (status == -1 || status2 == -1) {
            // failed to open file!
            free(mat1);
            free(mat2);
            free(ans);
            printf("Failed to open matrices: %s and %s\n", mat1Path, mat2Path);
            return EXIT_FAILURE;
        }

        // check equal dimensions
        if (mat1->cols != mat2->cols || mat1->rows != mat2->rows) {
            free(mat1);
            free(mat2);
            free(ans);
            printf("Matrices must be same dimensions for addition!\n");
            return EXIT_FAILURE;
        }

        start = clock();
        matrixAddition(mat1, mat2, ans);
        end = clock();
        opTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (lflag) {
            logCOO(matOp, mat1Path, mat2Path, threadNum, ans, loadTime, opTime,
                   NULL);
        }

        // free up memory
        free(mat1);
        free(mat2);
        free(ans);
    } else if (strcmp(matOp, "ts") == 0) {
        /* MATRIX TRANSPOSE */
        COO *mat = malloc(sizeof(COO));
        COO *ans = malloc(sizeof(COO));

        // read in matrix
        start = clock();
        readCOO(mat1Path, mat);
        end = clock();
        loadTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (status == -1) {
            // failed to open file!
            free(mat);
            free(ans);
            printf("Failed to open matrix: %s\n", mat1Path);
            return EXIT_FAILURE;
        }

        // Do opertation
        start = clock();
        matrixTranspose(mat, ans);
        end = clock();
        opTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (lflag) {
            logCOO(matOp, mat1Path, NULL, threadNum, ans, loadTime, opTime,
                   NULL);
        }

        // clean up our allocations
        free(mat);
        free(ans);
    } else if (strcmp(matOp, "mm") == 0) {
        /* MATRIX MULTIPLICATION */
        int status2 = 0;
        CS *mat1 = (CS *)malloc(sizeof(CS));
        CS *mat2 = (CS *)malloc(sizeof(CS));
        CS *ans = (CS *)malloc(sizeof(CS));

        // read in both matrices
        start = clock();
        status = readCSR(mat1Path, mat1);
        status2 = readCSC(mat2Path, mat2);
        end = clock();
        loadTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (status == -1 || status2 == -1) {
            // failed to open file!
            free(mat1);
            free(mat2);
            free(ans);
            printf("Failed to open matrices: %s and %s\n", mat1Path, mat2Path);
            return EXIT_FAILURE;
        }

        start = clock();
        matrixMultiplication(mat1, mat2, ans);
        end = clock();
        opTime = (double)(end - start) / CLOCKS_PER_SEC;

        if (lflag) {
            logCSR(matOp, mat1Path, mat2Path, threadNum, ans, loadTime, opTime,
                   NULL);
        }

        // free up memory
        free(mat1);
        free(mat2);
        free(ans);
    } else if (strcmp(matOp, "ti") == 0) {
        /* TIMING ONLY CSR */
        CS *mat1 = (CS *)malloc(sizeof(CS));
        start = clock();
        status = readCSR(mat1Path, mat1);
        end = clock();

        loadTime = (double)(end - start) / CLOCKS_PER_SEC;

        free(mat1);
    } else {
        /* INVALID OPERATION */
        printf("Invalid matrix operation!\n");
        return EXIT_FAILURE;
    }

		// print out times to stdout if required
		if (loadFlag) {
				printf("%f\n", loadTime);
		}
		if (opFlag) {
				printf("%f\n", opTime);
		}

    return EXIT_SUCCESS;
}
