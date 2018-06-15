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
	char datos[10];	
} variableMem;

int NO_PROCESOS = 20;
int NO_IO = 20;
#define NO_SEMAFOROS 8

int crearSemaforo(char*);
int inicializarSemaforo(int, int, int);
int destruirSemaforo(int);
int crearMemoriaCompartida(int, variableMem**);
int obtenerValorSemaforo(int, int);
int operacionSemaforo(int, int, int);
void imprimirSemaforos(int);
void inicializarMemorias(variableMem***, variableMem**, variableMem**, variableMem**);


int main() {
	int semId, shmId1, shmId2, shmId3, contadorEscritura, contadorMemoria, processCounter;
	char *nombreSemaforo = {"/semaforo7"};
	char auxEscritura[10] = "\0";	
	int childPid;	
	int contadorLetra; 	
	variableMem *arregloMem;		

	semId = crearSemaforo(nombreSemaforo);
	
	if(semId != -1) {
		printf("Proceso con ID: %d\n", getpid());
		shmId1 = crearMemoriaCompartida(1, &arregloMem);		
		
		for(processCounter = 0; processCounter < NO_PROCESOS; processCounter ++) {
			childPid = fork();
			switch(childPid){
				case -1:
					printf("Error al crear el proceso hijo\n");					
					break;
				case 0://proceso hijo
					for (contadorEscritura = 0; contadorEscritura < NO_IO; ) {	
						/* decrementar semaforo productor */
						operacionSemaforo(semId, 0, 0);				
						for(contadorMemoria = 2 ; contadorMemoria < 7; contadorMemoria++) {				
							operacionSemaforo(semId, contadorMemoria, 0);								
							variableMem* aux = &arregloMem[contadorMemoria - 2];								
							if (aux->bandera == 0) {
								printf("Productor con PID %d, y no. escritura %d\n", getpid(), contadorEscritura + 1);
								for(contadorLetra = 0; contadorLetra < 9; contadorLetra ++)									
									aux->datos[contadorLetra] = processCounter + 97;
								aux->datos[contadorLetra] = '\0';
								aux->bandera = 1;								
								operacionSemaforo(semId, contadorMemoria, 1);											
								contadorEscritura ++;				
								break;
							}else{
								operacionSemaforo(semId, contadorMemoria, 1);					
							}				
						}									
						operacionSemaforo(semId, 1, 1);			
					}		
					exit(0);
				default://proceso padre
					break;
			} 
		}	

		for(processCounter = 0; processCounter < NO_PROCESOS; processCounter ++) 
			wait(&childPid);
	
		shmdt(&shmId1);        
	}	
	else{
		printf("Error al crear los semaforos\n");
	}
	return(0);
}

int crearSemaforo(char* nombreSemaforo) {
	/*                         P  C  M1 M2 M3  */
	int valoresIniciales[8] = {5, 0, 1, 1, 1, 1, 1, 1};
	int semId = -1, contador, bandera, banderaDestruir;
	key_t key = ftok("/bin/ls", 70);
	semId = semget(key, 8, 0666|IPC_CREAT|IPC_EXCL) ;
	if (semId == -1) {
		switch(errno) {
    		case EEXIST:    			
    			semId = semget(key, 8, 0666);
    			printf("Se liga a los semaforos\n");
    			break;
			default:
	    		printf("Error desconocido\n");
	    		break;
    	}
	}else{
		printf("Se crearon los semaforos\n");
		for (contador = 0; contador < 8; contador++) {
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
	/*union semun arg;
	arg.val = valor;*/
	return (semctl(semId, noSem, SETVAL, valor));
}

int crearMemoriaCompartida(int noMem, variableMem** value) {
	// ftok to generate unique key
    key_t key = ftok("/bin/ls", (65 + noMem));
	variableMem** aux;
 
    // shmget returns an identifier in shmid
    int shmId = shmget(key, 5*sizeof(variableMem*), 0666|IPC_CREAT|IPC_EXCL);
    if(shmId == -1) {
    	switch(errno) {
    		case EEXIST:    			
    			shmId = shmget(key,5*sizeof(variableMem*),0666);
    			printf("Se liga a memoria compartida\n");
				/* aux = (variableMem*) shmat(shmId,(void*)0,0); */
    			break;
			default:
	    		printf("Error desconocido\n");
	    		break;
    	}
    }  
	else{
		printf("Se creo la memoria compartida\n");
		/* aux = (variableMem*) shmat(shmId,(void*)0,0);		 */
	}
	*value = (variableMem*) shmat(shmId,(void*)0,0);	
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
	int auxiliar;
	if(tipo == 1)
		auxiliar = 1;
	else
		auxiliar = -1;
	/* int auxiliar = (tipo == 1 ? 1 : -1 ); */
	struct sembuf op_disminuir = {noSem, auxiliar, 0};
	int bandera = semop(semId, &op_disminuir, 1);
	if(bandera != 0) {
		if(tipo == 0)
			printf("Error al decrementar el semaforo no. %d\n", noSem);
		else
			printf("Error al aumentar el semaforo no. %d\n", noSem);
	}
	/* printf("Nuevo Valor semaforo no: %d, %d\n", noSem, obtenerValorSemaforo(semId, noSem)); */
	return bandera;
}

void imprimirSemaforos(int semId) {
	int contadorMemoria;
	for(contadorMemoria = 0; contadorMemoria < 5; contadorMemoria ++) {
		printf("Valor semaforo no %d: %d\n", contadorMemoria, obtenerValorSemaforo(semId, contadorMemoria));
	}
}

void inicializarMemorias(variableMem*** arreglo, variableMem** mem1, variableMem** mem2, variableMem** mem3) {

}
