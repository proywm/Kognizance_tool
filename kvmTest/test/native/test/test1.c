#define N (0xfffffff)
#define I 10000
volatile long * volatile a;
volatile long x;
int sum;


void __attribute__((noinline))Foo(int i){
	sum += i;	
}


int main(){
        int i = 0, j = 0;
        a = malloc(N * sizeof(long));
	for(;j<1000000;j++)
        for(;i<1000000;i++)
        	Foo(i);
	//printf("Alloc -> %llx\n" , a);
	printf("%d\n" , sum);
        return x;
}


