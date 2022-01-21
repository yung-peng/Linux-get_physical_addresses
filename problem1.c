#include<syscall.h>
#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>


int globalVar = 0;
int bss;
unsigned long heap_addr;

void* thread1(void* args);
void* thread2(void* args);

int main()
{
	int *heap = malloc(sizeof(int));
	*heap = 888;
	heap_addr = (unsigned long)heap;
	printf("=========main==============\n");
	int n = 0;
	unsigned long result = 0;
	printf("==========virtual address===========\n");
	printf("code: 0x%-16lx, data: 0x%-16lx, bss: 0x%-16lx, heap: 0x%-16lx\n", (unsigned long)main, (unsigned long)&globalVar, (unsigned long)&bss, (unsigned long)heap_addr);
	printf("share_lib: 0x%-16lx, stack: 0x%-16lx\n", (unsigned long)printf, (unsigned long)&n);
	printf("=========physical address===========\n");
	int a = syscall(334, (unsigned long)main, &result);
	printf("code: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&globalVar, &result);
	printf("data: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&bss, &result);
        printf("bss: 0x%-16lx, ", result);
	a = syscall(334, (unsigned long)heap_addr, &result);
        printf("heap: 0x%-16lx\n", result);
	a = syscall(334, (unsigned long)printf, &result);
        printf("share_lib: 0x%-16lx, ", result);
	a = syscall(334, (unsigned long)&n, &result);
        printf("stack: 0x%-16lx\n", result);

	pthread_t p1, p2;
	pthread_create(&p1, NULL, thread1, 0);
	pthread_create(&p2, NULL, thread2, 0);

	pthread_join(p1, NULL);
	pthread_join(p2, NULL);

	return 0;
}

void* thread1(void* args)
{
	int n = 0;
	int *heap = malloc(sizeof(int));
        *heap = 888;
        heap_addr = (unsigned long)heap;
        unsigned long result = 0;
	printf("==========Thread 1===============\n");
        printf("==========virtual address===========\n");
        printf("code: 0x%-16lx, data: 0x%-16lx, bss: 0x%-16lx, heap: 0x%-16lx\n", (unsigned long)main, (unsigned long)&globalVar, (unsigned long)&bss, (unsigned long)heap_addr);
        printf("share_lib: 0x%-16lx, stack: 0x%-16lx\n", (unsigned long)printf, (unsigned long)&n);
        printf("=========physical address===========\n");
        int a = syscall(334, (unsigned long)main, &result);
        printf("code: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&globalVar, &result);
        printf("data: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&bss, &result);
        printf("bss: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)heap_addr, &result);
        printf("heap: 0x%-16lx\n", result);
        a = syscall(334, (unsigned long)printf, &result);
        printf("share_lib: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&n, &result);
        printf("stack: 0x%-16lx\n", result);
	//while(1){};

	return 0;
}

void* thread2(void* args)
{
	sleep(3);
	int n = 0;
	int *heap = malloc(sizeof(int));
        *heap = 888;
        heap_addr = (unsigned long)heap;
        unsigned long result = 0;
	printf("=========thread2=============\n");
        printf("==========virtual address===========\n");
        printf("code: 0x%-16lx, data: 0x%-16lx, bss: 0x%-16lx, heap: 0x%-16lx\n", (unsigned long)main, (unsigned long)&globalVar, (unsigned long)&bss, (unsigned long)heap_addr);
        printf("share_lib: 0x%-16lx, stack: 0x%-16lx\n", (unsigned long)printf, (unsigned long)&n);
        printf("=========physical address===========\n");
        int a = syscall(334, (unsigned long)main, &result);
        printf("code: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&globalVar, &result);
        printf("data: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&bss, &result);
        printf("bss: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)heap_addr, &result);
        printf("heap: 0x%-16lx\n", result);
        a = syscall(334, (unsigned long)printf, &result);
        printf("share_lib: 0x%-16lx, ", result);
        a = syscall(334, (unsigned long)&n, &result);
        printf("stack: 0x%-16lx\n", result);
	//while(1){};
	return 0;
}
