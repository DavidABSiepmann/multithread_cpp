#include <iostream>
#include <chrono>

#include "pipeline.h"

int main( void )
{
    Pipeline mt;

    //Inicia pipeline
    mt.start();

    //Aguarda 1 segundos
    std::this_thread::sleep_for(std::chrono::milliseconds(1*1000));

    //Encerra pipeline
    mt.stop();

    return 0;
}