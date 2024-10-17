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
