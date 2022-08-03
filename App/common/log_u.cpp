#include "log_u.h"
#include <sys/time.h>
#include <unistd.h>


// Project name in log line prefix
static const char* g_log_project_name = nullptr;

// Get current time in microseconds
static __int64_t time_usec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<__int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

TeeLogHelper::TeeLogHelper( const char* dir,
                            const char* program,
                            const int log_level )
{
    google::InitGoogleLogging(program);

    // if dir is null or doesn't exist,
    // we use "/tmp" as the output folder in default
    if ( dir && strlen(dir) > 0 &&
         access( dir, F_OK ) != -1 ) {
        FLAGS_log_dir = dir;
    }
    g_log_project_name = program;

    // Set whether log messages go to stderr instead of logfiles
    FLAGS_logtostderr = false;

    // Set whether log messages go to stderr in addition to logfiles.
    FLAGS_alsologtostderr = false;

    // Log messages at a level >= this flag are automatically sent to
    // stderr in addition to log files.
    FLAGS_stderrthreshold = google::WARNING;

    // Log messages at a level <= this flag are buffered.
    // Log messages at a higher level are flushed immediately.
    FLAGS_logbuflevel = google::ERROR;

    // Log suppression level: messages logged at a lower level than this
    // are suppressed.
    FLAGS_minloglevel = log_level;

    // Set whether the log prefix should be prepended to each line of output.
    FLAGS_log_prefix = false;

    // Sets the maximum number of seconds which logs may be buffered for.
    FLAGS_logbufsecs = 0;

    // Sets the maximum log file size (in MB).
    FLAGS_max_log_size = 30;

    // Sets whether to avoid logging to the disk if the disk is full.
    FLAGS_stop_logging_if_full_disk = true;
}

TeeLogHelper::~TeeLogHelper()
{
    google::ShutdownGoogleLogging();
}

#ifdef __cplusplus
extern "C"{
#endif

/**
 * Output log messag with glog library.
 * add log prefix by hand, don't use the default prefix in glog (set FLAGS_log_prefix = false)
 * because we need to use a standard log format which is defined
 * by log monitor server.
 * the standard log format is below:
 * yyyy-MM-dd HH:mm:ss.SSS[${PROJECT_NAME}][LEVEL] - message
*/
void ocall_log(uint32_t level, char * message ){
    __int64_t t_usec;
    double wall_time;
    time_t t_now;
    struct tm tm_time;
    char log_time[256] = { 0 };
    char log_prefix[512] = { 0 };

    // get current time in microseconds
    t_usec = time_usec();

    // convert it to seconds
    wall_time = 0.000001 * t_usec;

    // convert it to time_t
    t_now = static_cast<time_t>(wall_time);

    // convert it with local
    localtime_r( &t_now, &tm_time );

    sprintf(log_time, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            1900+tm_time.tm_year, 1+tm_time.tm_mon, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
            static_cast<int32_t>((wall_time - t_now) * 1000) );

    switch ( level ){
        case LL_INFO:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "INFO" );
            LOG(INFO) << log_prefix << message;
            break;
        case LL_WARNING:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "WARNING" );
            LOG(WARNING) << log_prefix << message;
            break;
        case LL_ERROR:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "ERROR" );
            LOG(ERROR) << log_prefix << message;
            break;
//    case LL_FATAL:
//      LOG(FATAL) << message;
//      break;
    }
}

#ifdef __cplusplus
}
#endif


void glog_log(int level, char* log){
    __int64_t t_usec;
    double wall_time;
    time_t t_now;
    struct tm tm_time;
    char log_time[256] = { 0 };
    char log_prefix[512] = { 0 };

    // get current time in microseconds
    t_usec = time_usec();

    // convert it to seconds
    wall_time = 0.000001 * t_usec;

    // convert it to time_t
    t_now = static_cast<time_t>(wall_time);

    // convert it with local
    localtime_r( &t_now, &tm_time );

    sprintf(log_time, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            1900+tm_time.tm_year, 1+tm_time.tm_mon, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
            static_cast<int32_t>((wall_time - t_now) * 1000) );

    switch ( level ){
        case LL_INFO:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "INFO" );
            LOG(INFO) << log_prefix << log;
            break;
        case LL_WARNING:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "WARNING" );
            LOG(WARNING) << log_prefix << log;
            break;
        case LL_ERROR:
            sprintf( log_prefix, "%s[%s][%s] - ", log_time, g_log_project_name, "ERROR" );
            LOG(ERROR) << log_prefix << log;
            break;
//    case LL_FATAL:
//      LOG(FATAL) << message;
//      break;
    }
}

