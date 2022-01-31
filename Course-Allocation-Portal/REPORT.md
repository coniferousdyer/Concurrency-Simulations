# Q1: Alternate Course Allocation Portal

Sanchirat and Pratrey, students of the college, have been asked to use their skills to design an unconventional course registration system for the college, where a student can take trial classes of a course and can withdraw and opt for a different course if he/she does not like the course. Different labs in the college have been asked to provide students who can act as course TA mentors temporarily and take the trial tutorials.

## Logic

The program has been divided into two logical parts:

1. Courses first calling TAs and then calling interested students to join the tutorial.
2. Students waiting until the course calls them to join the tutorial.

The `main()` function takes input, creates threads for each Student and Course, and then joins them.

<b>IMPORTANT: </b>I have assumed that a tutorial cannot take place without any students. Therefore, I allotted a TA to a course only if there are students interested in the course. This has been elaborated upon in II. Courses.

### I. Students

In a student thread, the following happens:

1. A student first sleeps for some time, simulating the time it takes for them to choose their course preferences.

2. We then iterate through the course preferences of the student. The idea is that each iteration of the loop is dedicated to one course preference of the student.

3. First, we check if the course has not terminated yet. If it has, we skip the iteration, if it is not the last preference, or break out of the loop if it is.

4. Then we store the current priority in `prio`, and wait for the tutorial in the function `joinTutorial()`. The student thread is placed in a conditional wait, until the course tells the student to leave the tutorial, i.e. the course thread sends the student thread a conditional signal.

5. In the event that the course was removed while the student was waiting for or was in the tutorial, the course increases the student's current priority by 1 (discussed in II. Courses). This is why we stored the priority before waiting for the tutorial, so that we could compare it and see if the course was removed.

6. If the course was removed, we skip the iteration, if it is not the last preference, or break out of the loop if it is.

7. We then calculate the probability of the student joining the course permanently, and if the probability is greater than a random number, we do so by exiting the loop.

8. Else, we increase the student's current priority by 1, moving them on to their next preference. However, if the student has reached the last preference, we break out of the loop, because the student couldn't get their preferred course.

### II. Courses

In a course thread, the following happens:

1. The course thread runs in an infinite loop, because it must run until it runs out of TAs to teach it, which is an indefinite amount of time.

2. First, we choose the TA by iterating through the labs the course can choose the TA from, and store the TA's lab index (`l`) and the TA's index in the lab (`t`), setting `TA_available[t]` to false.

    - First, we search for unoccupied TAs.
    - If none were found, then we just find an eligible TA and wait for them to become free.
    - But if no eligible TAs were found, then the course thread exits. We set `Course_available[C->index]` to false and break out of the loop.

3. However, we do not allot the TA yet. First, we check if there any students currently interested in the course. If not, then we skip the iteration. But first, we do a double check to ensure that there still could be students who might be interested in the course in the future. If not, then the course is terminated. We set `Course_available[C->index]` to false and break out to the loop.

4. If there are students currently interested, then we allot the TA. We increase `TA_times` by 1 and lock their mutex to ensure no other course can choose them simultaneously.

5. Next, we find interested students. We iterate through the array of students, and if they are waiting for the course, and they are interested in this course, we call them to join, by setting their status variable `S->enrolled` to `STUDENT_IN_TUTORIAL`.

6. The thread sleeps for 2 seconds, to simulate the TA taking the tutorial.

7. We then release the students from the tutorial by setting `S->enrolled` to `STUDENT_LEFT_TUTORIAL` and sending them a conditional signal.

8. Then, we release the TA from the course, by unlocking their mutex. Now we check if the lab can offer any more TAs. If not, print the appropriate message. Move on to the next iteration.

### III. Labs

I have assumed a Lab to be a passive entity. The only role of the labs here is to supply TAs, and the course will summon TAs on its own from the labs. The lab does not need to do anything. It merely maintains an array of TAs, from which the lab can choose.

## File Structure

```
q1/
|___course.c
|___student.c
|___main.c
|___q1.h
|___REPORT.md
|___Makefile
```

## Compilation

1. To compile,
```bash
make
```
2. To run,
```bash
./a.out
```