/// Odile SCHAEFFER
/// PSEUDOCODE:
/// Input: Number of Tasks, Execution Time, Deadline, and Period
/// Process:
/// 1. Check if the system is schedulable (using utilization check or other methods)
/// 2. If not schedulable: Output: Display an error message and stop the program
/// 3. If schedulable:
///    a. Generate all jobs for each task based on its period and deadlines
///    b. Sort jobs by Earliest Deadline First (EDF)
///    c. Execute the jobs in EDF order, ensuring no deadlines are missed
///    d. Calculate total waiting time and execution time
/// 4. After normal schedule (no missed deadlines), calculate the schedule where τ5 may miss its deadline
/// Output: Display the execution order, total waiting time, and execution time.

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NAME 50
#define MAX_TASKS 10
#define MAX_JOBS 50

// Constants Initialisation: --------
typedef struct {
    char TaskName[NAME];
    int ExeTime;
    int Deadline; // Period = Deadline
} Task;

typedef struct {
    char JobName[NAME];
    int ReleaseTime;
    int JobExeTime;
    int AbsoluteDeadline;
} Job;


Task taskList[MAX_TASKS];
int taskCount = 0;
Job jobList[MAX_JOBS];
int jobCount = 0;

// Fonctions Initialisation : --------
void Initialisation(int NumTask);
int Hyperperiod();
int Executability();
void GenerateJobs(int Hyperperiod);
void SortByDeadline();
void ExecuteJobs(int j);


int main()
{
    int NumTask;

    printf("How many tasks do you have to execute? ");
    scanf("%d", &NumTask);

    Initialisation(NumTask);
    Hyperperiod();

    if (Executability())
    {
        printf("\n\n--- Executing Tasks in EDF Order ---\n");
        GenerateJobs(Hyperperiod());
        SortByDeadline();
        printf("\n\n=== FIRST EXECUTION : Normal schedule ===\n");
        ExecuteJobs(0); // 0 -> no task allowed to miss deadline

        printf("\n\n=== SECOND EXECUTION : Minimizing waiting time (task 5 allowed to miss) ===\n");
        ExecuteJobs(1); // 1 -> Task 5 is allowed to miss deadline

    }
    return 0;
}

// Initialisation : --------
void AddTask(const char *TaskName, int ExeTime, int Deadline)
{
    if (taskCount < MAX_TASKS)
    {
        strncpy(taskList[taskCount].TaskName, TaskName, NAME);
        taskList[taskCount].ExeTime = ExeTime;
        taskList[taskCount].Deadline = Deadline;
        taskCount++;
    } else
    {
        printf("Task list is full!\n");
    }
}

void Initialisation (int NumTask)
{
    for (int i = 0; i < NumTask; i++)
    {
        char TaskName[NAME];
        int exe, deadline;

        sprintf(TaskName, "Task%d", i + 1);

        printf("Enter execution time for %s: ", TaskName);
        scanf("%d", &exe);

        printf("Enter deadline for %s: ", TaskName);
        scanf("%d", &deadline);

        AddTask(TaskName, exe, deadline);
    }

    printf("\nTasks added:\n");
    for (int i = 0; i < taskCount; i++)
    {
        printf("Task: %s, Execution: %d, Deadline: %d\n",
               taskList[i].TaskName, taskList[i].ExeTime, taskList[i].Deadline);
    }
}

// Least Common Multiple
int lcm(int a, int b)
{
    int x=a, y=b;
    while (y!=0)
    {
        int t = y;
        y = x%y;
        x = t;
    }
    return (a*b)/x;
}

int Hyperperiod()
{
    int H = taskList[0].Deadline;
    for (int i = 1; i < taskCount; i++)
    {
        H = lcm(H, taskList[i].Deadline);
    }
    printf("The Hyperperiod is: %d\n", H);
    return H;
}


int Executability()
{
    float U = 0.0; // Utilization
    for (int i = 0; i < taskCount; i++ )
    {
        U += (float)taskList[i].ExeTime / taskList[i].Deadline;
    }
    printf("\nTotal utilization U = %f \n", U);
    if (U <= 1.0)
    {
        printf("The System is Schedulable.\n");
        return 1;
    }
    else
    {
        printf("--- Error : The System is not Schedulable ---\n");
        return 0;
    }
}

