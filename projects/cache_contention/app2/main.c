#include "s3k.h"
#include <stdio.h>

#define MATRIX_SIZE 64 

void mm(int n, volatile int m1[n][n], volatile int m2[n][n], volatile int res[n][n])
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                int op1 = m1[i][k];
                int op2 = m2[k][j];
                sum += op1 * op2;
            }
            res[i][j] = sum;
        }
    }
}

void run_test()
{
    static volatile int m1[MATRIX_SIZE][MATRIX_SIZE];
    static volatile int m2[MATRIX_SIZE][MATRIX_SIZE];
    static volatile int res[MATRIX_SIZE][MATRIX_SIZE];

    // Initialize matrices
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            m1[i][j] = i + j;
            m2[i][j] = i - j;
        }

    // Run multiplication many times to ensure cache contention
    for (int repeat = 0; repeat < 5; ++repeat) {
        mm(MATRIX_SIZE, m1, m2, res);
    }
}

int main(void)
{
	printf("App 2 (PID 2) started\n");
	for (int i = 0; i < 50; ++i) {
		run_test();
		printf("App 2 iteration %d completed\n", i);
	}
	printf("App 2 finished\n");
	while(1) {
		s3k_sync();
	}
}
