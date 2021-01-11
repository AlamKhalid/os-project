#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <netinet/in.h>

char taskBuffer[512];
char cpuBuffer[5];
char memBuffer[5];

void handlerTasks()
{
	char cmd[] = "whoami | xargs top -b -n 1 -u | awk '{if(NR>7)printf \"%-s %6s %-4s %-4s %-4s\\n\",$NF,$1,$9,$10,$2}' | sort -k 1";
	FILE *fp;
	int i = 0;
	key_t key = ftok("taskfile",75);
	int shmid = shmget(key,32768,0666|IPC_CREAT); 
	char *str = (char*) shmat(shmid,(void*)0,0); 
	if ((fp = popen(cmd, "r")) == NULL)
	{
		printf("Error opening pipe");
	}
	while (fgets(taskBuffer, 512, fp))
	{
		if(i == 0) {
			strcpy(str, taskBuffer);
		} else {
			strcat(str, taskBuffer);
		}
		i++;
	}
	shmdt(str);
	sleep(1);
}

void handlerCPU()
{
	char cmd[] = "top -bn1 | grep \"Cpu(s)\" | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk \'{printf(\"%0.1f\",100-$1);}\'";
	FILE *fp;
    key_t key = ftok("cpufile",65); 
	int shmid = shmget(key,32,0666|IPC_CREAT); 
	char *str = (char*) shmat(shmid,(void*)0,0); 
	if ((fp = popen(cmd, "r")) == NULL)
	{
		printf("Error opening pipe!\n");
	}
	while (fgets(cpuBuffer, 5, fp) != NULL)
	{
        strcpy(str, cpuBuffer);
	}
    shmdt(str);
	sleep(1);
}

void handlerMem()
{
	char cmd[] = "free -m | grep Mem | awk \'{printf(\"%0.1f\",$3/$2*100)}\'";
	FILE *fp;
    key_t key = ftok("memfile",70);
	int shmid = shmget(key,32,0666|IPC_CREAT); 
	char *str = (char*) shmat(shmid,(void*)0,0); 
	if ((fp = popen(cmd, "r")) == NULL)
	{
		printf("Error opening pipe!\n");
	}
	while (fgets(memBuffer, 5, fp) != NULL)
	{
		strcpy(str,memBuffer);
	}
    shmdt(str);
	sleep(1);
}

int main(int argc, char *argv[])
{
	while (1)
	{
		pthread_t sniffer_thread[3];
		int idx = 0;
		while (idx < 3)
		{
			if (idx == 0)
			{
				if (pthread_create(&sniffer_thread[idx++], NULL, handlerTasks))
				{
					perror("could not create thread");
					return 1;
				}
			}
			else if (idx == 1)
			{
				if(pthread_create( &sniffer_thread[idx++] , NULL ,  handlerCPU)){
					perror("could not create thread");
					return 1;
				}
			}
			else if (idx == 2)
			{
				if(pthread_create( &sniffer_thread[idx++] , NULL ,  handlerMem)){
					perror("could not create thread");
					return 1;
				}
			}
		}
		pthread_join(sniffer_thread[0], NULL);
		pthread_join(sniffer_thread[1], NULL);
		pthread_join(sniffer_thread[2], NULL);
		sleep(1);
	}
}
