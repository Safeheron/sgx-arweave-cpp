/*
 * Copyright 2020-2022 Safeheron Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.safeheron.com/opensource/license.html
 */


#ifndef SAFEHERON_EXCEPTION_H
#define SAFEHERON_EXCEPTION_H

#include "located_exception.h"

namespace safeheron{
namespace exception{

/**
 * BadAllocException class thrown when a memory allocation fails
 */
class BadAllocException : public LocatedException
{
public:
    explicit BadAllocException(const char * file_path, int line_num, const char * func, int internal_code, const char * message) : LocatedException(file_path, line_num, func, internal_code, message) {}
};

/**
 * RandomSourceException class thrown when a generation of random bytes fails
 */
class RandomSourceException : public LocatedException
{
public:
    explicit RandomSourceException(const char * file_path, int line_num, const char * func, int internal_code, const char * message) : LocatedException(file_path, line_num, func, internal_code, message) {}
};

/**
 * OpensslException class thrown when a exception for error code in openssl library
 */
class OpensslException : public LocatedException
{
public:
    explicit OpensslException(const char * file_path, int line_num, const char * func, int internal_code, const char * message) : LocatedException(file_path, line_num, func, internal_code, message) {}
};

}
}


#endif // SAFEHERON_EXCEPTION_H
