
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

    public:
        // Pipeline now accepts an optional BenchConfig pointer so workers can access config without globals
        Pipeline( BenchConfig *cfg = nullptr ) :
            source_Captura(),
            process_Captura(&source_Captura, cfg),
            process_cap_gen(&source_Captura),
            process_gen(&process_cap_gen, cfg) {}
        
        ~Pipeline()
        {
            stop();
        }

        void start( void )
        {
            /// Inicia a thread de source
            source_Captura.start();

            /// Inicia thread de processamento, para Source_A
            process_Captura.start();

            /// Inicia thread de processamento e geração
            process_cap_gen.start();
            
            /// Inicia thread de processamento, para source_B
            process_gen.start();
        }

        void stop( void )
        {
            //Envia sinal parar as threads (stop() agora também faz join() internamente)
            source_Captura.stop();
            process_Captura.stop();
            process_cap_gen.stop();
            process_gen.stop();
        }
};

#endif