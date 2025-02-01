#include <iostream>
#include <list>
#include <string>
#include <map>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <omp.h>

using namespace std;

struct Task {
    std::string name;
    std::tm deadline;
    std::list<std::string> dependencies;
    int duration;
    bool assigned = false;
};

struct Member {
    std::string name;
    bool isAvailable;
    int maxHours;
    int assignedHours;
    std::map<std::string, int> tasks;
};

std::tm parseDate(const std::string& dateStr) {
    std::tm date = {};
    sscanf(dateStr.c_str(), "%d-%d-%d", &date.tm_year, &date.tm_mon, &date.tm_mday);
    date.tm_year -= 1900;
    date.tm_mon -= 1;
    return date;
}

std::string formatDate(const std::tm& date) {
    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &date);
    return buffer;
}

bool areDependenciesMet(const Task& task, const std::map<std::string, bool>& completedTasks) {
    for (const std::string& dep : task.dependencies) {
        if (!completedTasks.at(dep)) {
            return false;
        }
    }
    return true;
}

void sortTasksByDeadline(std::list<Task>& tasks) {
    tasks.sort([](const Task& a, const Task& b) {
        return std::difftime(mktime(const_cast<std::tm*>(&a.deadline)), mktime(const_cast<std::tm*>(&b.deadline))) < 0;
    });
}

// Start of allocateTasks
void allocateTasks(std::list<Task>& tasks, std::list<Member>& members) {  
    std::map<std::string, bool> completedTasks;
    for (const auto& task : tasks) {
        completedTasks[task.name] = false;
    }

    std::tm currentDate = parseDate("2024-10-03");
    std::list<std::string> memberSchedules;

    clock_t startSort = clock();
    sortTasksByDeadline(tasks);
    clock_t endSort = clock();
    double elapsedSort = double(endSort - startSort) / CLOCKS_PER_SEC;
    printf("Time to sort tasks: %.6f seconds\n", elapsedSort);

    clock_t startAllocation = clock();

    while (!tasks.empty()) {
        for (auto& member : members) {
            member.assignedHours = 0;
            member.tasks.clear();
        }

        bool tasksAssignedToday = false;

        // Start of parallel section 
#pragma omp parallel num_threads(2)  
        {
            bool taskAssignedInRound = false;

            #pragma omp for
            for (auto& member : members) {
                if (!member.isAvailable) continue;

                for (auto it = tasks.begin(); it != tasks.end();) {
                    auto& task = *it;
                    if (areDependenciesMet(task, completedTasks) && member.assignedHours < member.maxHours) {
                        if (member.assignedHours + task.duration <= member.maxHours) {
                            member.tasks[task.name] = task.duration;  
// Race condition: multiple threads may write to member.tasks
                            member.assignedHours += task.duration;  
// Race condition: multiple threads may modify member.assignedHours
                            task.assigned = true;  
// Race condition: multiple threads may modify task.assigned
                            completedTasks[task.name] = true; 
 // Race condition: multiple threads may modify completedTasks
                            it = tasks.erase(it); 
 // Race condition: multiple threads may modify tasks concurrently
                            taskAssignedInRound = true;
                            tasksAssignedToday = true;  
// Race condition: multiple threads may modify tasksAssignedToday
                        } else {
                            ++it;
                        }
                    } else {
                        ++it;
                    }
                }
            }

            if (!taskAssignedInRound) {
                break;
            }
        }  
// End of parallel section

        for (const auto& member : members) {
            if (!member.tasks.empty()) {
                std::string schedule = formatDate(currentDate) + " - Member: " + member.name + " did ";
                schedule += std::to_string(member.tasks.size()) + " tasks with duration " + std::to_string(member.assignedHours) + "h total. Tasks: ";

                for (const auto& task : member.tasks) {
                    schedule += task.first + " (" + std::to_string(task.second) + "h), ";
                }

                if (!member.tasks.empty()) {
                    schedule.erase(schedule.end() - 2, schedule.end());
                }

                memberSchedules.push_back(schedule);
            }
        }

        currentDate.tm_mday++;
        mktime(&currentDate);

        if (!tasksAssignedToday) {
            break;
        }
    }

    clock_t endAllocation = clock();
    double elapsedAllocation = double(endAllocation - startAllocation) / CLOCKS_PER_SEC;
    printf("Time for task allocation process: %.6f seconds\n", elapsedAllocation);

    std::cout << "\nMember Schedules:\n";
    for (const auto& schedule : memberSchedules) {
        std::cout << schedule << "\n";
    }
}  
// End of allocateTasks

void readTasksFromFile(const std::string& filename, std::list<Task>& tasks) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        Task task;
        std::string dependencies;

        std::getline(ss, task.name, ',');
        std::string dateStr;
        std::getline(ss, dateStr, ',');
        task.deadline = parseDate(dateStr);
        std::getline(ss, dependencies, ',');

        std::string durationStr;
        if (std::getline(ss, durationStr, ',')) {
            try {
                if (durationStr.empty()) {
                    std::cerr << "Error: Missing duration for task " << task.name << "\n";
                    continue;
                }
                task.duration = std::stoi(durationStr);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid duration for task " << task.name << ": " << durationStr << "\n";
                continue;
            }
        } else {
            std::cerr << "Error: Missing duration for task " << task.name << "\n";
            continue;
        }

        std::istringstream depStream(dependencies);
        std::string dep;
        while (std::getline(depStream, dep, ',')) {
            task.dependencies.push_back(dep);
        }

        tasks.push_back(task);
    }

    file.close();
}

int main() {
    std::list<Task> tasks;

    clock_t startFileRead = clock();
    readTasksFromFile("tasks.txt", tasks);
    clock_t endFileRead = clock();
    double elapsedFileRead = double(endFileRead - startFileRead) / CLOCKS_PER_SEC;
    printf("Time to read tasks from file: %.6f seconds\n", elapsedFileRead);

    std::list<Member> members = {
        {"Alice", true, 8, 0},
        {"Bob", true, 8, 0},
        {"Sara", true, 6, 0},
        {"Max", true, 8, 0},
    };

    clock_t startTotal = clock();
    
    allocateTasks(tasks, members);
    
    clock_t endTotal = clock();
    double elapsedTotal = double(endTotal - startTotal) / CLOCKS_PER_SEC;
    printf("Total Execution Time: %.6f seconds\n", elapsedTotal);

    return 0;
}