// Generate Every Jobs
void GenerateJobs(int Hyperperiod)
{
    for (int i = 0; i < taskCount; i++)
    {
        int release = 0;
        while (release < Hyperperiod)
        {
            if (jobCount >= MAX_JOBS)
            {
                printf("Too many jobs!\n");
                return;
            }
            sprintf(jobList[jobCount].JobName, "T(%d,%d)", i+1, release / taskList[i].Deadline + 1);
            jobList[jobCount].ReleaseTime = release;
            jobList[jobCount].JobExeTime = taskList[i].ExeTime;
            jobList[jobCount].AbsoluteDeadline = release + taskList[i].Deadline;
            jobCount++;
            release += taskList[i].Deadline;
        }
    }
}

void SortByDeadline() // Earliest Deadline First
{
    for (int i = 0; i < jobCount - 1; i++)
    {
        for (int j = i + 1; j < jobCount; j++)
        {
            if (jobList[i].AbsoluteDeadline > jobList[j].AbsoluteDeadline)
            {
                Job t = jobList[i];
                jobList[i] = jobList[j];
                jobList[j] = t;
            }
        }
    }
}

void ExecuteJobs(int allowMissedDeadlineForT5)
{
    int currentTime = 0;
    int finishedJobs = 0;
    int executed[MAX_JOBS] = {0}; // 0 = not yet executed
    int totalBusyTime = 0;
    int totalTime = 0;
    int totalWaitingTime = 0;
    char executionOrder[MAX_JOBS * NAME] = "";

    printf("\nExecution Timeline:\n");
    printf(" %-10s %-10s %-10s %-10s %-10s\n", "Job", "Start", "End", "Waiting", "Response");

    while (finishedJobs < jobCount)
    {
        int earliestJob = -1;
        int minDeadline = 1e9;

        // Look for the job with the closest deadline
        for (int i = 0; i < jobCount; i++)
        {
            if (!executed[i] && jobList[i].ReleaseTime <= currentTime)
            {
                int isT5 = (jobList[i].JobName[2] == '5');

                // If we allow Task 5 to miss its deadline, we ignore it in the first pass
                if (allowMissedDeadlineForT5 && isT5)
                {
                    continue;
                }

                if (jobList[i].AbsoluteDeadline < minDeadline)
                {
                    minDeadline = jobList[i].AbsoluteDeadline;
                    earliestJob = i;
                }
            }
        }

        // If no "normal" job found, allow Task 5
        if (earliestJob == -1 && allowMissedDeadlineForT5)
        {
            for (int i = 0; i < jobCount; i++)
            {
                if (!executed[i] && jobList[i].ReleaseTime <= currentTime)
                {
                    earliestJob = i;
                    break;
                }
            }
        }

        // If nothing : IDLE
        if (earliestJob == -1)
        {
            int nextRelease = 1e9;
            for (int i = 0; i < jobCount; i++)
            {
                if (!executed[i] && jobList[i].ReleaseTime < nextRelease)
                {
                    nextRelease = jobList[i].ReleaseTime;
                }
            }
            printf("[Idle] from %d to %d\n", currentTime, nextRelease);

            currentTime = nextRelease;
            continue;
        }

        // Calculs
        int startTime = currentTime;
        int endTime = currentTime + jobList[earliestJob].JobExeTime;
        int waitingTime = currentTime - jobList[earliestJob].ReleaseTime;
        if (waitingTime < 0) waitingTime = 0;
        int responseTime = endTime - jobList[earliestJob].ReleaseTime;

        totalWaitingTime += waitingTime;

        // Table Display
        printf(" %-10s %-10d %-10d %-10d %-10d\n",
               jobList[earliestJob].JobName, startTime, endTime, waitingTime, responseTime);

        // Execution Order
        strcat(executionOrder, jobList[earliestJob].JobName);
        if (finishedJobs < jobCount - 1)
            strcat(executionOrder, "->");

        finishedJobs++;
        executed[earliestJob] = 1;
        currentTime += jobList[earliestJob].JobExeTime;
        totalBusyTime += jobList[earliestJob].JobExeTime;

        // Deadline Miss Verification
        if (!allowMissedDeadlineForT5 || (jobList[earliestJob].JobName[2] != '5'))
        {
            if (currentTime > jobList[earliestJob].AbsoluteDeadline)
            {
                printf("--- WARNING: Deadline Missed for %s ---\n", jobList[earliestJob].JobName);
            }
        }
    }

    totalTime = currentTime;

    // Resume
    printf("\n--- Summary ---\n");
    printf("Execution Order: %s\n", executionOrder);
    printf("Total Busy Time (without Idle): %d\n", totalBusyTime);
    printf("Total Time (including Idle): %d\n", totalTime);
    printf("Total Waiting Time: %d\n", totalWaitingTime);
}
