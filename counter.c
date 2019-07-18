#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <papi.h>
#include <pthread.h>

#define PID 1211	

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
	int retval, cycle_, inst_;
	float cpi_;
	//initialize to use PAPI
	int EventSet = PAPI_NULL;
	PAPI_library_init(PAPI_VER_CURRENT);
	PAPI_create_eventset(&EventSet);
	PAPI_add_event(EventSet, PAPI_TOT_INS);
	retval = PAPI_attach(EventSet, (unsigned long)(((mem*)server)->pid));

	//PAPI count total cycle, total instruction
	int Events[2] = {PAPI_TOT_CYC, PAPI_TOT_INS};
	long long values[2];
	PAPI_start_counters(Events, 2);
	printf("hi\n");
	//sleep(1);
	PAPI_stop_counters(values, 2);
	cycle_ = values[0];
	inst_ = values[1];
	if(inst_ == 0)
		cpi_ = -1;
	else
		cpi_= (float)cycle_ / (float)inst_;
	((mem*)server)->cycle = cycle_;
	((mem*)server)->inst = inst_;
	((mem*)server)->cpi = cpi_;
	PAPI_shutdown();
	pthread_exit(NULL);
}

void* temp(void* server){
	printf("hi\n");
	return NULL;
}

int main(){
	mem* server = (mem*)malloc(sizeof(mem)*4);
	pthread_t tid[4];
	
	for(int i=0; i<4 ; i++){
//		server[i].pid = PID+i;
		pthread_create(&tid[i], NULL, thread, (void*)&(server[i]));
//		pthread_create(&tid[i], NULL, temp, NULL);
	}
	for(int i=0; i<4 ; i++){
		pthread_join(tid[i], NULL);
		printf("cpi of thread #%d: %f\n", i, server[i].cpi);
	}
	return 0;
}

