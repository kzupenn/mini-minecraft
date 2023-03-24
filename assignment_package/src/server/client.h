#pragma once


class Client
{
private:
    static void* receive_messages(void*arg);
public:
    int start();
};

