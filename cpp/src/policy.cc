#include "policy.h"
#include <algorithm>
#include <queue>
#include <vector>

struct TaskInfo {
    int taskId;
    int arrivalTime;
    int deadline;
    Event::Task::Priority priority;
    bool inCpu;

    bool operator<(const TaskInfo& other) const {
        if (priority != other.priority) {
            return priority == Event::Task::Priority::kLow; // High priority first
        }
        return deadline > other.deadline; // Earlier deadline first
    }
};

Action policy(const std::vector<Event>& events, int currentCpuTask, int currentIoTask) {
    Action action = {0, 0}; // Default: no task assigned
    std::priority_queue<TaskInfo> taskQueue;

    // Collect all pending tasks from events
    for (const auto& event : events) {
        if (event.type == Event::Type::kTaskArrival && event.task.taskId != 0) {
            TaskInfo task = {
                event.task.taskId,
                event.task.arrivalTime,
                event.task.deadline,
                event.task.priority,
                false
            };
            taskQueue.push(task);
        }
    }

    // Check current tasks
    bool cpuBusy = (currentCpuTask != 0);
    bool ioBusy = (currentIoTask != 0);

    // If CPU is idle, assign a task
    if (!cpuBusy && !taskQueue.empty()) {
        TaskInfo nextTask = taskQueue.top();
        taskQueue.pop();
        if (nextTask.taskId != currentIoTask) { // Avoid conflict with IO
            action.cpuTask = nextTask.taskId;
        }
    }

    // If IO is idle, assign a task (only if not already in CPU)
    if (!ioBusy && !taskQueue.empty()) {
        TaskInfo nextTask = taskQueue.top();
        taskQueue.pop();
        if (nextTask.taskId != currentCpuTask && nextTask.taskId != action.cpuTask) {
            action.ioTask = nextTask.taskId;
        }
    }

    // Ensure no conflict
    if (action.cpuTask == action.ioTask && action.cpuTask != 0) {
        action.ioTask = 0; // Prefer CPU if conflict
    }

    return action;
}