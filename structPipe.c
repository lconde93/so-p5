#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include<sys/stat.h>

typedef struct memoriaCompartida{
	int bandera;	
	char datos[10];
} variableMem;

typedef struct memoriaPipe{
	int caso;
	char datos[150];
}variablePipe;

#define MAX_BUF 1024
int NO_PROCESOS = 2;
int NO_IO = 20;
#define NO_SEMAFOROS 8

int main() {
	int semId, shmId1, shmId2, shmId3, contadorLectura, contadorMemoria, processCounter, bytesLeidos, pipe;
	char *nombreSemaforo = {"/semaforo7"};	
	char *auxLectura;
	variablePipe* buffer;
	int childPid, grandsonPid;	
	char lectura[150] = "aaaa";	
	char *myfifo = "/tmp/myfifo11";	
	int counter, caso;
	int charCounter;	
	FILE  **file;
	
	/* creacion de pipe */
	mkfifo(myfifo, 0666);
	pipe = open(myfifo, O_RDWR);		
	for(processCounter = 0; processCounter < NO_PROCESOS; processCounter ++) {
		childPid = fork();
		switch(childPid) {
			case -1:
				printf("Error al crear el proceso hijo\n");
				break;
			case 0:
				if(processCounter == 0){					
					/*printf("a la espera de lectura\n");*/
					/*bytesLeidos = read(pipe, &buffer, sizeof(variablePipe*));*/
					/*bytesLeidos = read(pipe, lectura, sizeof(lectura));*/
					bytesLeidos = read(pipe, &caso, sizeof(int));
					printf("bytes leidos: %d, datos: %d\n", bytesLeidos, caso);
					bytesLeidos = read(pipe, lectura, sizeof(lectura));
					/*printf("bytes leidos: %d, tipo: %d, datos: %s\n", bytesLeidos, buffer->caso, buffer->datos);*/
					printf("bytes leidos: %d, datos: %s\n", bytesLeidos, lectura);
				}else{									
					strcpy(lectura, "hello");
					buffer = (variablePipe*) malloc(sizeof(variablePipe));
					buffer->caso = (0);
					printf("--%s\n", lectura);
					strcpy(buffer->datos, lectura);
					printf("buffer enviado %d, %s \n", buffer->caso, buffer->datos);
					/*bytesLeidos = write(pipe, &buffer, sizeof(variablePipe*));*/
					/*bytesLeidos = write(pipe, lectura, sizeof(lectura));*/
					bytesLeidos = write(pipe, &buffer->caso, sizeof(int));
					printf("%d bytes enviados\n", bytesLeidos);
					bytesLeidos = write(pipe, lectura, sizeof(lectura));
					printf("%d bytes enviados\n", bytesLeidos);
				}
				exit(0);
			default:			
				/*strcpy(lectura, "hello");
				buffer = (variablePipe*) malloc(sizeof(variablePipe));
				buffer->caso = (0);
				printf("--%s\n", lectura);
				strcpy(buffer->datos, lectura);
				printf("buffer enviado %d, %s \n", buffer->caso, buffer->datos);*/
				/*bytesLeidos = write(pipe, buffer, sizeof(variablePipe*));*/
				/*bytesLeidos = write(pipe, lectura, sizeof(lectura));
				printf("%d bytes enviadosn\n", bytesLeidos);					*/
				break;
		} 
	}	
			
	for(processCounter = 0; processCounter < NO_PROCESOS; processCounter ++) 
		wait(&childPid);
	close(pipe);
	unlink(myfifo);
	return(0);
}
