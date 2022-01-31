#include "q1.h"

// Function to choose a TA to take a tutorial
// LOGIC: First look for free TAs, then look for TAs who are not free but could be eligible
void chooseTA(Course *C, int *l, int *t)
{
    bool TA_found = false;

    // 1. Looking for free TAs (who are not taking a course)
    // Choosing a lab to select a TA from
    for (*l = 0; *l < C->num_labs; (*l)++)
    {
        // Finding an unoccupied TA from that lab
        for (*t = 0; *t < Labs[C->clabs[*l]].num_TAs; (*t)++)
        {
            // Locking the TA
            pthread_mutex_lock(&Labs[C->clabs[*l]].TA_mutexes[*t]);

            // If the TA is available
            if (Labs[C->clabs[*l]].TA_available[*t] && Labs[C->clabs[*l]].TA_times[*t] < Labs[C->clabs[*l]].limit)
            {
                TA_found = true;
                Labs[C->clabs[*l]].TA_available[*t] = false;
                break;
            }

            pthread_mutex_unlock(&Labs[C->clabs[*l]].TA_mutexes[*t]);
        }

        // If a TA was found
        if (TA_found)
            break;
    }

    // 2. If no free TA was found yet, look for eligible TAs who are currently taking a course
    if (!TA_found)
    {
        // Choosing a lab to select a TA from
        for (*l = 0; *l < C->num_labs; (*l)++)
        {
            // Finding an eligible TA from that lab
            for (*t = 0; *t < Labs[C->clabs[*l]].num_TAs; (*t)++)
            {
                // Locking the TA or waiting for them to become available
                pthread_mutex_lock(&Labs[C->clabs[*l]].TA_mutexes[*t]);

                // If the TA is available
                if (Labs[C->clabs[*l]].TA_times[*t] < Labs[C->clabs[*l]].limit)
                {
                    TA_found = true;
                    Labs[C->clabs[*l]].TA_available[*t] = false;
                    break;
                }

                pthread_mutex_unlock(&Labs[C->clabs[*l]].TA_mutexes[*t]);
            }

            // If a TA was found
            if (TA_found)
                break;
        }
    }
}

// Function to allot seats in tutorial to students
void allotStudents(Course *C, int *num_students_preferring, bool to_be_chosen[])
{
    for (int i = 0; i < num_students; i++)
    {
        // If the student is waiting for this course
        if (Students[i].current_priority >= 0 && Students[i].current_priority < 3 && Students[i].courses[Students[i].current_priority] == C->index && Students[i].enrolled == STUDENT_WAITING)
        {
            to_be_chosen[i] = true;
            (*num_students_preferring)++;
        }
        else
            to_be_chosen[i] = false;
    }

    // Selecting a random number for the course slot
    int d = rand() % C->course_max_slot + 1;
    int actual_number = (d > *num_students_preferring ? *num_students_preferring : d);
    pthread_mutex_lock(&print_lock);
    printf(MAGENTA "Course %s has been allocated %d seats\n" RESET, C->course_name, d);
    pthread_mutex_unlock(&print_lock);

    // Allocating the slots
    for (int i = 0, allotted = 0; i < num_students && allotted < actual_number; i++)
    {
        if (to_be_chosen[i])
        {
            Students[i].enrolled = STUDENT_IN_TUTORIAL;
            allotted++;
            pthread_mutex_lock(&print_lock);
            printf(GREEN "Student %d has been allocated a seat in course %s\n" RESET, i, C->course_name);
            pthread_mutex_unlock(&print_lock);
        }
    }

    pthread_mutex_lock(&print_lock);
    printf(YELLOW "Tutorial has started for Course %s with %d seats filled out of %d\n" RESET, C->course_name, actual_number, d);
    pthread_mutex_unlock(&print_lock);
}

// Function to release the students once the tutorial is over
void releaseStudents(Course *C, bool to_be_chosen[])
{
    for (int i = 0; i < num_students; i++)
        if (to_be_chosen[i] && Students[i].enrolled == STUDENT_IN_TUTORIAL && Students[i].courses[Students[i].current_priority] == C->index)
        {
            Students[i].enrolled = STUDENT_LEFT_TUTORIAL;
            pthread_cond_signal(&Students[i].cond_tut);
        }
}

