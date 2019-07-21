#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <papi.h>
#include <pthread.h>

#define PID 1126	

int fatal(char *s){
	perror(s);
	exit(1);
}

typedef struct memcached{
	pid_t pid;
	int cycle;
	int inst;
	float cpi;
}mem;

void* thread(void* server){
	int cycle_, inst_;
	float cpi_;
	//initialize to use PAPI
	int EventSet = PAPI_NULL;
	int Events[2] = {PAPI_TOT_CYC, PAPI_TOT_INS};
	PAPI_create_eventset(&EventSet);
	PAPI_add_events(EventSet, Events, 2);


	//PAPI_add_event(EventSet, PAPI_TOT_CYC);
	PAPI_attach(EventSet, (unsigned long)(((mem*)server)->pid));

	//PAPI count total cycle, total instruction
	long long values[2];


//	long long start, end;
	PAPI_start(EventSet);
		
	//start = PAPI_get_real_cyc();
	sleep(5);
	//end = PAPI_get_real_cyc();
	
	//cycle_ = end - start;
	PAPI_stop(EventSet, values);
	cycle_ = values[0];
	inst_ = values[1];
	if(inst_ == 0){
		printf("can not get instruction counter\n");
		exit(0);
	}
	cpi_= (float)cycle_ / (float)inst_;
	((mem*)server)->cycle = cycle_;
	((mem*)server)->inst = inst_;
	((mem*)server)->cpi = cpi_;
	pthread_exit(NULL);
}

void* temp(void* server){
	printf("hi\n");
	return NULL;
}

int main(){
	mem* server = (mem*)malloc(sizeof(mem)*4);
	pthread_t tid[4];
	PAPI_library_init(PAPI_VER_CURRENT);

	for(int i=0; i<4 ; i++){
		server[i].pid = PID+i;
		pthread_create(&tid[i], NULL, thread, (void*)&(server[i]));
//		pthread_create(&tid[i], NULL, temp, NULL);
	}
	for(int i=0; i<4 ; i++){
		pthread_join(tid[i], NULL);
		printf("cpi of pid #%d: %f\n", server[i].pid, server[i].cpi);
		printf("cycle : %d , inst : %d \n\n", server[i].cycle, server[i].inst);
	}
	return 0;
}

