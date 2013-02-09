#include <stdio.h>
#include <process.h>
#define STKSIZE 16536

class Thread
{
	public:Thread(){}
	virtual ~Thread() {}
	static void *pthread_callback(void *ptrThis);
	virtual void run() = 0;
	void start();
};

void *Thread::pthread_callback(void *ptrThis)
{
	if (ptrThis == NULL) return NULL;
	Thread *ptr_this = (Thread *)(ptrThis);
	ptr_this-> run();
	return NULL;
}

void Thread::start()
{
	int result;
	if((result=_beginthread((void(*)(void *))Thread::pthread_callback,STKSIZE,this))<0)
	{
		printf("_beginthread error\n");
		exit(-1);
	}
}
