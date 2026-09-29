#include "lib/parse_log.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "lib/hashmap.h"
#include "lib/hashmap_func.h"


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

        const long long zero_val = 0;
        Hashmap *hashmap = hashmap_init(20, hash, comparable_str, print_str, print_str);

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

            void *was_accessed = hashmap -> search(hashmap, (void *)log.url);

            if(was_accessed == NULL)
                hashmap -> insert(hashmap, (void *)log.url, (void*)&zero_val, strlen(log.url), sizeof(zero_val));
            else {
                (*(long long*)was_accessed)++;
                printf("%s -> %lld\n\n", log.url, *(long long*)was_accessed);
            }


            *next_line = '\n';

            line_start = next_line + 1;
        }

        return stats;
}

int main(int argc, char *argv[]){

    printf("============================================================\nANALISADOR DE LOGS - RELATÓRIO COMPLETO\n============================================================\n\n");

    for(int i = 1; i < argc; i++){

        printf("ARQUIVO: %s\nTHREADS: %d\n", argv[i], 1);

        clock_t begin = clock();
            char *buffer = read_file(argv[i]);
            LogStats *stats = gen_stats(buffer);
        clock_t end = clock();

        double task = (double)(begin - end) / CLOCKS_PER_SEC;
        printf("TEMPO DE EXECUÇÃO: %fs\n", task);

        double rate_200 = (double)stats -> total_200 / stats -> total_requests;
        double rate_404 = (double)stats -> total_404 / stats -> total_requests;
        stats -> avg_bytes = (double)stats -> total_bytes / stats -> total_requests;
        stats -> error_rate = (double) stats -> errors / stats -> total_requests;

        printf("------------------------------------------------------------\nESTATÍSTICAS BÁSICAS\n------------------------------------------------------------\n\n");
        printf("Total de Requisições:\t%lld\nRequisições 200 (OK):\t%lld (%.2f%%)\nRequisições 404 (Not Found):\t%lld (%.2f%%)\nTotal de Bytes:\t%lld\nMédia de Bytes/Req:\t%f bytes\nTaxa de Erro Geral:\t%f%%\n", stats -> total_requests, stats -> total_200, rate_200 * 100, stats -> total_404, rate_404 * 100, stats -> total_bytes, stats -> avg_bytes, stats -> error_rate * 100);

        free(buffer);
        free(stats);
    }

    return 0;
}