#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>

#include "pipeline.h"
#include "bench_config.h"
#include "bench_metrics.h"
#include "profile_print.h"

static void print_usage(const char *prog)
{
    printf("Usage: %s --out RESULTS.csv --profile PROFILE.csv [--duration S] [--work-us US] [--warmup N] [--repeats R] [--seed S]\n", prog);
}

int main(int argc, char** argv)
{
    // Local bench configuration (no longer a global)
    BenchConfig benchConfig;

    // Simple manual parsing
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"--duration")==0 && i+1<argc){ benchConfig.duration_s = atoi(argv[++i]); }
        else if(strcmp(argv[i],"--work-us")==0 && i+1<argc){ benchConfig.work_us = atoi(argv[++i]); }
        else if(strcmp(argv[i],"--warmup")==0 && i+1<argc){ benchConfig.warmup = atoi(argv[++i]); }
        else if(strcmp(argv[i],"--repeats")==0 && i+1<argc){ benchConfig.repeats = atoi(argv[++i]); }
        else if(strcmp(argv[i],"--seed")==0 && i+1<argc){ benchConfig.seed = (unsigned int)atoi(argv[++i]); }
        else if(strcmp(argv[i],"--out")==0 && i+1<argc){ benchConfig.out_file = std::string(argv[++i]); }
        else if(strcmp(argv[i],"--profile")==0 && i+1<argc){ benchConfig.profile_file = std::string(argv[++i]); }
        else if(strcmp(argv[i],"--threads")==0 && i+1<argc){ benchConfig.threads = atoi(argv[++i]); }
        else if(strcmp(argv[i],"--help")==0){ print_usage(argv[0]); return 0; }
        else { printf("Unknown arg: %s\n", argv[i]); print_usage(argv[0]); return 1; }
    }

    if( benchConfig.seed != 0 ) srand(benchConfig.seed);

    // Require both output files: results CSV and profile events
    if( benchConfig.out_file.empty() || benchConfig.profile_file.empty() )
    {
        printf("Error: both --out and --profile must be specified.\n");
        print_usage(argv[0]);
        return 1;
    }

    // Prepare results CSV header if needed
    if( !benchConfig.out_file.empty() )
    {
        FILE *f = fopen(benchConfig.out_file.c_str(), "a+");
        if(f)
        {
            // move to start and check if file empty
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            if(size == 0)
            {
                fprintf(f, "threads,duration_s,work_us,run,processed,throughput_items_s\n");
            }
            fclose(f);
        }
    }

    // Open profile file via ProfilePrinter
    if (!ProfilePrinter::get().open_file(benchConfig.profile_file)) {
        printf("Error: cannot open profile file '%s' for writing\n", benchConfig.profile_file.c_str());
        return 1;
    }

    // Warmup runs
    for(int w=0; w<benchConfig.warmup; ++w)
    {
        reset_processed_items();
        Pipeline mt;
        mt.start();
        std::this_thread::sleep_for(std::chrono::seconds(benchConfig.duration_s));
        mt.stop();
        printf("warmup %d done\n", w+1);
    }

    // Repeat benchmark runs
    for(int r=1; r<=benchConfig.repeats; ++r)
    {
        reset_processed_items();
        Pipeline mt;
        mt.start();
        std::this_thread::sleep_for(std::chrono::seconds(benchConfig.duration_s));
        mt.stop();

        long long processed = get_processed_items();
        double throughput = 0.0;
        if(benchConfig.duration_s > 0) throughput = (double)processed / (double)benchConfig.duration_s;

        // Write to file if requested, else to stdout
        if( !benchConfig.out_file.empty() )
        {
            FILE *f = fopen(benchConfig.out_file.c_str(), "a");
            if(f)
            {
                fprintf(f, "%d,%d,%d,%d,%lld,%.2f\n", benchConfig.threads, benchConfig.duration_s, benchConfig.work_us, r, processed, throughput);
                fclose(f);
            }
            else
            {
                printf("error opening out file %s\n", benchConfig.out_file.c_str());
            }
        }
        else
        {
            printf("%d,%d,%d,%d,%lld,%.2f\n", benchConfig.threads, benchConfig.duration_s, benchConfig.work_us, r, processed, throughput);
        }
    }

    return 0;
}