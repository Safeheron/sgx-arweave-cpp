#ifndef PROJECT_DISPATCHER_H
#define PROJECT_DISPATCHER_H
#include <iostream>
#include <vector>
#include <string>

class Task{
public:
    virtual int get_task_type( ) = 0;
    virtual int execute( const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg ) = 0;
protected:
    std::string request_id_;
};

class Dispatcher
{
public:
    int dispatch( uint32_t task_type, const std::string & request_id, const std::string & request, std::string & reply, std::string & error_msg );
    void register_task( Task * t );

private:
    std::vector<Task *> m_vTask;

};

#endif //PROJECT_DISPATCHER_H
