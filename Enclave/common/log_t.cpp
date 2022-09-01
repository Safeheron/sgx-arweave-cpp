#include "log_t.h"
#include "Enclave_t.h"

/**
 * Output log to file by ocall_log()
*/
void tee_log(int level, char* log)
{
    ocall_log(level, log);
}
