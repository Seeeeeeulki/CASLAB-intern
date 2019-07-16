#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <signal.h>
#include <papi.h>

#define WINDOW 10	//decision interval
#define INIT_RATIO 0.6	//initial ratio of batch phase-in
#define TARGET 0.8	//target QoS, 0~1

#define PID 24373	//server PID

int fatal(char *s){
	perror(s);
	exit(1);
}

int main(){
	pid_t pid[5];
	float real_time, proc_time, ipc;
	long long ins;
	int retval;
	
	for(int i = 0; i<5; i++){
		if((pid[i] = fork())==0){
			switch(i){
				case 0:
					execlp("docker","docker", "run","--cpus=1","--cpuset-cpus=1","-e", "RECORDCOUNT=400000","-e", "OPERATIONCOUNT=400000", "--name","cassandra-client", "--net","serving_network","cloudsuite/data-serving:client","cassandra-server",NULL);
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

	//initialize to use PAPI
	int EventSet = PAPI_NULL;
	PAPI_library_init(PAPI_VER_CURRENT);
	PAPI_create_eventset(&EventSet);
	PAPI_add_event(EventSet, PAPI_TOT_INS);

	//from now, PAPI look server(by server-PID)
	retval = PAPI_attach(EventSet, (unsigned long)PID);
	printf("attach return: %d\n", retval);

	//PAPI count total cycle, total instruction
	int Events[2] = {PAPI_TOT_CYC, PAPI_TOT_INS};
	long long values[2];

	float ratio = INIT_RATIO; //phase in ratio, out=1-ratio
	long long cycle, inst;
	float ipc_out, ipc_in, qos;
	
	for(int i=0; i <1000; i++){
		for(int i=0; i<5; i++)
			printf("pid of child: %d\n",pid[i]);
		//batch phase out
		PAPI_start_counters(Events, 2);
		kill(pid[1],SIGSTOP);
		kill(pid[2],SIGSTOP);
		kill(pid[3],SIGSTOP);
		kill(pid[4],SIGSTOP);
		sleep((1-ratio) * WINDOW);
		PAPI_stop_counters(values, 2);
		cycle = values[0];
		inst = values[1];
		ipc_out = (float)inst / (float)cycle;

		//batch phase in
		PAPI_start_counters(Events, 2);
		kill(pid[1],SIGCONT);
		kill(pid[2],SIGCONT);
		kill(pid[3],SIGCONT);
		kill(pid[4],SIGCONT);
		kill(pid[4],SIGCONT);
		sleep(ratio * WINDOW);
		PAPI_stop_counters(values, 2);
		cycle = values[0];
		inst = values[1];
		ipc_in = (float)inst / (float)cycle;

		//calculate new ratio
		qos = (ratio*ipc_in + (1-ratio)*ipc_out)/ipc_out;
		ratio = ratio - (TARGET - qos)/TARGET; 

		//Even if strange value appear at ipc, DO NOT AFFECT PROGRAM EXECUTION
		if (ratio > 0.95 )
			ratio = 0.95;

		printf("\n========================\n");
		printf("ipc in : %f\n",ipc_in);
		printf("ipc out : %f\n",ipc_out);
		printf("qos: %f\n", qos);
		printf("new ratio : %f\n", ratio);
		PAPI_shutdown();
	}

	return 0;
}
