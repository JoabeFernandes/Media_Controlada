/* Inlcudes Padrão C */
#include <stdio.h>
#include <locale.h>
#include <string.h>



/* Include para prints*/
#include "defines.h"

/* Includes do Kernel */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define STRSIZE 16

FILE* fp;
char* filename = ".\\input.txt";
char str[STRSIZE];
float vetor[10];

void sensor_ISR(void* pvParameters);
void Calculadora(void* pvParameters);
void Alarme(void* pvParameters);

portTickType xLastWakeTime;           //Variável de tempo/Tick
xSemaphoreHandle xBinarySemaphore;   // Semáforo Binario responsavel por liberar função calculadora após 'n' apresentaçoes de amostras 
xTaskHandle xTask_Alarme;           // "Apelido" para chamar uma Task para modificar sua propriedade


void main_ex26(void)
{
	
	vSemaphoreCreateBinary(xBinarySemaphore,0); //Cria o Semáforo Binário - Inicializado indisponível

	fp = fopen(filename, "r");

	if (fp == NULL) {
		printf("Arquivo %s nao localizado\n", filename);
		return 1;
	}

	xTaskCreate(Alarme, "Alarme", 1000, NULL, 1, &xTask_Alarme);  
	printf("Criada Alarme\r\n\n");
	xTaskCreate(sensor_ISR, "sensor_ISR",   1000, NULL, 3 , NULL);
	printf("Criada sensor_ISR\r\n\n");
	xTaskCreate(Calculadora, "Calculadora", 1000, NULL, 5, NULL);
	printf("Criada Calculadora\r\n\n");
  


	printf("Escalonador Iniciado!\r\n\n");
	vTaskStartScheduler();

	
}

void sensor_ISR(void* pvParameters)
{
	while (1)
	{
	    float x;
		unsigned int ui;
		unsigned long ul;

		for (int i = 0; i < 10; i++)
		{


			if (fgets(str, STRSIZE, fp) == NULL)
			{
				printf("Final do Arquivo!\r\n\n");
				fclose(fp);
				vTaskDelete(NULL);
			}
			x = atof(str);
			printf("Amostra Coletada de valor = %.2f\n", x);

		
			vetor[i] = x; // Grava cada amostra em um vetor de tamanho 10

			for (ui = 0; ui < 10; ui++)
			{
				for (ul = 0; ul < 10000000; ul++)
				{

					// Utilizado para "pausar" o código, porém sem deixar a Task ir para blocked

				}
			}


		}
		
		xSemaphoreGive(xBinarySemaphore); // Libera semaforo para chamar a função calculadora de prioridade maior

	}
}

void Calculadora(void* pvParameters)
 {
	float soma = 0, media = 0, aux;
	int j;
	unsigned portBASE_TYPE uxPriority;
	
	while (1)
	
	 {		
		xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);// solicita semaforo para rodar

		// Se semáforo estiver indisponivel o codigo para aqui e vai para blocked
		// Se semáforo estiver disponivel o codigo segue normalmente

		for (j = 0; j < 10; j++) // leitura do vetor e somatorio para calculo 
		{
			
			aux = vetor[j];
			soma = aux + soma;

		}
		 media = soma / j; // Calculo da média

	   //float media = atof(accumulate(vetor));  //Essa função não deu certo. intenção: otimizar o calculo da media

		if (media > 100)
		{
			printf_colored("****************************\n", COLOR_CYAN);
			printf("A media e = %.f\n", media);

			uxPriority = uxTaskPriorityGet(xTask_Alarme);// Ler prioridade atual
			vTaskPrioritySet(xTask_Alarme, (uxPriority+5));// Aumenta prioridade da task Alarme, fazendo assim que o Alarme vá para Runnig e a Calculadora vá para Ready

				
		}
		if (media <= 100)
		{
			printf_colored("****************************\n", COLOR_CYAN);
			printf("A media e = %.f\n\n", media);
		}
		
	
		
	}
} 


void Alarme(void* pvParameters)
{
	while (1) {
		unsigned int ui;
		unsigned long ul;
		unsigned portBASE_TYPE uxPriority;
	
		for (int i = 0; i < 3; i++)
		{

			printf("==================================\n");
			printf_colored("'  'Alerta!! Media maior que 100\n", COLOR_RED);
			printf("==================================\n");
			printf("\n\n");

			for (ui = 0; ui < 3; ui++)
			{
				for (ul = 0; ul < 100000000; ul++)
				{
					// Utilizado para "pausar" o código, porém sem deixar a Task ir para blocked
				}
			}
		}


		uxPriority = uxTaskPriorityGet(xTask_Alarme);     // Ler prioridade da task 
		vTaskPrioritySet(xTask_Alarme, (uxPriority - 5));// Diminuindo prioridade task Alarme, após ela ter feito seu objetivo


	}
}
