#ifndef _Q1_H_
#define _Q1_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

// Constants
#define MAX_NAME_LEN 100
#define PROB_OF_CHOOSING 1

// States (S->current_priority)
#define STUDENT_CHOOSING -1        // Student is waiting and choosing preferences

// States (S->enrolled)
#define STUDENT_WAITING 0          // Student is waiting for a course to be available
#define STUDENT_IN_TUTORIAL 1      // Student is in the tutorial
#define STUDENT_LEFT_TUTORIAL 2    // Student has left the tutorial

// Colours
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define RESET "\x1B[0m"

// The Lab entity
typedef struct Lab
{
    char name[MAX_NAME_LEN];        // Lab name
    int num_TAs;                    // Number of TAs the lab has
    int limit;                      // Maximum number of times a TA can take a tutorial
    int *TA_times;                  // Array of times each TA has taken a tutorial
    bool *TA_available;             // Array of booleans indicating whether a TA is available
    pthread_mutex_t *TA_mutexes;    // Array of mutex locks for each TA
} Lab;

// The Course entity
typedef struct Course
{
    pthread_t tid;                  // Thread ID corresponding to this Course struct 
    int index;                      // Index of Course struct in Courses array
    char course_name[MAX_NAME_LEN]; // Course name
    double interest;                // Interest level in course
    int num_labs;                   // Number of labs from which Course can choose a TA
    int *clabs;                     // Array of labs from which Course can choose a TA
    int course_max_slot;            // Maximum number of slots for a tutorial in this Course
} Course;

// The Student entity
typedef struct Student
{
    pthread_t tid;                  // Thread ID corresponding to this Student struct
    int index;                      // Index of Student struct in Students array
    int courses[3];                 // Array of courses student is interested in
    int time;                       // Time after which student chooses preferred courses
    double calibre;                 // Calibre level of students
    int current_priority;           // Index of current course chosen in Student::courses array
    int enrolled;                   // 0 before tutorial, 1 in tutorial and 2 after tutorial
    pthread_mutex_t mutex;          // Mutex lock for this Student struct
    pthread_cond_t cond_tut;        // Conditional variable for tutorial
} Student;

// Declaring the global arrays
extern Lab *Labs;
extern Course *Courses;
extern bool *Course_available;      // An array that indicates whether a course has terminated or not
extern Student *Students;
extern int num_students, num_labs, num_courses;
extern pthread_mutex_t print_lock;

// Student function declarations
void joinTutorial(Student *S);
void *studentThread(void *arg);

// Course function declarations
void chooseTA(Course *C, int *l, int *t);
void allotStudents(Course *C, int *num_students_preferring, bool to_be_chosen[]);
void releaseStudents(Course *C, bool to_be_chosen[]);
void removeCourse(Course *C);
void *courseThread(void *arg);

#endif