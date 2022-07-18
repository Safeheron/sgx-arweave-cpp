//
// Created by max on 4/17/20.
//

#ifndef PROJECT_GLOG_HELPER_H
#define PROJECT_GLOG_HELPER_H

class GLogHelper
{
  public:
  /**
  * dir: log files output dir, use "/tmp" is it is null.
  * program: the program name string, as the log file prefix
  * log_level: message logged at level >= this level will be logged
  */
  GLogHelper(const char* dir, const char* program, const int log_level = 0);
  ~GLogHelper();
};

#endif //PROJECT_GLOG_HELPER_H
