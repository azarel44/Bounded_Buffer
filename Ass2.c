#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "thpool.h"
#define num_material_buffer_size 10
#define product_buffer_size 30

pthread_cond_t notfound1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t notfound2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t notfound3 = PTHREAD_COND_INITIALIZER;

pthread_cond_t maxmat1generated = PTHREAD_COND_INITIALIZER;
pthread_cond_t maxmat2generated = PTHREAD_COND_INITIALIZER;
pthread_cond_t maxmat3generated = PTHREAD_COND_INITIALIZER;

pthread_cond_t occupiedtool = PTHREAD_COND_INITIALIZER;

pthread_cond_t prodA = PTHREAD_COND_INITIALIZER;
pthread_cond_t prodB = PTHREAD_COND_INITIALIZER;
pthread_cond_t prodC = PTHREAD_COND_INITIALIZER;

/*

gen1     material buffer    operator1    product buffer
gen2  ====================  operator2  ======================
gen3                        operator3    uses material + tool

mutex: the tools

*/


typedef struct 
{
	int total; //total number of elements that is in the buffer
	int buffer[num_material_buffer_size]; // circular array
	int in; //available spot to put in the material
	int mat1_generated;
	int mat2_generated;
	int mat3_generated;
	pthread_mutex_t mutexmaterial;
	sem_t emptymat; //Counting the number of empty spot 
	sem_t fullmat; //Counting the number of taken spot
} mat_buffer;

typedef struct 
{
	int total; //total number of elements that is in the buffer
	int in; //available spot to put in the material
	int num_product_A;
	int num_product_B;
	int num_product_C;
	int buffer[product_buffer_size]; // circular char array
	pthread_mutex_t mutexproduct;
	sem_t emptymat; //Counting the number of empty spot 
	sem_t fullmat; //Counting the number of taken spot

} product_buffer;

typedef struct 
{
	int occupied;
	pthread_mutex_t toolmutex;
} tool_buffer;

void create(mat_buffer* buff)
{
	buff->total = 0;
	buff->in = 0;
	buff->mat1_generated = 0;
	buff->mat2_generated = 0;
	buff->mat3_generated = 0;
	pthread_mutex_init(&(buff->mutexmaterial),NULL);
	sem_init(&(buff->fullmat),1,0);
	sem_init(&(buff->emptymat),1,num_material_buffer_size);
}

void create_production(product_buffer* buff)
{
	buff->total = 0;
	buff->in = 0;
	buff->num_product_A = 0;
	buff->num_product_B = 0;
	buff->num_product_C = 0;
	pthread_mutex_init(&buff->mutexproduct,NULL);
}

void create_tool(tool_buffer * buff)
{
	buff->occupied = 0;
	pthread_mutex_init(&(buff->toolmutex),NULL);
}

/*Global variable*/
mat_buffer queue; //The material buffer
product_buffer product_queue; //The product buffer
tool_buffer tool_queue; 

int find(int value)
{
	int i = 0;
	
	while (queue.buffer[i] != value)
	{
		i++;
		if ((i%num_material_buffer_size == 0)&&(i ==10))
		{
			if (value == 1)
			{
				pthread_cond_wait(&notfound1, &queue.mutexmaterial);
			}
			else if (value == 2)
			{
				pthread_cond_wait(&notfound2, &queue.mutexmaterial);
			}
			else if (value == 3)
			{
				pthread_cond_wait(&notfound3, &queue.mutexmaterial);
			}
			i%=num_material_buffer_size;
		}
	}
	return i;
	
}

void mat_status()
{
	printf("Buffer Status: [");
	for (int i = 0; i < num_material_buffer_size; ++i)
	{
		printf("%d ", queue.buffer[i]); 
	}
	printf("]\n");
}

void product_status()
{
	printf("Product Status: [");
	for (int i = 0; i < product_buffer_size; ++i)
	{
		printf("%d ", product_queue.buffer[i]); 
	}
	printf("]\n");
}


