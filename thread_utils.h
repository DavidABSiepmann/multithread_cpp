
#include <iostream>
#include <thread>         // std::thread
#include <mutex>          // std::mutex

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
        /// Indica se a thread está ativa ou não
        bool active;

    public:
        /// Semáforo para utilização de variáveis sensíveis
        std::mutex mtx;

        thread_base( void )
        {
            active = false;
        }

        /**
         * @brief Inicia thread
         * 
         * Função para iniciar o funcionamento da thread, que deverá conter
         * o laço infinito para execução continua.
         * 
         */
        virtual void start ( void )
        {
            if ( active )
                return;
            
            active = true;

            while( active )
            {
                this->run( );
            }
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
            active = false;
        }

        /**
         * @brief Verifica se a thread está ativa
         * 
         * @return true está ativa
         * @return false não está ativa
         */
        bool isActive ( void )
        {
            return active;
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