
#include "thread_utils.h"
#include "source_process_threads.h"
#include "bench_config.h"
#include "bench_metrics.h"

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
    /// Ponteiro para uma classe de source
    source_A *cap;

    /// Optional pointer to config (no global external dependency)
    BenchConfig *cfg;

    /**
     * @brief Inicializa a classe
     * 
     * @param cap_ ponteiro para classe source
     */
    void init( source_A *cap_, BenchConfig *cfg_ = nullptr )
    {
        cap = cap_;
        cfg = cfg_;
    }

    public:
        /**
         * @brief Contrutor da classe process_A
         * 
         */
        process_A( void ) : thread_base()
        { 
            this->init( NULL, nullptr );
        };

        /**
         * @brief Contrutor da classe process_A
         * 
         * @param cap_ ponteiro para classe source
         */
        process_A( source_A *cap_ ) : thread_base()
        { 
            this->init(cap_, nullptr);
        };

        process_A( source_A *cap_, BenchConfig *cfg_ ) : thread_base()
        {
            this->init(cap_, cfg_);
        };

        /**
         * @brief Função principal
         * 
         * Efetua o processamento das informações do buffer,
         * de modo continuo. Usa condition_variable para
         * sincronizar com a source.
         * 
         */
        void run( void ) override;

        /**
         * @brief Efetua o processamento do dado lido
         * 
         * @param buffer ponteiro para o dado
         */
        void process_buffer( int *buffer );
};


/**
 * @brief Classe de processamento
 * 
 * Realiza o processamento de uma dado coletado uma classe source
 * 
 */
class process_B : public thread_base
{
    /// Ponteiro para uma classe de source
    source_B *cap;

    /// Optional pointer to config (no global external dependency)
    BenchConfig *cfg;

    /**
     * @brief Inicializa a classe
     * 
     * @param cap_ ponteiro para classe source
     */
    void init( source_B *cap_, BenchConfig *cfg_ = nullptr )
    {
        cap = cap_;
        cfg = cfg_;
    }

    public:
        /**
         * @brief Contrutor da classe process_B
         * 
         */
        process_B( void ) : thread_base()
        { 
            this->init( NULL, nullptr );
        };

        /**
         * @brief Contrutor da classe process_B
         * 
         * @param cap_ ponteiro para classe source
         */
        process_B( source_B *cap_ ) : thread_base()
        { 
            this->init(cap_, nullptr);
        };

        process_B( source_B *cap_, BenchConfig *cfg_ ) : thread_base()
        {
            this->init(cap_, cfg_);
        };

        /**
         * @brief Função principal
         * 
         * Efetua o processamento das informações do buffer,
         * de modo continuo. Usa condition_variable para
         * sincronizar com a source.
         * 
         */
        void run( void ) override;

        /**
         * @brief Efetua o processamento do dado lido
         * 
         * @param buffer ponteiro para o dado
         */
        void process_buffer( int *buffer );
};

#endif