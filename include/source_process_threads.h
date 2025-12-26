
#include "thread_utils.h"
#include "source_threads.h"
#include <condition_variable>

#ifndef SOURCE_PROCESS_THREADS_H
#define SOURCE_PROCESS_THREADS_H

/**
 * @brief Armazena os dados utilizado pela classe
 * 
 * Neste exemplo serão os mesmo dados, não haveria necessídade de criar. 
 * Mas como poderiam ser diferentes será criado outra struct. 
 * 
 */
struct buffer_source_B
{
    /// Buffer de dados
    /// Não pode ser manipulado sem ter efetuado a aquisição do semáforo
    int data;
};


/**
 * @brief Classe de processo e source
 * 
 * Está classe efetua o processamento de uma outra source,
 * gerando uma nova informação que poderá ser utilizada por outras
 * threads. Devendo ser thread-safe. Usa condition_variable para
 * notificar leitores sobre novos dados processados.
 * 
 */
class source_B : public thread_base
{
    source_A *cap;

    buffer_source_B buffer;
    std::condition_variable cv_;

    /**
     * @brief Inicializa o classe
     * 
     * @param cap_ ponteiro para classe de captura
     */
    void init( source_A *cap_ )
    {
        cap = cap_;
        buffer.data = 0;
    }

    public:

        /**
         * @brief Contrutor para source_B 
         * 
         */
        source_B( void ) : thread_base()
        { 
            this->init( NULL );
        };

        /**
         * @brief Contrutor para source_B 
         * 
         * @param cap_ ponteiro para classe captura
         */
        source_B( source_A *cap_ ) : thread_base()
        { 
            this->init(cap_);
        };

        /**
         * @brief Função principal
         * 
         * Efetua o processamento das informações do buffer,
         * de modo continuo. Gerando uma nova informação, sendo
         * disponibilizada pelo seu buffer
         * 
         */
        void run( void ) override;

        /**
         * @brief Efetua o processamento do dado lido, gerando um novo dado
         * 
         * @param buffer ponteiro para o dado
         */
        void process_buffer( int *value );

        /**
         * @brief Efetua leitura do buffer, do valor que foi gerado
         * utilizando a outra source
         * 
         * Realiza a leitura do buffer de modo thread-safe
         * 
         * @param dado ponteiro pegar o valor que está no buffer
         */
        void read( buffer_source_B *dado );
        // implementations moved to src/source_process_threads.cpp
};

#endif