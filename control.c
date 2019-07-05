#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <signal.h>

int fatal(char *s){
	perror(s);
	exit(1);
}



int main(){
	pid_t pid[4];
	for(int i = 0; i<4; i++){
		if((pid[i] = fork())==0){
			switch(i){
				case 0:
					printf("I'm child with #%d\n",i);
					execlp("taskset","taskset","-c","2","./batch/lbm_base.Xeon-gcc4.3","3000","reference.dat","0","0", NULL);
					//execlp("taskset","taskset","-c","2","./batch/mcf_base.Xeon-gcc4.3","./batch/inp.in", NULL);
					break;
				case 1:
					printf("I'm child with #%d\n",i);
					//execlp("taskset","taskset","-c","3","./batch/mcf_base.Xeon-gcc4.3","./batch/inp.in", NULL);
					execlp("taskset","taskset","-c","3","./batch/libquantum_base.Xeon-gcc4.3","1397","8", NULL);
					break;
				case 2:
					printf("I'm child with #%d\n",i);
					//execlp("taskset","taskset","-c","4","./batch/mcf_base.Xeon-gcc4.3","./batch/inp.in", NULL);
					execlp("taskset","taskset","-c","4","./batch/soplex_base.Xeon-gcc4.3","-s1","-e","-m45000", "./batch/pds-50.mps", NULL);
					break;
				case 3:
					printf("I'm child with #%d\n",i);
					execlp("taskset","taskset","-c","5","./batch/mcf_base.Xeon-gcc4.3","./batch/inp.in", NULL);
					break;
			}
			printf("Not in switch\n");
		}else{
			printf("I'm parent with #%d\n",i);
		}

	}
	sleep(10);
	//pause();
	kill(pid[0], SIGSTOP);
	kill(pid[1], SIGSTOP);
	kill(pid[2], SIGSTOP);
	kill(pid[3], SIGSTOP);
	sleep(10);
	//pause();
	kill(pid[0], SIGCONT);
	kill(pid[1], SIGCONT);
	kill(pid[2], SIGCONT);
	kill(pid[3], SIGCONT);
	sleep(10);
	kill(pid[0], SIGSTOP);
	kill(pid[1], SIGSTOP);
	kill(pid[2], SIGSTOP);
	kill(pid[3], SIGSTOP);
	sleep(10);
}
