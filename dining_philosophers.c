#include <stdio.h>
#include "gtthread.h"

gtthread_t p1,p2,p3,p4,p5;
struct gtthread_mutex_t c[5];
struct gtthread_mutex_t left,right;

/*
p1 - c1 and c2
p2 - c2 and c3
p3 - c3 and c4
p4 - c4 and c5
p5 - c5 and c1
*/

void eating (int n){
	int eating_time = rand()%100;
	long unsigned int l,r;
	if (n==1){
		left = c[1];
		l=1;
		right = c[2];
		r=2;
	}
	if (n==2){
                left = c[2];
		l=2;
                right = c[3];
		r=3;
        }
	if (n==3){
                left = c[3];
		l=3;
                right = c[4];
		r=4;
        }
	if (n==4){
                left = c[4];
		l=4;
                right = c[5];
		r=5;
        }
	if (n==5){
                left = c[1];
		l=5;
                right = c[5];
		r=1;
        }
	printf("Philosopher %d tries to get chopstick %lu\n",n,l);
	gtthread_mutex_lock(&left);
	printf("Philosopher %d tries to get chopstick %lu\n",n,r);
        gtthread_mutex_lock(&right);
	printf("Philosopher %d eating with chopsticks %lu and %lu\n",n,l,r);
	while (eating_time > 0){
		eating_time--;
	}
	printf("Philosopher %d releases chopstick %lu\n",n,r);
        gtthread_mutex_unlock(&right);
        printf("Philosopher %d releases chopstick %lu\n",n,l);
        gtthread_mutex_unlock(&left);
}

void thinking(int n){
	long thinking_time = rand()%100;
	printf("Philosopher %d is thinking\n",n);
	while (thinking_time >0)
		thinking_time--;
}

void* phil(void *t){
	while(1){
		thinking((int)t);
		printf("Philosopher %d was thinking, now he is hungry!\n",t);
		eating((int)t);
		printf("Philosopher %d was eating, now he is thinking!\n",t);
	}
}

void main(){
	gtthread_init(50000L);
	int i =1;
	for (i=1; i<6; i++){

		gtthread_mutex_init(&c[i]);
		printf("mutex initialized %d\n",i);
	}
	gtthread_create(&p1, phil, (void *)1);
	gtthread_create(&p2, phil, (void *)2);
	gtthread_create(&p3, phil, (void *)3);
	gtthread_create(&p4, phil, (void *)4);
	gtthread_create(&p5, phil, (void *)5);
	printf("Philosopher Threads created\n");

	//while(1);
	gtthread_join(p1, NULL);
	gtthread_join(p2, NULL);
	gtthread_join(p3, NULL);
	gtthread_join(p4, NULL);
	gtthread_join(p5, NULL);
}


