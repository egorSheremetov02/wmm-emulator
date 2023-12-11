#ifndef USER_EXECUTOR_H
#define USER_EXECUTOR_H

#include "controllable_executor.h"

struct UserExecutor {
    ControllableExecutor controllable_executor;
    bool tracing_on;

    UserExecutor(ControllableExecutor controllable_executor, bool tracing_on);
    virtual bool IsDone() const;
    virtual size_t Select() const = 0;
    virtual void ExecuteNext();
    virtual void PrintSnapshot();
    virtual ~UserExecutor() = default;
};

#endif //USER_EXECUTOR_H