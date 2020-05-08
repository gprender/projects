/**
 * concurrent_lad_regression.c
 * A concurrent least absolute deviation regression calculator.
 * I didn't come up with this concurrent map scheme, but I did
 * make some modifications to it.
 *
 * Written by Graeme Prendergast, Spring 2020
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>


#define DATAFILE "data.csv"
#define N 365

double data[N];


struct map_arg {
    void** functor;
    void** results;
    void* (*f)(void*);
    int from;
    int to;
};


void init_map_arg(struct map_arg* arg, void** functor, void** results,
                    void* (*f)(void*), int from, int to) {
    arg->functor = functor;
    arg->results = results;
    arg->f = f;
    arg->from = from;
    arg->to = to;
}


void* chunk_map(void* a) {
    struct map_arg* arg = (struct map_arg*)a;

    for (int i = arg->from; i < arg->to; i++) {
        arg->results[i] = (*(arg->f))(&(arg->functor[2*i]));
    }

    return NULL;
}


void** concurrent_map(void** functor, void* (*f)(void*), int length, int nthreads) {

    void** results = malloc(sizeof(void*) * length);
    struct map_arg args[nthreads];
    pthread_t threads[nthreads];
    int chunk_size = (length / nthreads);

    for (int i=0; i<nthreads; i++) {

        int from = i * chunk_size;

        // Special case for when (length % nthreads != 0)
        int to;
        if (i == nthreads - 1) { to = length; }
        else { to = from + chunk_size; }

        struct map_arg* arg = &args[i];
        init_map_arg(arg, functor, results, f, from, to);
        pthread_create(&threads[i], NULL, chunk_map, (void*)arg);
    }

    for (int i=0; i<nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return results;
}


void* get_sum_abs_devs(void* e) {
    
    double* elem = *(double**)(&e);

    /**
     * Similarly to get_lines(), these two reads are considered invalid.
     * Again though, elem is a pair of doubles here, and I'm just
     * reading each index of the pair. 
     */
    double m = elem[0];
    double b = elem[1];

    // Need to malloc so we can return a local pointer
    double* sad = malloc(sizeof(double));
    *sad = 0;
    for (int i=0; i<N; i++) {
        double y_line = (m * (i+1)) + b;
        double abs_dev = fabs(data[i] - y_line);
        *sad += abs_dev;
    }

    return (void*)sad;
}


// Get the index of the smallest element of an iterable.
int mindex(double** vals, int length) {

    double min_so_far = *vals[0];
    int min_index = 0;

    for (int i=1; i<length; i++) {
        if (!min_so_far) {
            min_so_far = *vals[i];
            min_index = i;
        }
        else {
            if (*vals[i] < min_so_far) {
                min_so_far = *vals[i];
                min_index = i;
            }
        }
    }

    return min_index;
}


// Split a string into an array of strings by the given delimiter. Like Python!
char** str_split(char* str, char* delim, int buffer_size) {

    char** tokens = malloc(sizeof(char*) * buffer_size);
    char* t = NULL;
    int position = 0;

    if (!tokens || !str) { return NULL; }

    t = strtok(str, delim);
    while (t != NULL) {
        tokens[position] = t;
        position++;

        if (position >= buffer_size) { return NULL; }

        t = strtok(NULL, delim);
    }
    tokens[position] = NULL;

    return tokens;
}


// Solve (y = mx + b) for each combination of points in the data array.
void get_lines(double (*dest)[2]) {

    int count = 0;
    for (int i=0; i<N; i++) {
        for (int j=i+1; j<N; j++){
            double x1 = i+1;
            double x2 = j+1;
            double y1 = data[i];
            double y2 = data[j];

            double m = (y2 - y1) / (x2 - x1);
            double b = y1 - (m * x1);


            /**
             * According to valgrind, these 2 lines are "invalid writes",
             * but I don't see how they could be. I'm writing doubles to
             * a 2d array of doubles, which is totally kosher.
             */
            dest[count][0] = m;
            dest[count][1] = b;

            count++;
        }
    }

}


/**
 * Fit a least absolute deviations (l1 norm) line to the given dataset.
 * In this case, we're assuming the data is a sequence of floats describing
 * the dependent variables, and that the independant variables are the range
 * of integers from 1 to n, where n is the number of entries in the dataset.
 */
void simple_lad_fit(char* filename) {

    FILE* infile = fopen(filename, "r");
    if (!infile) {
        printf("Error opening the data file: \"%s\"\n", filename);
        return;
    }

    for (int i=0; i<N; i++) {
        fscanf(infile, "%lf", &data[i]);
    }

    int nc2 = N * (N-1) / 2; // N choose 2
    
    double lines[nc2][2];

    get_lines(lines);

    void** results = concurrent_map((void**)(&lines), get_sum_abs_devs, nc2, 1);

    double** sum_abs_devs = (double**)results;

    int min_index = mindex(sum_abs_devs, nc2);

    printf("The line of least absolute deviations is: y = %lfx + %lf\n", 
            lines[min_index][0], lines[min_index][1]);
    printf("With a sum of absolute deviations of %lf\n.", 
            *sum_abs_devs[min_index]);

}


void time_series_lad_fit(char* filename) {

    FILE* infile = fopen(filename, "r");
    if (!infile) {
        printf("Error opening the data file: \"%s\"\n", filename);
        return;
    }

    char** line_tokens = NULL;
    char* buffer = NULL;
    size_t len = 0;

    getline(&buffer, &len, infile); // Throw out the header line
    int num_lines = 0; // Start at 1, since we turfed the header

    while((getline(&buffer, &len, infile) != -1) && (num_lines < N)) {

        line_tokens = str_split(buffer, ",", 4);

        data[num_lines] = strtod(line_tokens[1], NULL);

        num_lines++;

        free(line_tokens);
    }
    free(buffer);

    int nc2 = N * (N-1) / 2; // N choose 2
    
    double lines[nc2][2];

    get_lines(lines);

    void** results = concurrent_map((void**)(&lines), get_sum_abs_devs, nc2, 4);

    double** sum_abs_devs = (double**)results;

    int min_index = mindex(sum_abs_devs, nc2);

    printf("The line of least absolute deviations is: y = %lfx + %lf\n", 
            lines[min_index][0], lines[min_index][1]);
    printf("With a sum of absolute deviations of %lf\n.", 
            *sum_abs_devs[min_index]);

    // Free the stuff
    for (int i=0; i<nc2; i++) {
        free(sum_abs_devs[i]);
    }
    free(sum_abs_devs);
}

// Get time in milliseconds - from the worm part's util.c
size_t time_ms() {
    struct timeval tv;
    if(gettimeofday(&tv, NULL) == -1) {
        perror("gettimeofday");
        exit(2);
    }
    
    // Convert timeval values to milliseconds
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}


int main() {

    size_t start_time = time_ms();

    printf("\nCalculating LAD fit for %s...\n", DATAFILE);
    time_series_lad_fit(DATAFILE);

    size_t end_time = time_ms();

    printf("\n\nRuntime was %lu milliseconds.\n", (end_time - start_time));

    return 1;
}