void remove_ele(int value, int opcode)
{

	sem_wait(&queue.fullmat);
	pthread_mutex_lock(&queue.mutexmaterial);
	//int i = 0;
	int want = find(value);
	//printf("Operator %d is removing material number %d\n", opcode,queue.buffer[want]);
	int val = queue.buffer[want];
	queue.buffer[want] = 0;
    queue.total--;
    if (val == 1)
    {
    	pthread_cond_signal(&maxmat1generated);
    	queue.mat1_generated--;
    }
    else if (val == 2)
    {
    	pthread_cond_signal(&maxmat2generated);
    	queue.mat2_generated--;
    }
    else if (val == 3)
    {
    	pthread_cond_signal(&maxmat3generated);
    	queue.mat3_generated--;
    }
    pthread_mutex_unlock(&queue.mutexmaterial);
	sem_post(&queue.emptymat);
}

void put_product(int product)
{
	pthread_mutex_lock(&product_queue.mutexproduct);
	//printf("Generator %d generating material number %d\n", mat ,mat);
	//Inserting materials into buffer
	//printf("The product %d\n", product);
	int check;
	if (product_queue.in == 0)
	{
		check = product_buffer_size-1;
	}
	else
	{
		check = product_queue.in-1;
	}
	if (product_queue.buffer[check] == product)
	{
		pthread_mutex_unlock(&product_queue.mutexproduct);
		return;
	}
	if (product == 1)
	{
		if ((product_queue.num_product_A - product_queue.num_product_B >= 9) || (product_queue.num_product_A - product_queue.num_product_C >= 9))
		{
			pthread_mutex_unlock(&product_queue.mutexproduct);
			return;
		}
	}
	else if (product == 2)
	{
		if ((product_queue.num_product_B - product_queue.num_product_A >= 9) || (product_queue.num_product_B - product_queue.num_product_C >= 9))
		{
			pthread_mutex_unlock(&product_queue.mutexproduct);
			return;
		}
	}
	else if (product == 3)
	{
		if ((product_queue.num_product_C - product_queue.num_product_B >= 9) || (product_queue.num_product_C - product_queue.num_product_A >= 9))
		{
			pthread_mutex_unlock(&product_queue.mutexproduct);
			return;
		}
	}
	product_queue.buffer[product_queue.in] = product;
	product_queue.in++;
	product_queue.in %= product_buffer_size;
	product_queue.total++;
	if (product == 1)
	{
		product_queue.num_product_A++;
	}
	else if (product == 2)
	{
		product_queue.num_product_B++;
	}
	else if (product == 3)
	{
		product_queue.num_product_C++;
	}
	system("/bin/stty cooked");
	mat_status();
	printf("Generated %d of material 1 so far\n", queue.mat1_generated);
	printf("Generated %d of material 2 so far\n", queue.mat2_generated);
	printf("Generated %d of material 3 so far\n", queue.mat3_generated);
	product_status();
	printf("Generated %d of product A so far\n", product_queue.num_product_A);
	printf("Generated %d of product B so far\n", product_queue.num_product_B);
	printf("Generated %d of product C so far\n", product_queue.num_product_C);
	system("/bin/stty raw");
	pthread_mutex_unlock(&product_queue.mutexproduct);
}

