//
// Created by max on 19-12-26.
//

#ifndef PROJECT_TEEEXCEPTION_H
#define PROJECT_TEEEXCEPTION_H

#include <string>
#include <exception>

class TeeException : public std::exception
{
public:
    TeeException( const TeeException & e );
    TeeException( int error_code, const std::string & error_msg );
    virtual ~TeeException() throw(); 

    const char * what() const throw();
    int error_code( ) const;
private:
    int m_error_code;
    std::string m_error_msg;

};

#endif //PROJECT_TEEEXCEPTION_H
