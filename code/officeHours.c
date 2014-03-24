/***********************************************************
* Program: officeHours
* Introduction: 
* 		A C program that uses pThreads and semaphores to \
* synchronize a TA and students during office hours.
* Author: liang
* Date: 2014/3/21
***********************************************************/
/***********************************************************
[Detailed Instructions]
-  Only one person is speaking at a time.
-  The TA answers each question from a student.
-  No student asks another question before the TA is done answering the previous 
question.
-  A student can only ask one question when they get a chance to ask a question.
-  The TA and the students must be synchronized so that when a student asks their 
question the next person to speak is the TA with the answer.
-  A limited number of students are allowed in the TA's office.
-  Students will wait if the office is full.
-  A  student  does  not  leave  the  TA's  office  until  all their questions  have  been 
answered.
-  There is only one TA but there may be multiple students. 
-  You do not know the order of students in asking questions.
***********************************************************/
#include "officeHours.h"

/* Student - ask question */
void studentAsk(int student_id, int idx)
{
	tbc_num++;
	//printf("[tbc_num %d]",tbc_num);
	printf("I am student %d asking question %d\n", student_id, idx);
}

/* TA - answer question */
void officeAnswer()
{
	tbc_num--;
	//printf("[tbc_num %d]",tbc_num);
	printf("I am TA and this will be discussed in class.\n");
}

/* Thread func of student */
void *studentThread(void *param)
{
	int idx;
	student_info *si = (student_info *)param;
	printf("I am student %d and I'm waiting outside the TA's door\n", si->student_id);
	sem_wait(&room_sem);
	printf("I am student %d and I'm entering the office\n", si->student_id);
	//block - run - block ...
	for(idx=0; idx<si->question_num; idx++)
	{
		pthread_mutex_lock(&question_mutex);
		// while loop, cond check(satisfied) - wait - cond check(satisfied - wait / else ask)
		while(tbc_num>0) 
		{
			pthread_cond_wait(&question_cond, &question_mutex);
		}
		studentAsk(si->student_id, idx+1);
		pthread_mutex_unlock(&question_mutex);
	}
	sem_post(&room_sem);
	printf("I am student %d and I'm leaving the office\n", si->student_id);
    return NULL;
}

/* Thread func of office TA */
void *officeThread(void *param) 
{
    while(1) 
    {
        pthread_mutex_lock(&question_mutex);
		if(tbc_num>0)
		{
			total_num--;
			officeAnswer();  
		}
        pthread_mutex_unlock(&question_mutex);
		if(total_num<=0)
		{
			//break;
			//sleep(5);
			printf("[Zzzzz...Zzzzzz....]\n");
			printf("I am TA, no students in the office, I'm available now.\n");
			break;
		}
		else if(tbc_num<=0)
		{
			pthread_cond_signal(&question_cond);
		}
    }
    return NULL;
}

/* main function */
int main(int argc, char *argv[])
{
	int student_num, room_size, question_num;
	int i, res;
	pthread_t ta_pt;
	pthread_t *sp_ptr=NULL;
	student_info *si_ptr=NULL;
	tbc_num=0;
	
	if(argc!=4)
	{
		printf("Usage:\n");
		printf("\tofficeHours <number1> <number2> <number3>\n");
		printf("\t\tnumber1: number of students that will come during office hours\n");
		printf("\t\tnumber2: number of students allowed into the TA's office\n");
		printf("\t\tnumber3: number of questions each student may ask\n");
		handle_error("Invalid Input");
	}
	
	//1. initialization
	student_num = atoi(argv[1]);
	room_size = atoi(argv[2]);
	question_num = atoi(argv[3]);
	// check if input arguments valid or not
	if(student_num<=0||room_size<=0||question_num<=0)
	{
		printf("Invalid input numbers(should be greater than zero): \n");
		printf("number of students that will come:%d\n", student_num);
		printf("number of students allowed into the TA's office:%d\n", room_size);
		printf("number of questions each student may ask:%d\n", question_num);
		handle_error("Invalid Input");
		//return -1;
	}

	total_num = student_num*question_num;
	sp_ptr =(pthread_t *)malloc(student_num*sizeof(pthread_t));
	if(!sp_ptr)
	{
		handle_error("Allocate memory for students' thread index failed.");
	}

	si_ptr=(student_info *)malloc(student_num*sizeof(student_info));
	if(!si_ptr)
	{
		free(sp_ptr);
		handle_error("Allocate memory for student info failed.");
	}
	memset(sp_ptr, 0, student_num*sizeof(pthread_t));
	memset(si_ptr, 0, student_num*sizeof(student_info));
	
	// initialize semaphores and condition 
	res=sem_init(&room_sem,0,(unsigned int)room_size); // room_sem is initialized as room size 
	if(res==-1)
		handle_error("sem_init error");
	res=pthread_mutex_init(&question_mutex, NULL); // initialize question mutex
	if(res!=0)
		handle_error("pthread_mutex_init error");
	res=pthread_cond_init(&question_cond, NULL); // initialize question cond
	if(res!=0)
		handle_error("pthread_cond_init error");
   
	//2. create threads
	// threads of students
	for(i=0; i<student_num; i++)
	{	
		pthread_t pid;
		si_ptr[i].student_id = i+1;
		si_ptr[i].question_num = question_num;
		res=pthread_create(&pid, NULL, studentThread, &si_ptr[i]);
		if(res!=0)
			handle_error("pthread_create error");
		sp_ptr[i] = pid;
	}
	// threads of TA
	res=pthread_create(&ta_pt, NULL, officeThread, NULL);
	if(res!=0)
		handle_error("pthread_create error");
	
	//3. join threads
	for(i=0; i<student_num; i++)
	{	
		res=pthread_join(sp_ptr[i], NULL);
		if(res!=0)
			handle_error("pthread_join error");
	}
	res=pthread_join(ta_pt, NULL);
	if(res!=0)
		handle_error("pthread_join error");
	
	//4. cond, mutex, semaphore destory
	pthread_cond_destroy(&question_cond);
	pthread_mutex_destroy(&question_mutex);  
	sem_destroy(&room_sem);
	
	//5. free resource
	free(sp_ptr);
	free(si_ptr);
	
	exit(EXIT_SUCCESS);
}