// Function to perform actions for students are interested in a terminated course
void removeCourse(Course *C)
{
    for (int i = 0; i < num_students; i++)
    {
        // If the student is waiting for this course
        if (Students[i].current_priority >= 0 && Students[i].current_priority < 3 && Students[i].courses[Students[i].current_priority] == C->index && (Students[i].enrolled == STUDENT_WAITING || Students[i].enrolled == STUDENT_IN_TUTORIAL))
        {
            Students[i].current_priority++;
            Students[i].enrolled = STUDENT_LEFT_TUTORIAL;
            pthread_cond_signal(&Students[i].cond_tut);
        }
    }
}

// The thread for courses
void *courseThread(void *arg)
{
    Course *C = (Course *)arg;

    for (;;)
    {
        /*------CHOOSING TA------*/

        int l; // Lab index
        int t; // TA index
        chooseTA(C, &l, &t);

        // If no TA could be chosen, course is dropped, breaking out of infinite loop
        if (l == C->num_labs)
        {
            pthread_mutex_lock(&print_lock);
            printf(RED "Course %s doesn’t have any TA’s eligible and is removed from course offerings\n" RESET, C->course_name);
            pthread_mutex_unlock(&print_lock);
            break;
        }

        // Checking if students are currently interested in the course
        bool student_interested = false;
        for (int i = 0; i < num_students; i++)
        {
            // If the student is interested in this course
            if (Students[i].current_priority >= 0 && Students[i].current_priority < 3 && Students[i].courses[Students[i].current_priority] == C->index && Students[i].enrolled == STUDENT_WAITING)
            {
                student_interested = true;
                break;
            }
        }

        if (!student_interested)
        {
            Labs[C->clabs[l]].TA_available[t] = true;
            pthread_mutex_unlock(&Labs[C->clabs[l]].TA_mutexes[t]);

            // Checking if any students are interested at all in the course
            bool all_students_finished = true;
            for (int i = 0; i < num_students; i++)
                for (int j = Students[i].current_priority; j < 3; j++)
                    if (Students[i].courses[j] == C->index)
                    {
                        all_students_finished = false;
                        goto check;
                    }

        check:
            // If no student is interested in this course, course is dropped, breaking out of infinite loop
            if (all_students_finished)
                break;

            continue;
        }

        /*------ASSIGNING TA TO COURSE------*/

        // Updating TA status
        Labs[C->clabs[l]].TA_times[t]++;

        // Checking number of times TA was assigned
        pthread_mutex_lock(&print_lock);
        printf(YELLOW "TA %d from lab %s has been allocated to course %s for his %dth TA ship\n" RESET, t, Labs[C->clabs[l]].name, C->course_name, Labs[C->clabs[l]].TA_times[t]);
        printf(YELLOW "Course %s has been allocated TA %d from lab %s\n" RESET, C->course_name, t, Labs[C->clabs[l]].name);
        pthread_mutex_unlock(&print_lock);

        /*------ALLOTTING SLOTS------*/

        int num_students_preferring = 0;
        bool to_be_chosen[num_students];
        allotStudents(C, &num_students_preferring, to_be_chosen);

        /*------TAKING TUTORIAL------*/

        // Sleeping for a while to simulate the TA taking the tutorial
        sleep(2);

        /*------RELEASING STUDENTS------*/

        releaseStudents(C, to_be_chosen);

        /*------RELEASING TA FROM COURSE------*/

        // Release the TA
        pthread_mutex_lock(&print_lock);
        printf(YELLOW "TA %d from lab %s has completed the tutorial and left the course %s\n" RESET, t, Labs[C->clabs[l]].name, C->course_name);
        pthread_mutex_unlock(&print_lock);

        // Checking the lab to see if it can offer any more TAs
        bool any_TA_available = false;
        for (int i = 0; i < Labs[C->clabs[l]].num_TAs; i++)
            if (Labs[C->clabs[l]].TA_times[i] < Labs[C->clabs[l]].limit)
            {
                any_TA_available = true;
                break;
            }

        pthread_mutex_lock(&print_lock);

        if (!any_TA_available)
            printf(RED "Lab %s no longer has students available for TA ship\n" RESET, Labs[C->clabs[l]].name);
        
        pthread_mutex_unlock(&print_lock);        

        // Releasing the TA, making them free for other tutorials if eligible
        Labs[C->clabs[l]].TA_available[t] = true;
        pthread_mutex_unlock(&Labs[C->clabs[l]].TA_mutexes[t]);
    }

    // Removing the course from the course offerings
    Course_available[C->index] = false;
    removeCourse(C);

    pthread_exit(NULL);
}