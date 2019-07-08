#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <signal.h>
#include <papi.h>

#define WINDOW 5

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
					execlp("docker","docker", "run","-e", "RECORDCOUNT=1000000","-e", "OPERATIONCOUNT=1000000", "--name","cassandra-client", "--net","serving_network","cloudsuite/data-serving:client","cassandra-server",NULL);
					break;
				case 1:
					execlp("taskset","taskset","-c","2","./batch/lbm_base.Xeon-gcc4.3","3000","reference.dat","0","0", NULL);
					break;
				case 2:
					execlp("taskset","taskset","-c","3","./batch/libquantum_base.Xeon-gcc4.3","1397","8", NULL);
					break;
				case 3:
					execlp("taskset","taskset","-c","4","./batch/soplex_base.Xeon-gcc4.3","-s1","-e","-m45000", "./batch/pds-50.mps", NULL);
					break;
				case 4:
					execlp("taskset","taskset","-c","5","./batch/mcf_base.Xeon-gcc4.3","./batch/inp.in", NULL);
					break;
			}
		}
	}
	
	sleep(10);	//consider time for start

	if((retval = PAPI_ipc(&real_time, &proc_time, &ins, &ipc)) < PAPI_OK ) {
		printf("[start] Could not get PAPI_ipc value: PAPI error %d\n", retval);
		exit(1);
	}
	
	float RATIO = 0.6;
	FILE* f_ipc = fopen("count.txt","w");
	
	for(int i=0; i <1000; i++){
		kill(pid[1],SIGSTOP);
		kill(pid[2],SIGSTOP);
		kill(pid[3],SIGSTOP);
		kill(pid[4],SIGSTOP);
		sleep(RATIO * WINDOW);
		kill(pid[1],SIGCONT);
		kill(pid[2],SIGCONT);
		kill(pid[3],SIGCONT);
		kill(pid[4],SIGCONT);
		sleep((1 - RATIO) * WINDOW);
		PAPI_ipc(&real_time, &proc_time, &ins, &ipc);
		printf (" real time : %f\n proc time : %f\n instruction : %lli\n ipc : %f\n", real_time, proc_time, ins, ipc);
	}

	//record ipc-result
	PAPI_shutdown();
	return 0;
}
