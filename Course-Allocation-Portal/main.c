#include "q1.h"

Lab *Labs = NULL;
Course *Courses = NULL;
Student *Students = NULL;
bool *Course_available = NULL;
int num_students = 0, num_labs = 0, num_courses = 0;
pthread_mutex_t print_lock;

// Function to take input and initialise values
void takeInput()
{
    scanf("%d %d %d", &num_students, &num_labs, &num_courses);

    // Allocate memory for the arrays
    Labs = (Lab *)malloc(num_labs * sizeof(Lab));
    Courses = (Course *)malloc(num_courses * sizeof(Course));
    Course_available = (bool *)malloc(num_courses * sizeof(bool));
    Students = (Student *)malloc(num_students * sizeof(Student));

    // Taking input for courses
    for (int i = 0; i < num_courses; i++)
    {
        scanf("%s %lf %d %d", Courses[i].course_name, &Courses[i].interest, &Courses[i].course_max_slot, &Courses[i].num_labs);
        Courses[i].index = i;
        Course_available[i] = true;

        // Mallocing an array for labs
        Courses[i].clabs = (int *)malloc(Courses[i].num_labs * sizeof(int));

        for (int j = 0; j < Courses[i].num_labs; j++)
            scanf("%d", &Courses[i].clabs[j]);
    }

    // Taking input for students
    for (int i = 0; i < num_students; i++)
    {
        scanf("%lf %d %d %d %d", &Students[i].calibre, &Students[i].courses[0], &Students[i].courses[1], &Students[i].courses[2], &Students[i].time);
        Students[i].current_priority = STUDENT_CHOOSING;
        Students[i].enrolled = STUDENT_CHOOSING;
        Students[i].index = i;
        pthread_mutex_init(&Students[i].mutex, NULL);
        pthread_cond_init(&Students[i].cond_tut, NULL);
    }

    // Taking input for labs
    for (int i = 0; i < num_labs; i++)
    {
        int n_i;
        scanf("%s %d %d", Labs[i].name, &n_i, &Labs[i].limit);
        Labs[i].num_TAs = n_i;
        Labs[i].TA_times = (int *)calloc(n_i, sizeof(int));
        Labs[i].TA_available = (bool *)malloc(n_i * sizeof(bool));
        Labs[i].TA_mutexes = (pthread_mutex_t *)malloc(n_i * sizeof(pthread_mutex_t));
        for (int j = 0; j < n_i; j++)
        {
            Labs[i].TA_available[j] = true;
            pthread_mutex_init(&Labs[i].TA_mutexes[j], NULL);
        }
    }

    pthread_mutex_init(&print_lock, NULL);
}

// Function to create the threads for each entity instance
void createThreads()
{
    // Creating threads for courses
    for (int i = 0; i < num_courses; i++)
        pthread_create(&Courses[i].tid, NULL, courseThread, &Courses[i]);

    // Creating threads for students
    for (int i = 0; i < num_students; i++)
        pthread_create(&Students[i].tid, NULL, studentThread, &Students[i]);
}

// Function to perform cleanup before exiting
void cleanup()
{
    // For students
    for (int i = 0; i < num_students; i++)
    {
        pthread_join(Students[i].tid, NULL);
        pthread_mutex_destroy(&Students[i].mutex);
        pthread_cond_destroy(&Students[i].cond_tut);
    }

    // For courses
    for (int i = 0; i < num_courses; i++)
    {
        pthread_join(Courses[i].tid, NULL);
        free(Courses[i].clabs);
    }

    // For labs
    for (int i = 0; i < num_labs; i++)
    {
        free(Labs[i].TA_times);
        for (int j = 0; j < Labs[i].num_TAs; j++)
            pthread_mutex_destroy(&Labs[i].TA_mutexes[j]);
        free(Labs[i].TA_mutexes);
    }

    free(Labs);
    free(Courses);
    free(Students);

    pthread_mutex_destroy(&print_lock);
}

int main(void)
{
    srand(time(0));
    takeInput();
    createThreads();
    cleanup();
}