/*------------------------------------------------------------------
* Multi-threaded version
*-------------------------------------------------------------------
*/
#include <asm/unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <linux/kernel.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <strings.h>
#include <time.h>

//******************************************* Workload to test *******************************************************************
#define SHELLSCRIPT "\
#/bin/bash \n\
ping localhost -c 1 >nul\n\
"
/* Test example starts */
#define MATRIX_SIZE 8
//512
static double a[MATRIX_SIZE][MATRIX_SIZE];
static double b[MATRIX_SIZE][MATRIX_SIZE];
static double c[MATRIX_SIZE][MATRIX_SIZE];

static void naive_matrix_multiply(int quiet) {

  double s;
  int i,j,k;
  #pragma omp parallel for
  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      a[i][j]=(double)i*(double)j;
      b[i][j]=(double)i/(double)(j+5);
    }
  }

  #pragma omp parallel for
  for(j=0;j<MATRIX_SIZE;j++) {
     for(i=0;i<MATRIX_SIZE;i++) {
        s=0;
        for(k=0;k<MATRIX_SIZE;k++) {
           s+=a[i][k]*b[k][j];
        }
        c[i][j] = s;
     }
  }
  s=0.0;
  #pragma omp parallel for
  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      s+=c[i][j];
    }
  }

  if (!quiet) printf("Matrix multiply sum: s=%lf\n",s);

  return;
}

//open and close a file. calls naive MM
static void workload1(int count)
{
        FILE *fp;
        char buffer[] = { 'x' , 'y' , 'z', 'a', 'b' };
        while(count--)
        {
                //cpu workload
                naive_matrix_multiply(1);

                //FS workload
#if 0
                fp = fopen("dummy.txt", "wb");
                if(fp == NULL)
                {
                         printf("Error opening file\n");
                         exit(1);
                }

                fwrite(buffer, sizeof(char), sizeof(buffer), fp);
                fclose(fp);
#endif
                //network workload
                //system(SHELLSCRIPT);
      }
}

//*********************************************************** Main ****************************************************************************
int
main(int argc, char **argv)
{
        long long count = 10;
        int outerLoop = 1000000;
        while(outerLoop--)
        {
            //workload
            workload1(60);

        }

}

