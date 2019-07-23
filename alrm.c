#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define MAX_INS 4000000
#define MAX_CYC 4500000
#define MAX_CPU 50000

void sig_handler (int signo){
	FILE *fp = fopen("out.txt", "r");
	if( fp == NULL){
		printf("DO NOT EXIST FILE\n");
		exit(1);
	}else{
		int ins, cyc, cpu;
		int result[3] = {0,};
		for(int i=0; i<100; i++){
			fscanf(fp, "%d, %d, %d\n", &ins, &cyc, &cpu);
			result[0] += (MAX_INS-ins)*(MAX_INS-ins);
			result[1] += (MAX_CYC-cyc)*(MAX_CYC-cyc);
			result[2] += (MAX_CPU-cpu)*(MAX_CPU-cpu);
		}
		printf("=======output=======\n");
		printf("instruction distance : %d\n", result[0]);
		printf("cycle distance : %d\n", result[1]);
		printf("cpu distance : %d\n", result[2]);
	}

	alarm(1);
}

int main(){
	signal(SIGALRM, sig_handler);
	alarm(1);
	while(1)
		;
}
