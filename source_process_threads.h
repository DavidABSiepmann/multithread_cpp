
#include "thread_utils.h"
#include "source_threads.h"

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

    /// Index de atualização do buffer, a cada mudança do
    /// buffer index será incrementado 1
    uint8_t index;
};


/**
 * @brief Classe de processo e source
 * 
 * Está classe efetua o processamento de uma outra source,
 * gerando uma nova informação que poderá ser utilizada por outras
 * threads. Devendo ser thread-safe
 * 
 */
class source_B : public thread_base
{
    /// Armazena o ultimo index lido da source
    uint8_t index;

    source_A *cap;

    buffer_source_B buffer;

    /**
     * @brief Inicializa o classe
     * 
     * @param cap_ ponteiro para classe de captura
     */
    void init( source_A *cap_ )
    {
        index = 0;
        cap = cap_;

        buffer.data = 0;
        buffer.index = 0;
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
        void run( void )
        {
            if( !cap )
            {
                printf("[source_B] Capture não inicado!!\n");
                return;
            }

            buffer_source_A val;
            cap->read( &val );

            if ( val.index != index )
            {
                process_buffer( &val.data );
                //Simula tempo do processo
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            index = val.index;
        }

        /**
         * @brief Efetua o processamento do dado lido, gerando um novo dado
         * 
         * @param buffer ponteiro para o dado
         */
        void process_buffer( int *value )
        {
            printf("[source_B] dado lido val: %d\n", *value);

            int temp_value = *value + 1000;

            //Simula tempo processamento
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            mtx.lock();

            buffer.data = temp_value;
            buffer.index++;
            
            //Simula tempo atualizar buffer
            std::this_thread::sleep_for(std::chrono::milliseconds(2));

            mtx.unlock();
        }

        /**
         * @brief Efetua leitura do buffer, do valor que foi gerado
         * utilizando a outra source
         * 
         * Realiza a leitura do buffer de modo thread-safe
         * 
         * @param dado ponteiro pegar o valor que está no buffer
         * @return int valor do index de atualização
         */
        uint8_t read( buffer_source_B *dado )
        {
            uint8_t ret = 0;

            mtx.lock();

            *dado = buffer;
            ret = buffer.index;

            mtx.unlock();

            return ret;
        }
};

#endif