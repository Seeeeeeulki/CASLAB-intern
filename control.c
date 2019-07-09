#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <signal.h>
#include <papi.h>

#define WINDOW 3
#define INIT_RATIO 0.6
#define TARGET 0.1724

#define PID 24373 

int fatal(char *s){
	perror(s);
	exit(1);
}

int main(){
	pid_t pid[4];
	float real_time, proc_time, ipc;
	long long ins;
	int retval;
	
	for(int i = 0; i<5; i++){
		if((pid[i] = fork())==0){
			switch(i){
				case 0:
					//execlp("docker","docker", "run","--cpus=1","--cpuset-cpus=1","-e", "RECORDCOUNT=1000000","-e", "OPERATIONCOUNT=1000000", "--name","cassandra-client", "--net","serving_network","cloudsuite/data-serving:client","cassandra-server",NULL);
					execlp("ls","ls", NULL);
					//execlp("taskset","taskset","-c","2","./batch/lbm_base.Xeon-gcc4.3","3000","reference.dat","0","0", NULL);
					break;
				case 1:
					execlp("ls","ls", NULL);
					//execlp("taskset","taskset","-c","2","./batch/lbm_base.Xeon-gcc4.3","3000","reference.dat","0","0", NULL);
					break;
				case 2:
					execlp("ls","ls", NULL);
					//execlp("taskset","taskset","-c","3","./batch/libquantum_base.Xeon-gcc4.3","1397","8", NULL);
					break;
				case 3:
					execlp("ls","ls", NULL);
					//execlp("taskset","taskset","-c","4","./batch/soplex_base.Xeon-gcc4.3","-s1","-e","-m45000", "./batch/pds-50.mps", NULL);
					break;
				case 4:
					execlp("ls","ls", NULL);
					//execlp("taskset","taskset","-c","5","./batch/mcf_base.Xeon-gcc4.3","./batch/inp.in", NULL);
					break;
			}
		}
	}
	
	sleep(1);	//consider time for start

	/*
	if((retval = PAPI_ipc(&real_time, &proc_time, &ins, &ipc)) < PAPI_OK ) {
		printf("[start] Could not get PAPI_ipc value: PAPI error %d\n", retval);
		exit(1);
	}
	*/
	int EventSet = PAPI_NULL;
	PAPI_library_init(PAPI_VER_CURRENT);
	PAPI_create_eventset(&EventSet);
	PAPI_add_event(EventSet, PAPI_TOT_INS);
	retval = PAPI_attach(EventSet, (unsigned long)PID);
	printf("attach error: %d\n", retval);


	int Events[2] = {PAPI_TOT_CYC, PAPI_TOT_INS};
	long long values[2];

	float ratio = INIT_RATIO;
	
	for(int i=0; i <1000; i++){
		//PAPI_start(EventSet);
		//retval = PAPI_attach(EventSet, (unsigned long)pid[3]);
		//printf("attach error: %d\n", retval);
		//PAPI_ipc(&real_time, &proc_time, &ins, &ipc);
		PAPI_start_counters(Events, 2);
		kill(pid[1],SIGSTOP);
		kill(pid[2],SIGSTOP);
		//kill(pid[3],SIGSTOP);
		kill(pid[4],SIGSTOP);
		sleep(ratio * WINDOW);
		kill(pid[1],SIGCONT);
		kill(pid[2],SIGCONT);
		//kill(pid[3],SIGCONT);
		kill(pid[4],SIGCONT);
		sleep((1 - ratio) * WINDOW);
		//PAPI_detach(EventSet);
		//PAPI_ipc(&real_time, &proc_time, &ins, &ipc);
		//printf("detach result: %d\n", EventSet);
		//printf (" real time : %f\n proc time : %f\n instruction : %lli\n ipc : %f\n", real_time, proc_time, ins, ipc);
		//ratio = ratio + (TARGET - ipc)/TARGET; 
		//printf("new ratio : %f\n", ratio);
		//printf("PID: %d\n",PID);
		//PAPI_stop(EventSet, values);
		PAPI_stop_counters(values, 2);
		printf("cycle: %lld\n",values[0]);
		printf("instruction: %lld\n",values[1]);
		PAPI_shutdown();
	}

	//record ipc-result
	return 0;
}
