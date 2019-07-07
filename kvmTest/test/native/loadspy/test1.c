#include <stdio.h>
#include <stdlib.h>
enum Benchmark {DEADSTORE, REDSTORE, REDLOAD};
typedef enum Benchmark Benchmark;

#define N (0xffff)
#define I 10000
volatile long * volatile a;
volatile long * volatile b;
volatile long x;
volatile long delta;

void __attribute__((noinline)) Dead(){
        int i;
        for(i = 0 ; i < I; i++){
                        a[i] = i;
        }
}

void __attribute__((noinline)) Kill(){
        int i;
        for(i = 0 ; i < I; i++){
                        a[i] = i;
        }
}


void __attribute__((noinline)) Use(){
        int i;
        for(i = 0 ; i < I; i++){
                        x +=a[i];
        }
}


void __attribute__((noinline))Foo(){
        Dead();
        Kill();
        Use();
}

void __attribute__((noinline))DeadStore(void){
        Dead();
        Kill();
        Use();
}

void __attribute__((noinline))RedundantStore(void){
        Dead();
        Use();
        Kill();
}

void __attribute__((noinline))RedundantLoad(void){
        int i;
        while (x < I ) {
                for (i = 0; i < 1000; i ++) {
                        x += a[i] + b[i] * delta ;
                }
                delta -=  (x / 10);
        }

}

int Run(Benchmark bench){

        int i = 0;
        a = malloc(N * sizeof(long));
        switch(bench)
        {
                case DEADSTORE:
                {
			printf("DeadStore\n");
                        for(;i<100000;i++)
                        {
                                DeadStore();
                        }
                }
                break;
                case REDSTORE:
                {
			printf("RedStore\n");
                        for(;i<100000;i++)
                        {
                                RedundantStore();
                        }
                }
                break;
                case REDLOAD:
                {
			printf("Red Load\n");
                        b = malloc(N * sizeof(long));
                        for(i = 0 ; i < N; i++){
                                a[i] = i + 1;
                                b[i] = i + 1;
                        }
                        delta = 1;
                        x = 1;
                        for(i = 0; i < 1000000; i++)
                        {
                                x = 1;
                                RedundantLoad();
                        }
                        free(b);
                }
                break;
        }
        printf("Alloc -> %llx\n" , a);
        free(a);
        return x;
}

int main(int argc, char **argv)
{
  int iter = 1, i = 0;
  if (argc <= 1) {
    printf("Must provide a string to give to system call and times to execute(optional).\n");
    return -1;
  }
  char *arg = argv[1];
  if(argc == 3)
        iter = atoi(argv[2]);
  else iter = 1;
  printf("Running with \"%s\".\n", arg);

  long res = 0;
  for(i = 0; i < iter; i++)
        res = Run((Benchmark)atoi(argv[1]));
  printf("System call returned %ld.\n", res);
  return res;
}


