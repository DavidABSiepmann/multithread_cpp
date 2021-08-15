
#include "source_threads.h"
#include "process_thread.h"
#include "source_process_threads.h"

#ifndef PIPELINE_H
#define PIPELINE_H

class Pipeline
{
    source_A source_Captura;
    process_A process_Captura;
    source_B process_cap_gen;
    process_B process_gen;

    std::thread th_source_Captura, 
                th_process_Captura, 
                th_process_cap_gen, 
                th_process_gen;

    public:
        Pipeline( void ) :  process_Captura(&source_Captura), 
                            process_cap_gen(&source_Captura), 
                            process_gen(&process_cap_gen){};
        void start( void )
        {
            /// Inicia a thread de source
            th_source_Captura = std::thread( &source_A::start, &source_Captura );

            /// Inicia thread de processamento, para Source_A
            th_process_Captura = std::thread( &process_A::start, &process_Captura );

            /// Inicia thread de processamento e geração
            th_process_cap_gen = std::thread( &source_B::start, &process_cap_gen );
            
            /// Inicia thread de processamento, para source_B
            th_process_gen = std::thread( &process_B::start, &process_gen );
        }

        void stop( void )
        {
            //Envia sinal parar as threads
            source_Captura.stop();
            process_Captura.stop();
            process_cap_gen.stop();
            process_gen.stop();

            //Aguarda as threads finalizarem
            th_source_Captura.join();
            th_process_Captura.join();
            th_process_cap_gen.join();
            th_process_gen.join();
        }
};

#endif