#include <iostream>
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <chrono>
#include <thread>

// /usr/bin/g++ -g main.cpp -pthread -o main

class thread_base
{
    bool active;
    public:
        thread_base( void )
        {
            active = false;
        }
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
        virtual void stop ( void )
        {
            active = false;
        }
        bool isActive ( void )
        {
            return active;
        }
        virtual void run ( void ) = 0;
};

class capture : public thread_base
{
    std::mutex mtx;
    int buffer;
    uint8_t index = 0;
    public:
        capture( void ) : thread_base()
        {
            buffer = 0;
        }
        void run( void )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(45));
            mtx.lock();

            buffer+=5;
            index++;

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            mtx.unlock();

            printf("[capture] novo valor: %d\n", buffer);
        }
        int read( int *dado )
        {
            int ret = 0;

            mtx.lock();

            *dado = buffer;
            ret = index;

            mtx.unlock();

            return ret;
        }
};

class save : public thread_base
{
    uint8_t index;
    capture *cap;

    void init( capture *cap_ )
    {
        index = 0;
        cap = cap_;
    }

    public:
        save( void ) : thread_base()
        { 
            this->init( NULL );
        };
        save( capture *cap_ ) : thread_base()
        { 
            this->init(cap_);
        };
        void run( void )
        {
            if( !cap )
            {
                printf("Capture não inicado!!\n");
                return;
            }
            
            int val;
            uint8_t index_temp = cap->read( &val );

            if ( index_temp != index )
            {
                printf("[save] salvando val: %d\n", val);

                //Simula o tempo de efeutar o processo
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            // else
            //     printf("[save] valor repetido val: %d\n", val);

            index = index_temp;
        }
};

class process : public thread_base
{
    uint8_t index;
    capture *cap;

    void init( capture *cap_ )
    {
        index = 0;
        cap = cap_;
    }

    public:
        process( void ) : thread_base()
        { 
            this->init( NULL );
        };
        process( capture *cap_ ) : thread_base()
        { 
            this->init(cap_);
        };
        void run( void )
        {
            if( !cap )
            {
                printf("Capture não inicado!!\n");
                return;
            }

            int val;
            uint8_t index_temp = cap->read( &val );

            if ( index_temp != index )
            {
                printf("[process] processando val: %d\n", val);
                //Simula tempo do processo
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            // else
            //     printf("[process] valor repetido: %d\n", val);
            
            index = index_temp;
        }
};

class Pipeline
{
    std::mutex semaforo;

    capture cap;
    save sv;
    process pr;

    std::thread t_cap, t_save, t_pr;

    public:
        Pipeline( void )
        {
            sv = save( &cap );
            pr = process( &cap );
        }
        void start( void )
        {
            //Cria e inicia as threads
            t_cap = std::thread( &capture::start, &cap );
            t_save = std::thread( &save::start, &sv );
            t_pr = std::thread( &process::start, &pr );
        }

        void stop( void )
        {
            //Envia sinal parar as threads
            cap.stop();
            sv.stop();
            pr.stop();

            //Aguarda as threads finalizarem
            t_cap.join();
            t_save.join();
            t_pr.join();
        }
};


//Para compilar
//$ /usr/bin/g++ -g main.cpp -pthread -o main

int main( void )
{
    Pipeline mt;

    //Inicia pipeline
    mt.start();

    //Aguarda 1 segundos
    std::this_thread::sleep_for(std::chrono::milliseconds(1*1000));

    //Encerra pipeline
    mt.stop();

    return 0;
}