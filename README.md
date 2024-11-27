# Parallel Task Scheduling for Deadline-Oriented Systems

## Project Overview

This project aims to develop an efficient task scheduling system that considers team members' working hours, deadlines, and dependencies between tasks. The primary goal is to transition from a sequential execution model to a parallel processing model, thereby enhancing performance and reducing overall execution time.

---

##  Objective 

- Measure the performance of the sequential code to identify the execution time, the most time-consuming part of the code and the parts to be parallelized.
- Develop a parallel code version that uses multiple processing units to improve performance. 
- compare the performance of the sequential and parallel code to assess improvements in execution time and efficiency.
 
---

## Sequential Benchmarking 

| Number of Tasks | Execution Time (seconds) |
|------------------|--------------------------|
| 100              | 0.0170s                  |
| 500              | 0.1220s                  |
| 1000             | 0.3280s                  |
| 2000             | 1.1660s                  |
| 3000             | 2.6330s                  |
| 5000 tasks       | 5.9520s                  |

## Time-Consuming Parts of Code Execution (1000 tasks)
The execution time of each function in the sequential code was measured using the clock() function on a dataset consisting of 1,000 tasks.

| Function          | Execution Time (seconds) |
|-------------------|--------------------------|
| Read from file    | 0.0040s                  |
| Sort tasks        | 0.0030s                  |
| Task allocation   | 0.1600s                  |

---

## Parallelization Plan

The **task allocation function** is identified as the most time-consuming part of the code. To improve performance, we will parallelize this function and ensure that the results are correct by addressing any potential race conditions that may arise during parallel execution. This involves solving issues that occur when multiple threads access shared resources concurrently, leading to data inconsistency.

---

## Identified Race Conditions During Parallelization

While parallelizing the task allocation function, race conditions occurred in some variables:

- **member.tasks**: A map that stores the member's tasks and the duration of each. Multiple threads accessing it could cause data corruption or task overwriting.
- **member.assignedHours**: An integer that tracks the total hours assigned to a member. Concurrent thread updates could lead to incorrect totals due to data races.
- **task.assigned**: A boolean variable that marks whether a task has been assigned to a member or not. Concurrent updates could lead to inconsistent assignment states.
- **completedTasks**: A map tracking whether task dependencies are completed. Concurrent updates could result in incorrect or missed updates.
- **tasks List**: A list of unassigned tasks. Concurrent modifications, such as erasing tasks, could lead to iterator invalidation or list corruption.
- **tasksAssignedToday**: A boolean indicating if any tasks were assigned today. Concurrent updates could cause incorrect day-end evaluations.

---

## Parallelization Performance (Multiple Cores)

Shown below is the execution time with different numbers of cores for both full code and task allocation:

| Synchronization Method | 1 Thread  | 2 Thread  | 4 Thread  | 8 Thread  |
|------------------------|-----------|-----------|-----------|-----------|
| **No Synchronization** |           |           |           |           |
| Full code              | 0.1580s   | 0.1480s   | 0.1570s   | 0.1510s   |
| Task allocation        | 0.0740s   | 0.0730s   | 0.0710s   | 0.0710s   |
| **Critical**           |           |           |           |           |
| Full code              | 0.1690s   | 0.1520s   | 0.1560s   | 0.1580s   |
| Task allocation        | 0.0780s   | 0.0760s   | 0.0800s   | 0.0790s   |
| **Atomic**             |           |           |           |           |
| Full code              | 0.1580s   | 0.1520s   | 0.1540s   | 0.1780s   |
| Task allocation        | 0.0790s   | 0.0790s   | 0.0740s   | 0.0710s   |
| **Reduction**          |           |           |           |           |
| Full code              | 0.1430s   | 0.1440s   | 0.1440s   | 0.1480s   |
| Task allocation        | 0.0720s   | 0.0710s   | 0.0750s   | 0.0740s   |

---

