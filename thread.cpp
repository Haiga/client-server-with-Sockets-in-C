#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t semaforoProdutor, semaforoConsumidor;
pthread_mutex_t mutexVetor;
int slotsUtilizados = 0;
const int numMaxSlots = 5;

void *producer(void *arg) {
	int i;c
	for (i = 0; i < 10; i++) {
		while(slotsUtilizados == numMaxSlots){
			printf("Produtor -- o buffer está cheio! A produção está em espera!\n");
			sem_wait(&semaforoConsumidor);
			printf("Produtor -- a produção voltou a funcionar!\n");
		}
		pthread_mutex_lock(&mutexVetor);
		sleep(1); //A produção "demora" 1 s.
		slotsUtilizados++;
		pthread_mutex_unlock(&mutexVetor);
		printf("Produtor -- slots utilizados: %d.\n", slotsUtilizados);
		sem_post(&semaforoProdutor);
		printf("Produtor -- foi enviado o sinal de produção para o consumidor!\n");
	}
	pthread_exit(NULL);
}

void *consumer(void *arg) {
	int i;
	for (i = 0; i < 10; i++) {
		while(slotsUtilizados == 0){
			printf("Consumidor -- o buffer está vazio! O consumo está em espera!\n");
			sem_wait(&semaforoProdutor);
			printf("Consumidor -- o consumo voltou a funcionar!\n");
		}
		pthread_mutex_lock(&mutexVetor);
		sleep(1); //A produção "demora" 1 s.
		slotsUtilizados--;
		pthread_mutex_unlock(&mutexVetor);
		printf("Consumidor -- slots utilizados: %d.\n", slotsUtilizados);
		sem_post(&semaforoConsumidor);
		printf("Consumidor -- foi enviado o sinal de consumo para o produtor!\n");
	}
	pthread_exit(NULL);
}

int main(void) {
	pthread_t tid0, tid1;
	sem_init(&semaforoProdutor, 0, 0);
	sem_init(&semaforoConsumidor, 0, 0);

	pthread_create(&tid0, NULL, consumer, NULL);
	pthread_create(&tid1, NULL, producer, NULL);
	pthread_join(tid0, NULL);
	pthread_join(tid1, NULL);

	sem_destroy(&semaforoConsumidor);
	sem_destroy(&semaforoProdutor);

	return 0;
}
