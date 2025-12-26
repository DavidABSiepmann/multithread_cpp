
#include <iostream>
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <atomic>

#ifndef THREADS_UTILS_H
#define THREADS_UTILS_H

/**
 * @brief Classe base para funcionamento das thread
 * 
 * Implementas as funções principais para trabalhar com uma thread
 * no formato de pipeline
*/
class thread_base
{
    private:
        /// Indica se a thread está ativa ou não (thread-safe)
        std::atomic<bool> active{false};
        
        /// Thread worker (stored here so Pipeline can manage it)
        std::thread worker_thread;
        
        /**
         * @brief Loop principal executado em thread separada
         * 
         * Esta é a função que roda na std::thread criada em start().
         * Executa o laço infinito e trata exceções.
         */
        void thread_main()
        {
            while( active.load(std::memory_order_acquire) )
            {
                try
                {
                    this->run();
                }
                catch( const std::exception &e )
                {
                    std::cerr << "[thread_base] Exceção capturada: " << e.what() << std::endl;
                }
                catch( ... )
                {
                    std::cerr << "[thread_base] Exceção desconhecida capturada" << std::endl;
                }
            }
        }

    public:
        /// Semáforo para utilização de variáveis sensíveis
        std::mutex mtx;

        thread_base( void ) = default;
        virtual ~thread_base() { stop(); } // polymorphic base should have virtual dtor and cleanup

        /**
         * @brief Inicia thread
         * 
         * Função para iniciar o funcionamento da thread, que deverá conter
         * o laço infinito para execução continua. Cria uma std::thread de verdade.
         * 
         */
        virtual void start ( void )
        {
            if ( active.load(std::memory_order_acquire) )
                return;
            
            active.store(true, std::memory_order_release);
            worker_thread = std::thread( &thread_base::thread_main, this );
        }

        /**
         * @brief Para thread
         * 
         * Função que deve executar a finalização da thread
         * que não poderá mais ser reativa
         * 
         */
        virtual void stop ( void )
        {
            active.store(false, std::memory_order_release);
            if ( worker_thread.joinable() )
                worker_thread.join();
        }

        /**
         * @brief Verifica se a thread está ativa
         * 
         * @return true está ativa
         * @return false não está ativa
         */
        bool isActive ( void )
        {
            return active.load(std::memory_order_acquire);
        }

        /**
         * @brief Função principal
         * 
         * Executa a função principal da thread, que deverá ser escrita 
         * na class herdeira
         * 
         */
        virtual void run ( void ) = 0;
};

#endif