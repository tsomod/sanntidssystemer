#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <native/mutex.h>
#include <rtdk.h>
#include <sys/io.h>

RT_SEM semaphore, synca;
static RT_MUTEX mutex;

void print_pri(RT_TASK *task, char *s){
	struct rt_task_info temp;
	rt_task_inquire(task, &temp);
	rt_printf("b:%i c:%i ", temp.bprio, temp.cprio);
	rt_printf(s);
}

int rt_task_sleep_ms(unsigned long delay){
	return rt_task_sleep(1000*1000*delay);
}

void busy_wait_ms(unsigned long delay){
	unsigned long count = 0;
	while (count <= delay*10){
		rt_timer_spin(1000*100);
		count++;
	}
}

void low(){
	rt_sem_p(&synca, TM_INFINITE);
	rt_printf("LOW RUNNING\n");
	//rt_sem_p(&semaphore, TM_INFINITE);
	rt_mutex_acquire(&mutex, TM_INFINITE);
	rt_printf("LOW AQUIRED LOCK\n");
	busy_wait_ms(100);
	busy_wait_ms(100);
	busy_wait_ms(100);
	rt_printf("LOW FINISHED\n");
	//rt_sem_v(&semaphore);
	rt_mutex_release(&mutex);
	
}

void medium(){
	rt_sem_p(&synca, TM_INFINITE);
	rt_task_sleep_ms(100);
	rt_printf("MEDIUM RUNNING\n");
	busy_wait_ms(100);
	busy_wait_ms(100);
	busy_wait_ms(100);
	busy_wait_ms(100);
	busy_wait_ms(100);
	rt_printf("MEDIUM FINISHED\n");
}

void high(){
	rt_sem_p(&synca, TM_INFINITE);
	rt_task_sleep_ms(200);
	rt_printf("HIGH RUNNING\n");
	//rt_sem_p(&semaphore, TM_INFINITE);
	rt_mutex_acquire(&mutex, TM_INFINITE);
	rt_printf("HIGH AQUIRED LCOK\n");
	busy_wait_ms(100);
	busy_wait_ms(100);
	rt_printf("HIGH FINISHED\n");
	//rt_sem_v(&semaphore);
	rt_mutex_release(&mutex);
	
}
	

int main(){
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_print_auto_init(1);
	//rt_sem_create(&semaphore, "sem", 1, S_PRIO);
	rt_sem_create(&synca, "sync", 0, S_PRIO);
	rt_mutex_create(&mutex, "mutex");

	RT_TASK L, M, H;
	
	rt_task_shadow(NULL, "main", 4, T_CPU(1)|T_JOINABLE);

	rt_task_create(&L, "low", 0, 1, T_CPU(1)|T_JOINABLE);
	rt_task_create(&M, "medium", 0, 2, T_CPU(1)|T_JOINABLE);
	rt_task_create(&H, "high", 0, 3, T_CPU(1)|T_JOINABLE);

	rt_task_start(&L, &low, (void*) 0);
	rt_task_start(&M, &medium, (void*) 0);
	rt_task_start(&H, &high, (void*) 0);

	usleep(100000);
	rt_printf("RELEASING SYNC\n");
	
	rt_sem_broadcast(&synca);

	rt_task_join(&L);
	rt_task_join(&M);
	rt_task_join(&H);

	rt_sem_delete(&synca);
	rt_sem_delete(&semaphore);
	rt_mutex_delete(&mutex);

	return 0;
}





