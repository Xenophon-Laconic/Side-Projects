#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <flann/flann.h> //Fast Library for Approximate Nearest Neighbors https://chpatrick.github.io/flann/

#define L 200.0
#define N 1000

void init_xy(double x[N], double y[N]);
int nearest_neighbours(int idx, double x_new, double y_new, double x[N], double y[N]);
int mc_move(double d, double x[N], double y[N]);

// gcc -Wall -Wextra -Werror -Wpedantic -std=c18 bulk_sim.c -o bulk_sim -lflann -L/lib/x86_64-linux-gnu /lib/x86_64-linux-gnu/liblz4.so




int main(void){
    srand(time(NULL));
    double x[N] = {0};
    double y[N] = {0};
    init_xy(x,y);

    int Nacc = 0;
    int Ncurr = 0;
    int Neq = 100;
    int Nmc = 100000;
    double d = 0.05;

    //adjusting step size for acceptance ratio in [0.45,0.55]
    for(int i = 0; i < Neq; i++){
        Nacc += mc_move(d, x, y);
        Ncurr ++;
        if(i%10==0){
            if(Nacc / (double)Ncurr < 0.45){
                d *= 0.9;
                Nacc = 0;
                Ncurr = 0;
            }
            if(Nacc / (double)Ncurr > 0.55){
                d *= 1.1;
                Nacc = 0;
                Ncurr = 0;
            }
        }
    }
    printf("d is now:%lf\n", d);

    //run simulation with tuned step size
    FILE *file = fopen("positions.txt", "w");
    for(int i = 0; i < Nmc; i++){
        mc_move(d, x, y);

        if(i%N == 0){
            for (int j = 0; j < N; j++) {
                fprintf(file, "%d %lf %lf\n", i, x[j], y[j]);
            }
            fprintf(file, "\n");
        }

    }
    fclose(file);

    return EXIT_SUCCESS;
}

void init_xy(double x[N], double y[N]){
    
    for(size_t i = 0; i < N; i++){
        x[i] = L* rand()/(double)RAND_MAX;
        y[i] = L* rand()/(double)RAND_MAX;
    }

}

int nearest_neighbours(int idx, double x_new, double y_new, double x[N], double y[N]){
    // idx - index of previous location in x[N], y[N]
    // x/y_new - proposed new location
    // x/y[N] - locations

    float radius = 0.5;
    float query[2] = {(float)x_new, (float)y_new};

    float dataset[(N-1)*2];
    int dataset_idx = 0;

    // Populate dataset with x and y values, skipping idx
    for (int i = 0; i < N; i++) {
        if (i != idx) {
            dataset[dataset_idx] = (float)x[i];     // x value as float
            dataset[dataset_idx + 1] = (float)y[i]; // y value as float
            dataset_idx += 2;
        }
    }

    flann_index_t index;
    struct FLANNParameters params = DEFAULT_FLANN_PARAMETERS;
    params.algorithm = FLANN_INDEX_KDTREE;
    params.trees = 1;

    index = flann_build_index(dataset, N-1, 2, NULL, &params); //flan_index_t flann_build_index(float* dataset, int rows, int cols, float* speedup, struct FLANNParameters* flann_params);

    int indices[N-1];
    float dists[N-1];

    int found = flann_radius_search(index, query, indices, dists, N-1, radius * radius, &params);
    flann_free_index(index, &params);

    return found;
}

int mc_move(double d, double x[N], double y[N]){
    //returns 1 if move accepted, 0 if rejected

    int idx = rand() % N;

    //random direction
    double x_tilde = -1 + 2.0*rand()/(double)RAND_MAX;
    double y_tilde = -1 + 2.0*rand()/(double)RAND_MAX;
    double normalise = x_tilde*x_tilde + y_tilde*y_tilde;
    x_tilde /= normalise;
    y_tilde /= normalise;

    double x_new = x[idx] + d*x_tilde;
    double y_new = y[idx] + d*y_tilde;

    //pbc
    if(x_new < 0) x_new += L;
    if(x_new > L) x_new -= L;
    if(y_new < 0) y_new += L;
    if(y_new > L) y_new -= L;

    //overlap
    if(nearest_neighbours(idx, x_new, y_new, x, y)){
        return 0;
    } else { //update
        x[idx] = x_new;
        y[idx] = y_new;
        return 1;
        }

}
