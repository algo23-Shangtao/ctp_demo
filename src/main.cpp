#include "mduserhandler.h"

int main()
{
    CMduserHandler *mduser = new CMduserHandler;
    mduser->connect();
    mduser->login();
    mduser->subscribe();
    mduser->join();
}
//g++ src/main.cpp  src/mduserhandler.cpp src/params.cpp -Iinclude_origin/ -Iinclude_my/ -lthostmduserapi_se -Llib/ -g -o main
//LD_LIBRARY_PATH=lib ./main
