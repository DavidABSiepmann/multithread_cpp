
#ifndef SOURCE_THREADS_H
#define SOURCE_THREADS_H

#include "thread_utils.h"
#include "profile_print.h"
#include <condition_variable>

/**
 * @brief Estrutura para armazenar buffers da clase de coelta A
 * 
 */
struct buffer_source_A
{
    /// Buffer de dados
    /// Não pode ser manipulado sem ter efetuado a aquisição do semáforo
    int data;
};


/**
 * \brief Classe que define uma fonte de dados
 * 
 * Esta classe tem como objetivo capturar dados de alguma fonte,
 * para deixa-los disponiveis em um buffer circular de tamanho 1.
 * Usa condition_variable para notificar leitores sobre novos dados.
 *  
*/
class source_A : public thread_base
{
    buffer_source_A buffer;
    std::condition_variable cv_;

    public:
        source_A( void ) : thread_base()
        {
            /// Inicializa os valores do buffer
            buffer.data = 0;
        }

        /**
         * @brief Funcão principal da classe source
         * 
         * Função de captura e atualização do buffer de captura
         */
        void run( void ) override;

        /**
         * @brief Efetua leitura do buffer
         * 
         * Realiza a leitura do buffer de modo thread-safe
         * 
         * @param dado ponteiro pegar o valor que está no buffer
         */
        void read( buffer_source_A *dado );
};

#endif