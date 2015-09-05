#include "gtthread.h"

int is_init = 0;
int tcount = 0;
int tnum = 0;
struct sigaction alarm_a;
struct itimerval quantum;

void schedule();

/*On signal, call schedule*/
void catch_signal (int sig) {
	if (sig == SIGVTALRM)
		schedule();
}

void schedule(){
	struct gtthread *temp = curr;
	/* If no thread in the list, exit. */
	if (tcount == 0)
		exit(1);
	int c=0;
	while(tcount!=0){
		c++;
		if(c>4*tcount){
			/*printf("No runnable thread found");*/
			exit(0);
		}
		/* Find the nest thread that needs to be scheduled */
	  	if (curr->next == NULL)
          	       curr = head;
	        else curr=curr->next;

		if (tcount == 1 && curr->exited==1){
			printf("check main exit\n");
			fflush(stdout);
			exit(1);
		}
		if (curr->exited!=1 && curr->completed != 1 && curr->canceled !=1)
		{
			break;
		}
	}
	swapcontext(&(temp->thd_context),&(curr->thd_context));
}

void run(void *(*start_routine)(void *),void *arg){
	curr->running=1;
    curr->ret_val=start_routine(arg);
	curr->completed=1;
	while(1);
}

/* Must be called before any of the below functions. Failure to do so may
 * result in undefined behavior. 'period' is the scheduling quantum (interval)
 * in microseconds (i.e., 1/1000000 sec.). */
void gtthread_init(long period){

	memset(&alarm_a, 0, sizeof(alarm_a));
	alarm_a.sa_handler = catch_signal;
	alarm_a.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&alarm_a.sa_mask);
	sigaction(SIGVTALRM, &alarm_a, NULL);
	if (sigaction(SIGVTALRM, &alarm_a, NULL) == -1) {
		printf("Error - Initialization of alarm handler failed\n");
		exit(-1);
	}
	quantum.it_value.tv_sec = 0;
	quantum.it_value.tv_usec =  (long) period;
	quantum.it_interval = quantum.it_value;
	setitimer(ITIMER_VIRTUAL, &quantum, NULL);
	if (setitimer(ITIMER_VIRTUAL, &quantum, NULL) != 0)
	   	printf("Timer could not be initialized \n");

	maint = (struct gtthread*) malloc (sizeof(struct gtthread));
	memset(maint, 0, sizeof(struct gtthread));
	maint->tid=tnum;
	maint->running=0;
	maint->canceled=0;
	maint->exited=0;
	maint->completed=0;
	maint->blocked=0;
	maint->next=NULL;
	getcontext(&maint->thd_context);

	/* Add main thread to the list. */
	head = maint;
	tail = maint;
	curr = maint;
	tnum++;
	tcount++;
	is_init = 1;
}

int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg){
	if(is_init == 0){
		printf("Call gtthread_init first. Initialization failed.");
		exit(1);
	}
	*thread = tnum;
	struct gtthread	*newt;
	newt = (struct gtthread*) malloc (sizeof(struct gtthread));
	memset(newt, 0, sizeof(struct gtthread));
	newt->tid = *thread;
	newt->running = 0;
	newt->completed = 0;
	newt->blocked = 0;
	newt->canceled =0;
	newt->exited=0;
	newt->next=NULL;
	getcontext(&newt->thd_context);
	newt->thd_context.uc_link=NULL;
	newt->thd_context.uc_stack.ss_sp=malloc(STACKSIZE);
	newt->thd_context.uc_stack.ss_size = STACKSIZE;
	if (newt->thd_context.uc_stack.ss_sp == 0) {
		printf("returning\n");
		return -1;
	}

	makecontext(&newt->thd_context, run, 2, start_routine, arg);
	tail->next = newt;
	tail = newt;
	tcount++;
	tnum++;
	return 0;
}

int  gtthread_join(gtthread_t thread, void **status){
	struct gtthread *dummy = head;
	while(dummy!=NULL){
		if (dummy->tid == thread){
			break;
		}
		dummy=dummy->next;
	}
	if (dummy != NULL) {
		while((dummy->completed!=1 &&  dummy->canceled!=1) &&  dummy->exited!=1){
		}
	if(status!=NULL){
		if(dummy->ret_val != NULL && dummy->canceled==1){
			*status = (void *)dummy->ret_val;
			return 0;
			}

		else if(dummy->canceled){
			*status = ((void *) -1);
			return 0;
			}
		else {
			*status = (void *)dummy->ret_val;
			return 0;
		}
		}
		return -1;
	}
	else {
		printf("No matching thread found!\n");
		return -1;
	}
}

void gtthread_exit(void *retval){
	curr->ret_val = retval;
	curr->exited=1;
	schedule();
}

int gtthread_yield(void){
	curr->blocked=1;
	schedule();
	return 0;
}

int  gtthread_equal(gtthread_t t1, gtthread_t t2){
	return t1==t2;
}

int  gtthread_cancel(gtthread_t thread){
	struct gtthread *dummy2 = head;
        for(;dummy2!=NULL;dummy2=dummy2->next){
                if (dummy2->tid == thread)
                        break;
        }
	dummy2->canceled=1;
	dummy2->exited=1;
	schedule();
	tcount--;
	return 0;
}

gtthread_t gtthread_self(void){
	return curr->tid;
}

