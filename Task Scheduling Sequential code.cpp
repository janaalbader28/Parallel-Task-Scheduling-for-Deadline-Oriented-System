#include <iostream>
#include <list>
#include <string>
#include <map>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

// Task structure
struct Task {
    std::string name;
    std::tm deadline;  // Deadline as a date
    std::list<std::string> dependencies;  // Task dependencies
    int duration;  // Estimated duration in hours
    bool assigned = false;  // Task assignment status
};

// Member structure
struct Member {
    std::string name;
    bool isAvailable;  // Availability status of team members
    int maxHours;      // Maximum hours member can work
    int assignedHours; // Total hours assigned to the member
    std::map<std::string, int> tasks; // Assigned tasks with their durations
};

// Helper function to parse a date input as "YYYY-MM-DD"
std::tm parseDate(const std::string& dateStr) {
    std::tm date = {};
    sscanf(dateStr.c_str(), "%d-%d-%d", &date.tm_year, &date.tm_mon, &date.tm_mday);
    date.tm_year -= 1900; // Adjust for struct tm format (years since 1900)
    date.tm_mon -= 1;     // Adjust for months starting from 0
    return date;
}

// Helper function to print a date
std::string formatDate(const std::tm& date) {
    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &date);
    return buffer;
}

// Check if dependencies are met for a task
bool areDependenciesMet(const Task& task, const std::map<std::string, bool>& completedTasks) {
    for (const std::string& dep : task.dependencies) {
        if (!completedTasks.at(dep)) {
            return false;  // Dependency not met
        }
    }
    return true;
}

// Function to sort tasks by deadline
void sortTasksByDeadline(std::list<Task>& tasks) {
    tasks.sort([](const Task& a, const Task& b) {
        return std::difftime(mktime(const_cast<std::tm*>(&a.deadline)), mktime(const_cast<std::tm*>(&b.deadline))) < 0;
    });
}

// Function to allocate tasks to available members based on deadlines and dependencies
void allocateTasks(std::list<Task>& tasks, std::list<Member>& members) {
    std::map<std::string, bool> completedTasks;
    for (const auto& task : tasks) {
        completedTasks[task.name] = false;  // Initialize all tasks as incomplete
    }

    std::tm currentDate = parseDate("2024-10-03"); // Start date
    std::list<std::string> memberSchedules;

    // While there are tasks to allocate
    while (!tasks.empty()) {
        // Reset assigned hours for the day
        for (auto& member : members) {
            member.assignedHours = 0;
            member.tasks.clear(); // Clear tasks for the new day
        }

        // Sort tasks by deadline
        sortTasksByDeadline(tasks);

        bool tasksAssignedToday = false;

        // While there are tasks to allocate
        while (true) {
            bool taskAssignedInRound = false;

            for (auto& member : members) {
                if (!member.isAvailable) continue;  // Skip unavailable members

                // Attempt to assign tasks to the current member until they reach their max hours
                for (auto it = tasks.begin(); it != tasks.end();) {
                    auto& task = *it;
                    if (areDependenciesMet(task, completedTasks) && member.assignedHours < member.maxHours) {
                        if (member.assignedHours + task.duration <= member.maxHours) {
                            // Assign task to member
                            member.tasks[task.name] = task.duration; // Store task duration
                            member.assignedHours += task.duration;  // Update member's assigned hours
                            task.assigned = true;
                            completedTasks[task.name] = true;  // Mark task as completed
                            it = tasks.erase(it);  // Remove the task from the list
                            taskAssignedInRound = true;
                            tasksAssignedToday = true; // Mark that tasks were assigned today
                        } else {
                            ++it;  // Move to the next task if it exceeds the member's daily limit
                        }
                    } else {
                        ++it;  // Move to the next task if dependencies are not met
                    }
                }
            }

            // If no tasks were assigned in this round, break to avoid infinite loop
            if (!taskAssignedInRound) {
                break;
            }
        }

        // Output the assignments for the day
        for (const auto& member : members) {
            if (!member.tasks.empty()) {
                std::string schedule = formatDate(currentDate) + " - Member: " + member.name + " did ";
                schedule += std::to_string(member.tasks.size()) + " tasks with duration " + std::to_string(member.assignedHours) + "h total. Tasks: ";

                // Append the task names and durations
                for (const auto& task : member.tasks) {
                    schedule += task.first + " (" + std::to_string(task.second) + "h), ";
                }

                // Remove the last comma and space
                if (!member.tasks.empty()) {
                    schedule.erase(schedule.end() - 2, schedule.end());
                }

                memberSchedules.push_back(schedule);
            }
        }

        // Move to the next day
        currentDate.tm_mday++;
        mktime(&currentDate); // Normalize the date (handles overflow into next month/year)

        // If no tasks were assigned at all, break to avoid infinite loop
        if (!tasksAssignedToday) {
            break;  // No further allocation possible
        }
    }

    // Print detailed schedule for each member
    std::cout << "\nMember Schedules:\n";
    for (const auto& schedule : memberSchedules) {
        std::cout << schedule << "\n";
    }
}

// Function to read tasks from a file
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

        // Read task details: name, deadline, dependencies (comma separated), duration
        std::getline(ss, task.name, ',');
        std::string dateStr;
        std::getline(ss, dateStr, ',');
        task.deadline = parseDate(dateStr);
        std::getline(ss, dependencies, ',');

        // Extracting the duration
        std::string durationStr;
        if (std::getline(ss, durationStr, ',')) {
            try {
                if (durationStr.empty()) {
                    std::cerr << "Error: Missing duration for task " << task.name << "\n";
                    continue; // Skip this task if duration is missing
                }
                task.duration = std::stoi(durationStr); // Convert duration to integer
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid duration for task " << task.name << ": " << durationStr << "\n";
                continue; // Skip this task if duration is invalid
            }
        } else {
            std::cerr << "Error: Missing duration for task " << task.name << "\n";
            continue; // Skip this task if duration is missing
        }

        // Parse dependencies
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
    // Define tasks list
    std::list<Task> tasks;

    // Read tasks from a file
    readTasksFromFile("tasks.txt", tasks);

    // Define team members and their availability
    std::list<Member> members = {
        {"Alice", true, 8, 0},  // Can work up to 8 hours
        {"Bob", true, 8, 0},    // Can work up to 8 hours
        {"Sara", true, 6, 0}, 
        {"Max", true, 8, 0}, 
    };

    // Measure execution time
    clock_t start = clock();

    // Allocate tasks
    allocateTasks(tasks, members);

    clock_t end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    printf("Execution Time: %.6f seconds\n", elapsed);  // Print the time

    return 0;
}