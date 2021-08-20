
#ifndef SORCE_THREADS_H
#define SORCE_THREADS_H

#include "thread_utils.h"
#include "profile_print.h"

/**
 * @brief Estrutura para armazenar buffers da clase de coelta A
 * 
 */
struct buffer_source_A
{
    /// Buffer de dados
    /// Não pode ser manipulado sem ter efetuado a aquisição do semáforo
    int data;

    /// Index de atualização do buffer, a cada mudança do
    /// buffer index será incrementado 1
    uint8_t index;
};


/**
 * \brief Classe que define uma fonte de dados
 * 
 * Esta classe tem como objetivo capturar dados de alguma fonte,
 * para deixa-los disponiveis em um buffer circular de tamanho 1.
 *  
*/
class source_A : public thread_base
{
    buffer_source_A buffer;

    public:
        source_A( void ) : thread_base()
        {
            /// Inicializa os valores do buffer
            buffer.data = 0;
            buffer.index = 0;
        }

        /**
         * @brief Funcão principal da classe source
         * 
         * Função de captura e atualização do buffer de captura
         */
        void run( void )
        {
            /// Adquire o semáforo, para utilizar o ultimo valor do buffer
            mtx.lock();
            startProfile("sA_mtx");

            /// Simula a aquisição de um novo valor
            int temp_buffer = buffer.data + 5;

            /// Siluma tempo para copiar o valor
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
            
            stopProfile("sA_mtx");
            /// Libera o semáforo
            mtx.unlock();
            startProfile("sA");

            /// Siluma tempo para capturar o novo valor
            std::this_thread::sleep_for(std::chrono::milliseconds(45));
            
            stopProfile("sA");

            /// Adquire o semáforo, para modificar buffer
            mtx.lock();
            startProfile("sA_mtx");

            /// Atualiza buffer com o novo valor
            buffer.data = temp_buffer;

            /// Atualiza indice de mudanças
            buffer.index++;

            /// Simula tempo para efetuar a alteração no dado do buffer
            std::this_thread::sleep_for(std::chrono::milliseconds(5));

            stopProfile("sA_mtx");
            /// Libera o semáforo
            mtx.unlock();

            /// Debug informa que um novo valor foi gerado
            // printf("[source_A] novo valor: %d\n", buffer.data);
        }

        /**
         * @brief Efetua leitura do buffer
         * 
         * Realiza a leitura do buffer de modo thread-safe
         * 
         * @param dado ponteiro pegar o valor que está no buffer
         * @return int valor do index de atualização
         */
        uint8_t read( buffer_source_A *dado )
        {
            uint8_t ret = 0;

            mtx.lock();
            startProfile("sA_mtx");

            *dado = buffer;
            ret = buffer.index;

            stopProfile("sA_mtx");
            mtx.unlock();

            return ret;
        }
};

#endif