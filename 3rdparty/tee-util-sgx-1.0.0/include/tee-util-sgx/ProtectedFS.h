//
// Created by max on 20-3-28.
//

#ifndef PROJECT_PROTECTEDFS_H
#define PROJECT_PROTECTEDFS_H

#include <string>

class ProtectedFS {
 public:
    int read(const char * file_path, uint8_t ** buf, size_t *len);
    int write(const char * file_path, const uint8_t * buf, size_t len);
};


#endif //PROJECT_PROTECTEDFS_H
