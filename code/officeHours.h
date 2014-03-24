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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)
   
typedef struct _student_info
{
	int student_id;
	int question_num;
}student_info;

sem_t room_sem; //start with room size, student (enter - wait /leave - post)
int tbc_num; // condition variable - num of question to be answered(0 empty/1 exists question)
int total_num; //total number of questions for the TA
pthread_cond_t question_cond; //cond var - question 
pthread_mutex_t question_mutex; // cond mutex - question

void studentAsk(int student_id, int idx);
void officeAnswer();
void *studentThread(void *param);
void *officeThread(void *param);

