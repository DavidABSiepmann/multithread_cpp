
#include "thread_utils.h"
#include "source_process_threads.h"

#ifndef PROCESS_THREADS_H
#define PROCESS_THREADS_H

/**
 * @brief Classe de processamento
 * 
 * Realiza o processamento de uma dado coletado uma classe source
 * 
 */
class process_A : public thread_base
{
    /// Index do ultimo dado coletado
    uint8_t index;

    /// Ponteiro para uma classe de source
    source_A *cap;

    /**
     * @brief Inicializa a classe
     * 
     * @param cap_ ponteiro para classe source
     */
    void init( source_A *cap_ )
    {
        index = 0;
        cap = cap_;
    }

    public:
        /**
         * @brief Contrutor da classe process_A
         * 
         */
        process_A( void ) : thread_base()
        { 
            this->init( NULL );
        };

        /**
         * @brief Contrutor da classe process_A
         * 
         * @param cap_ ponteiro para classe source
         */
        process_A( source_A *cap_ ) : thread_base()
        { 
            this->init(cap_);
        };

        /**
         * @brief Função principal
         * 
         * Efetua o processamento das informações do buffer,
         * de modo continuo
         * 
         */
        void run( void )
        {
            /// Verifica se a classe foi iniciada corretamente,
            /// pode ser utilizado um throw
            if( !cap )
            {
                printf("[process_A] Capture não inicado!!\n");
                return;
            }
            
            /// Armazena valor do buffer nesta classe
            buffer_source_A val;

            /// Efetua leitura do buffer e o index
            cap->read( &val );

            /// Verifica se é um valor novo
            if ( val.index != index )
            {
                /// Efetua o processamento
                process_buffer( &val.data );

                /// Simula o tempo de efeutar o processo
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }

            index = val.index;
        }

        /**
         * @brief Efetua o processamento do dado lido
         * 
         * @param buffer ponteiro para o dado
         */
        void process_buffer( int *buffer )
        {
            printf("[process_A] salvando val: %d\n", *buffer);
        }
};


/**
 * @brief Classe de processamento
 * 
 * Realiza o processamento de uma dado coletado uma classe source
 * 
 */
class process_B : public thread_base
{
    /// Index do ultimo dado coletado
    uint8_t index;

    /// Ponteiro para uma classe de source
    source_process *cap;

    /**
     * @brief Inicializa a classe
     * 
     * @param cap_ ponteiro para classe source
     */
    void init( source_process *cap_ )
    {
        index = 0;
        cap = cap_;
    }

    public:
        /**
         * @brief Contrutor da classe process_A
         * 
         */
        process_B( void ) : thread_base()
        { 
            this->init( NULL );
        };

        /**
         * @brief Contrutor da classe process_A
         * 
         * @param cap_ ponteiro para classe source
         */
        process_B( source_process *cap_ ) : thread_base()
        { 
            this->init(cap_);
        };

        /**
         * @brief Função principal
         * 
         * Efetua o processamento das informações do buffer,
         * de modo continuo
         * 
         */
        void run( void )
        {
            /// Verifica se a classe foi iniciada corretamente,
            /// pode ser utilizado um throw
            if( !cap )
            {
                printf("[process_B] Capture não inicado!!\n");
                return;
            }
            
            /// Armazena valor do buffer nesta classe
            buffer_source_process val;

            /// Efetua leitura do buffer e o index
            cap->read( &val );

            /// Verifica se é um valor novo
            if ( val.index != index )
            {
                /// Efetua o processamento
                process_buffer( &val.data );

                /// Simula o tempo de efeutar o processo
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }

            index = val.index;
        }

        /**
         * @brief Efetua o processamento do dado lido
         * 
         * @param buffer ponteiro para o dado
         */
        void process_buffer( int *buffer )
        {
            printf("[process_B] salvando val: %d\n", *buffer);
        }
};

#endif