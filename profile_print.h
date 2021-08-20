
#ifndef PROFILE_PRINT
#define PROFILE_PRINT

#include<chrono>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

void startProfile( const char *name )
{
    printf("%s,%ld,0\n", name, duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
    printf("%s,%ld,1\n", name, duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
}

void stopProfile( const char *name )
{
    printf("%s,%ld,1\n", name, duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
    printf("%s,%ld,0\n", name, duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count());
}

#endif