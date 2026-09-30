#include "lib/parse_log.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "pthread.h"



char *read_file(char *path){
        FILE *file = fopen(path, "r");
        
        if(file == NULL){
            fprintf(stderr, "Could not open file %s", path);
            return NULL;
        }

        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        rewind(file);

        char *buffer = (char *)malloc(file_size);

        fread(buffer, 1, file_size, file);

        return buffer;
}

LogStats *gen_stats(char *buffer){
        char *line_start = buffer;
        LogEntry log;
        LogStats *stats = (LogStats*)malloc(sizeof(LogStats));

        const long long zero_val = 1;
        stats -> urls = hashmap_init(20, hash, comparable_str, print_str, print_long_long);

        while(line_start != NULL && *line_start != '\0'){
            char *next_line = strchr(line_start, '\n');

            //printf("%.*s ", (int)(next_line - line_start), line_start); // print the whole line log
            *next_line = '\0';
            parse_log_line(line_start, &log);
            //printf("%lld ", log.bytes);

            if(log.status >= 400 && log.status <= 599)
                stats -> errors++;

            switch(log.status){
                case 404: stats -> total_404++;   break;
                case 200: stats -> total_200++; break;
            }
            stats -> total_requests++;
            stats -> total_bytes += log.bytes;

            void *was_accessed = stats -> urls -> search(stats -> urls, (void *)log.url);

            if(was_accessed == NULL)
                stats -> urls -> insert(stats -> urls, (void *)log.url, (void*)&zero_val, strlen(log.url), sizeof(zero_val));
            else {
                (*(long long*)was_accessed)++;
                //printf("%s -> %lld\n\n", log.url, *(long long*)was_accessed);
            }

           // printf("%s", log.timestamp);

            *next_line = '\n';

            line_start = next_line + 1;
        }

        return stats;
}

LogStats *worker(char *path, double* task){
        clock_t begin = clock();
            char *buffer = read_file(path);
            if(buffer == NULL)
                return NULL;

            LogStats *stats = gen_stats(buffer);
        clock_t end = clock();

        *task = (double)(begin - end) / CLOCKS_PER_SEC; 
        //stats -> urls -> print(stats -> urls);
        //stats -> urls -> lambda(stats -> urls);

        free(buffer);

        return stats;
}

int main(int argc, char *argv[]){

    printf("============================================================\nANALISADOR DE LOGS - RELATÓRIO COMPLETO\n============================================================\n\n");

    LogStats *global_stats[argc - 1]; // array of pointers to the struct 

    for(int i = 0; i < argc - 1; i++){
        printf("ARQUIVO: %s\nTHREADS: %d\n", argv[i + 1], i);

        double task;
        global_stats[i] = worker(argv[i + 1], &task); // argv starts at 1

        printf("TEMPO DE EXECUÇÃO: %fs\n", task);
        //
        //free(stats);


        printf("%lld ", global_stats[i] -> total_200);

        //
    }

    LogStats final_stats = {};
    for(int i = 0; i < argc - 1; i++){
        double rate_200 = (double)global_stats[i] -> total_200 / global_stats[i] -> total_requests;
        double rate_404 = (double)global_stats[i] -> total_404 / global_stats[i] -> total_requests;
        global_stats[i] -> avg_bytes = (double)global_stats[i] -> total_bytes / global_stats[i] -> total_requests;
        global_stats[i] -> error_rate = (double) global_stats[i] -> errors / global_stats[i] -> total_requests;

        printf("------------------------------------------------------------\nESTATÍSTICAS BÁSICAS\n------------------------------------------------------------\n\n");
        printf("Total de Requisições:\t%lld\nRequisições 200 (OK):\t%lld (%.2f%%)\nRequisições 404 (Not Found):\t%lld (%.2f%%)\nTotal de Bytes:\t%lld\nMédia de Bytes/Req:\t%f bytes\nTaxa de Erro Geral:\t%f%%\n", global_stats[i] -> total_requests, global_stats[i] -> total_200, rate_200 * 100, global_stats[i] -> total_404, rate_404 * 100, global_stats[i] -> total_bytes, global_stats[i] -> avg_bytes, global_stats[i] -> error_rate * 100);

        final_stats.total_requests += global_stats[i] -> total_requests;
        final_stats.total_404 += global_stats[i] -> total_404;
        final_stats.total_200 += global_stats[i] -> total_200;
        final_stats.total_bytes += global_stats[i] -> total_bytes;
        final_stats.errors += global_stats[i] -> errors;



        // hardcoded 
        for(int j = 0; j < 24; j++)
            final_stats.requests_per_hour[j] += global_stats[i] -> requests_per_hour[j];
        

        // Free sequence
        free_hashmap(global_stats[i] -> urls); /* Always free hashmap befor the buffer*/
        free(global_stats[i]);
    }

    double rate_200 = (double)final_stats.total_200 / final_stats.total_requests;
    double rate_404 = (double)final_stats.total_404 / final_stats.total_requests;
    final_stats.avg_bytes = (double)final_stats.total_bytes / final_stats.total_requests;
    final_stats.error_rate = (double)final_stats.errors / final_stats.total_requests;

    printf("------------------------------------------------------------\nESTATÍSTICAS BÁSICAS FINAIS \n------------------------------------------------------------\n\n");
    printf("Total de Requisições:\t%lld\nRequisições 200 (OK):\t%lld (%.2f%%)\nRequisições 404 (Not Found):\t%lld (%.2f%%)\nTotal de Bytes:\t%lld\nMédia de Bytes/Req:\t%f bytes\nTaxa de Erro Geral:\t%f%%\n", final_stats.total_requests, final_stats.total_200, rate_200 * 100, final_stats.total_404, rate_404 * 100, final_stats.total_bytes, final_stats.avg_bytes, final_stats.error_rate * 100);

 
    return 0;
}