void *generator (void *arg)
{
	int material = (intptr_t) arg; //Converting parameters into its original type
	int mat = material;
	while(1)
	{	
		sem_wait(&queue.emptymat);
		pthread_mutex_lock(&queue.mutexmaterial);
		if ((mat == 1) && (queue.mat1_generated == 4))
		{
			pthread_cond_wait(&maxmat1generated,&queue.mutexmaterial);
		}
		else if ((mat == 2) && (queue.mat2_generated == 3))
		{
			pthread_cond_wait(&maxmat2generated,&queue.mutexmaterial);
		}
		else if ((mat == 3) && (queue.mat3_generated == 3))
		{
			pthread_cond_wait(&maxmat3generated,&queue.mutexmaterial);
		}
		//printf("Generator %d generating material number %d\n", mat ,mat);
		//Inserting materials into buffer
		queue.in = find(0);
		queue.buffer[queue.in] = mat;
	    queue.total++;
	    if (mat == 1)
		{
			queue.mat1_generated++;
			pthread_cond_signal(&notfound1);
		}
		else if (mat == 2)
		{
			queue.mat2_generated++;
			pthread_cond_signal(&notfound2); 
		}
		else if (mat == 3)
		{
			queue.mat3_generated++;
			pthread_cond_signal(&notfound3); 
		}
	    pthread_mutex_unlock(&queue.mutexmaterial);
		sem_post(&queue.fullmat);
		
		// mat_status();
	}
	pthread_exit(NULL);
}


void *operator (void *arg)
{
	while(1)
	{
		int opcode = (intptr_t) arg; //Converting parameters into its original type
		int product;
		int selection = (rand()%3);
		int buildtime = (rand()%100)/100;
		if (selection == 0)
		{
			// char productname = 'A';
			// printf("Operator %d is currently producing product %c\n", opcode,productname);
			remove_ele(1,opcode);
			remove_ele(2,opcode);
			pthread_mutex_lock(&tool_queue.toolmutex);
			sleep(buildtime);
			pthread_mutex_unlock(&tool_queue.toolmutex);
			product = 1;
		}
		else if (selection == 1)
		{
		    // char productname = 'B';
			// printf("Operator %d is currently producing product %c\n", opcode,productname);
			remove_ele(2,opcode);
			remove_ele(3,opcode);
			pthread_mutex_lock(&tool_queue.toolmutex);
			sleep(buildtime);
			pthread_mutex_unlock(&tool_queue.toolmutex);
			product = 2;
		}
		else if(selection == 2)
		{
		    // char productname = 'C';
			// printf("Operator %d is currently producing product %c\n", opcode,productname);
			remove_ele(3,opcode);
			remove_ele(1,opcode);
			pthread_mutex_lock(&tool_queue.toolmutex);
			sleep(buildtime);
			pthread_mutex_unlock(&tool_queue.toolmutex);
			product = 3;
		}
		put_product(product);
	}
	pthread_exit(NULL);
}


int main()
{
	//Initialize the material buffer
	create(&queue);
	create_production(&product_queue);
	create_tool(&tool_queue);


	char c;
	char r;
	int input;
	int mat[3] = {1,2,3};
	printf("--Please note that the program will start once the number of operators is entered\n");
	printf("--Failure to do as instructed will result in program errors\n");
	printf("--Once the program runs,\n--Press p to pause.\n--Press r to resume.\n--Press e to terminate\n");
	printf("--Enter the valid number of operators or e to terminate:\n--");
	r = getchar();
	if (r == 'e')
	{
		printf("Program terminated\n");
		return 0;
	}
	input = atoi(&r);
	input += 3;
	threadpool thpool = thpool_init(input);
	
	for (int i = 0; i < 3; ++i)
	{
		thpool_add_work(thpool,(void*)generator,(void*)(intptr_t)mat[i]);
	}
	
	int op[input];
	for (int j = 0; j < input; ++j)
	{
		op[j] = j;
	}
	for (int k = 0; k < input; ++k)
	{
		thpool_add_work(thpool,(void*)operator,(void*)(intptr_t)op[k]);
	}
	int presscountp = 0;
	
	
	while(1)
	{
		system("/bin/stty raw");
		c = getchar();
		system("/bin/stty cooked");
		if (c == 'p')
		{
			if (presscountp == 0)
			{
				thpool_pause(thpool);
			}
			presscountp++;
		}
		if (c == 'r')
		{
			presscountp = 0;
			thpool_resume(thpool);
		}	
		if (c == 'e')
		{
			thpool_pause(thpool);
			break;
		}	
	}
	printf("Program terminated\n");
	return 0;
}