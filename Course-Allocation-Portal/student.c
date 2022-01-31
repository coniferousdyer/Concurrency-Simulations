#include "q1.h"

// Function to wait for, attend and leave the tutorial
void joinTutorial(Student *S)
{
    pthread_mutex_lock(&S->mutex);

    // Before waiting for the tutorial, setting status
    S->enrolled = STUDENT_WAITING;

    // Waiting to get allotted a seat, and then till tutorial is over
    while (S->enrolled != STUDENT_LEFT_TUTORIAL)
        pthread_cond_wait(&S->cond_tut, &S->mutex);

    // Leaving the tutorial
    pthread_mutex_unlock(&S->mutex);
}

// The thread for students
void *studentThread(void *arg)
{
    Student *S = (Student *)arg;

    // Waiting for the initial time to simulate choosing of course preferences
    sleep(S->time);
    pthread_mutex_lock(&print_lock);
    printf("Student %d has filled in preferences for course registration\n", S->index);
    pthread_mutex_unlock(&print_lock);
    S->current_priority = 0;

    while (S->current_priority < 3)
    {
        /*------CORE MULTITHREADING LOGIC------*/

        // Checking if course is available
        if (!Course_available[S->courses[S->current_priority]])
        {
            S->current_priority++;

            // If this happened to be the student's last course
            if (S->current_priority >= 3)
            {
                pthread_mutex_lock(&print_lock);
                printf(GREEN "Student %d couldn’t get any of his preferred courses\n" RESET, S->index);
                pthread_mutex_unlock(&print_lock);
                break;
            }
            else
            {
                pthread_mutex_lock(&print_lock);
                printf(GREEN "Student %d has changed current preference from %s (priority %d) to %s (priority %d)\n" RESET, S->index, Courses[S->courses[S->current_priority - 1]].course_name, S->current_priority, Courses[S->courses[S->current_priority]].course_name, S->current_priority + 1);
                pthread_mutex_unlock(&print_lock);
                continue;
            }
        }

        int prio = S->current_priority;

        // Wait for and eventually attend the tutorial
        joinTutorial(S);

        // If course was removed and student was upgraded in the meantime
        if (prio < S->current_priority)
        {
            if (S->current_priority >= 3)
            {
                pthread_mutex_lock(&print_lock);
                printf(GREEN "Student %d couldn’t get any of his preferred courses\n" RESET, S->index);
                pthread_mutex_unlock(&print_lock);
                break;
            }
            else
            {
                pthread_mutex_lock(&print_lock);
                printf(GREEN "Student %d has changed current preference from %s (priority %d) to %s (priority %d)\n" RESET, S->index, Courses[S->courses[S->current_priority - 1]].course_name, S->current_priority, Courses[S->courses[S->current_priority]].course_name, S->current_priority + 1);
                pthread_mutex_unlock(&print_lock);
                continue;
            }
        }

        /*------PERMANENTLY CHOOSING/WITHDRAWING------*/

        pthread_mutex_lock(&S->mutex);

        // Calculating probability of student to choose course permanently
        double probability = S->calibre * Courses[S->courses[S->current_priority]].interest;

        // Choosing permanently
        if (probability >= PROB_OF_CHOOSING)
        {
            pthread_mutex_lock(&print_lock);
            printf(GREEN "Student %d has selected the course %s permanently\n" RESET, S->index, Courses[S->courses[S->current_priority]].course_name);
            pthread_mutex_unlock(&print_lock);
            S->current_priority = 3;
            pthread_mutex_unlock(&S->mutex);
            break;
        }
        // Withdrawing
        else
        {
            pthread_mutex_lock(&print_lock);
            printf(GREEN "Student %d has withdrawn from course %s\n" RESET, S->index, Courses[S->courses[S->current_priority]].course_name);
            pthread_mutex_unlock(&print_lock);
            S->current_priority++;
        }

        /*------EXITING SIMULATION OR CHANGING PRIORITY------*/

        // If the student could not choose a course permanently
        pthread_mutex_lock(&print_lock);

        if (S->current_priority >= 3)
            printf(GREEN "Student %d couldn’t get any of his preferred courses\n" RESET, S->index);
        else
            printf(GREEN "Student %d has changed current preference from %s (priority %d) to %s (priority %d)\n" RESET, S->index, Courses[S->courses[S->current_priority - 1]].course_name, S->current_priority, Courses[S->courses[S->current_priority]].course_name, S->current_priority + 1);

        pthread_mutex_unlock(&print_lock);
        pthread_mutex_unlock(&S->mutex);
    }

    pthread_exit(NULL);
}