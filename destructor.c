#include <stdio.h>
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

typedef struct memoriaCompartida{
	int bandera;	
	int datos;
} variableMem;

int crearSemaforo(char*);
int inicializarSemaforo(int, int, int);
int destruirSemaforo(int);
int crearMemoriaCompartida(int, variableMem***);
int obtenerValorSemaforo(int, int);
int operacionSemaforo(int, int, int);


int main() {
	int semId, shmId1, shmId2, shmId3, contadorEscritura, contadorMemoria;
	char *nombreSemaforo = {"/semaforo7"};		
 	/* variableMem *mem1, *mem2, *mem3;  */
	variableMem **arregloMem;
	/* arregloMem = (variableMem **) malloc(3 * sizeof(variableMem)); */
	/* arregloMem[0] = mem1;
	arregloMem[1] = mem1;	 		
	arregloMem[2] = mem1; */

	semId = crearSemaforo(nombreSemaforo);
	
	if(semId != -1) {
		printf("Proceso con ID: %d\n", getpid());
		shmId1 = crearMemoriaCompartida(1, &arregloMem);
		/* shmId2 = crearMemoriaCompartida(2, mem2);
		shmId3 = crearMemoriaCompartida(3, mem3); */		

		
        for(contadorMemoria = 0; contadorMemoria < 5; contadorMemoria ++) {
            printf("Valor semaforo no %d: %d\n", contadorMemoria, obtenerValorSemaforo(semId, contadorMemoria));
        }
		getchar();
        shmdt(&shmId1);
        /* shmdt(&shmId2);
        shmdt(&shmId3); */
        shmctl (shmId1 , IPC_RMID , 0);
        /* shmctl (shmId2 , IPC_RMID , 0);
        shmctl (shmId3 , IPC_RMID , 0); */
		destruirSemaforo(semId);
	}	
	else{
		printf("Error al crear los semaforos\n");
	}
	return(0);
}

int crearSemaforo(char* nombreSemaforo) {
	/*                         P  C  M1 M2 M3  */
	int valoresIniciales[5] = {3, 0, 1, 1, 1};
	int semId = -1, contador, bandera, banderaDestruir;
	key_t key = ftok("/bin/ls", 70);
	semId = semget(key, 5, 0666|IPC_CREAT|IPC_EXCL) ;
	if (semId == -1) {
		switch(errno) {
    		case EEXIST:    			
    			semId = semget(key, 5, 0666);
    			printf("Se liga a los semaforos\n");
    			break;
			default:
	    		printf("Error desconocido\n");
	    		break;
    	}
	}else{
		printf("Se crearon los semaforos\n");
		for (contador = 0; contador < 5; contador++) {
			bandera = -1;
			bandera = inicializarSemaforo(semId, contador, valoresIniciales[contador]);
			if (bandera == -1) {
				printf("Error al inicializar el semaforo %d\n", contador + 1);
				banderaDestruir = destruirSemaforo(semId);
				if(banderaDestruir == -1)
					printf("Error al destruir el semaforo");
				semId = -1;
				break;
			}				
		}
	}	

	return semId;
}

int inicializarSemaforo(int semId, int noSem, int valor) {
	union semun arg;
	arg.val = valor;
	return (semctl(semId, noSem, SETVAL, arg));
}

int crearMemoriaCompartida(int noMem, variableMem*** value) {
	// ftok to generate unique key
    key_t key = ftok("/bin/ls", (65 + noMem));
	variableMem** aux;
 
    // shmget returns an identifier in shmid
    int shmId = shmget(key, 3*sizeof(variableMem*), 0666|IPC_CREAT|IPC_EXCL);
    if(shmId == -1) {
    	switch(errno) {
    		case EEXIST:    			
    			shmId = shmget(key,3*sizeof(variableMem*),0666);
    			printf("Se liga a memoria compartida\n");
				aux = (variableMem**) shmat(shmId,(void*)0,0);
    			break;
			default:
	    		printf("Error desconocido\n");
	    		break;
    	}
    }  
	else{
		printf("Se creo la memoria compartida\n");
		aux = (variableMem**) shmat(shmId,(void*)0,0);
		aux[0] = (variableMem*) malloc(sizeof(variableMem));
		aux[1] = (variableMem*) malloc(sizeof(variableMem));
		aux[2] = (variableMem*) malloc(sizeof(variableMem));
	}			
	*value = aux;		
	return shmId;    
}

int destruirSemaforo(int semId) {
	return (semctl(semId, 0, IPC_RMID));
}

int obtenerValorSemaforo(int semId, int noSem) {
	return(semctl(semId, noSem, GETVAL));
}

/* 
	recibe el id del semaforo, no de semaforo que se va a modificar
	y el tipo, 0 para decrementar y 1 para aumentar
*/
int operacionSemaforo(int semId, int noSem, int tipo) {
	int auxiliar = (tipo == 1 ? 1 : -1 );
	struct sembuf op_disminuir = {noSem, auxiliar, 0};
	int bandera = semop(semId, &op_disminuir, 1);
	if(bandera != 0) {
		if(tipo == 0)
			printf("Error al decrementar el semaforo no. %d\n", noSem);
		else
			printf("Error al aumentar el semaforo no. %d\n", noSem);
	}
	return bandera;
}