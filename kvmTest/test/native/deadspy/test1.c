#define N (0xfffffff)
#define I 10000
volatile long * volatile a;
volatile long x;

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


int main(){
        int i = 0;
        a = malloc(N * sizeof(long));
        for(;i<100000;i++)
        Foo();
	printf("Alloc -> %llx\n" , a);
        return x;
}


