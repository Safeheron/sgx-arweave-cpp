#ifndef HANDLER_H
#define HANDLER_H

#include "../controller/controller.h"


class handler
{
    public:
        handler(string url);
        virtual ~handler();

        pplx::task<void>open(){return m_listener_.open();}
        pplx::task<void>close(){return m_listener_.close();}

    protected:

    private:

        void HandlePost(http_request message);
        http_listener m_listener_;


};

#endif // HANDLER_H


