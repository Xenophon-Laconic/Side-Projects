//Just for fun implementation of Conway's Game of Life https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
//you will need the SDL2 library to run the visuals

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 1000   
#define HEIGHT 1000 
#define DIM 100 
#define CELL_SIZE (WIDTH / DIM)
#define UPDATE_INTERVAL 100 // time in milliseconds

void init_matrix_rand(int grid[DIM][DIM], int threshold);
int direct_nbrs(int init_grid[DIM][DIM], int coord[2], int state);
int darwin(int alive, int no_alive);
void t_evolution(int init_grid[DIM][DIM]);

void render_matrix(SDL_Renderer *renderer, int matrix[DIM][DIM]);
void run_simulation(int grid[DIM][DIM]);


int main(void) {
    srand(time(NULL));
    int grid[DIM][DIM] = {0};
    init_matrix_rand(grid, 30);

    run_simulation(grid);
    return EXIT_SUCCESS;
}
// compile command: gcc GameOfLife.c -o GameOfLife $(sdl2-config --cflags --libs) -lSDL2

void init_matrix_rand(int grid[DIM][DIM], int threshold) { //1

    for(size_t i = 0; i < DIM; i++){
        for(size_t j = 0; j < DIM; j++){
            if(rand()%100<threshold){
                grid[i][j] = 1;
            } else{
                grid[i][j] = 0;
            }
        }
    }
}

int direct_nbrs(int init_grid[DIM][DIM], int coord[2], int state){//5
    //checks the immediate neighbours of a given cell coord[i][j] for the characteristic state
    int i = coord[0];
    int j = coord[1];
    int no_nbrs = 0;

    int directions[8][2] = { //directions of all neighbours relative to cell
        {-1, 0}, 
        {1, 0}, 
        {0, -1}, 
        {0, 1}, 
        {-1, -1},
        {-1, 1}, 
        {1, -1}, 
        {1, 1}   
    };
    
    for (int k = 0; k < 8; k++) {
        int ni = i + directions[k][0];  // Neighbor row
        int nj = j + directions[k][1];  // Neighbor column

        // Check if the neighbor is within bounds and has characteristic state
        if (ni >= 0 && ni < DIM && nj >= 0 && nj < DIM) {
            if(init_grid[ni][nj] == state){
                no_nbrs ++;
            }
        }
    }
    return no_nbrs;
}

int darwin(int alive, int no_alive){
     //determines future state based on no_alive neighbours and whether the cell is alive or not (1/0)
    if(alive){
        switch (no_alive){
        case 2:
        case 3:
            alive = 1;
            break;
        default:
            alive = 0;
        }
    } else { // alive = 0
        if(no_alive == 3){
            alive = 1;
        }
    }

    return alive;
}

void t_evolution(int init_grid[DIM][DIM]){
    //performs the time evolution of the cellular system

    int temp_grid[DIM][DIM] = {0};
    int no_alive;

    //fill temp grid with next state
    for(size_t i = 0; i < DIM; i++){
        for(size_t j = 0; j < DIM; j++){
            no_alive = direct_nbrs(init_grid, (int[2]){i,j}, 1);
            if(init_grid[i][j]){
                temp_grid[i][j] = darwin(1,no_alive);
            } else {
                temp_grid[i][j] = darwin(0,no_alive);
            }
        }
    }

    //evolve init_grid
    for(size_t i = 0; i < DIM; i++){
        for(size_t j = 0; j < DIM; j++){
            init_grid[i][j] = temp_grid[i][j];
        }
    }

}

//greetings from Dr GPT
void render_matrix(SDL_Renderer *renderer, int matrix[DIM][DIM]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            if (matrix[i][j] == 1) {
                SDL_SetRenderDrawColor(renderer, 250, 230, 150, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 150, 250, 255);
            }

            SDL_Rect rect = {j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            SDL_RenderFillRect(renderer, &rect);

            // Grid lines
            //SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            //SDL_RenderDrawRect(renderer, &rect);
        }
    }
}

void run_simulation(int grid[DIM][DIM]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    // Create SDL window
    SDL_Window *window = SDL_CreateWindow("Conway's Game of Life",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    // Create SDL renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    int running = 1;
    SDL_Event event;
    int auto_update = 1;  // Start with automatic updates ON
    Uint32 last_update_time = SDL_GetTicks();  // Track time for updates

    while (running) {
        // Poll for events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                auto_update = !auto_update;  // Toggle auto-update mode
            }
        }

        // Automatic update logic
        if (auto_update && SDL_GetTicks() - last_update_time > UPDATE_INTERVAL) {
            t_evolution(grid);  // Update grid (or call your t_evolution() function)
            last_update_time = SDL_GetTicks();
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render grid
        render_matrix(renderer, grid);

        // Present the renderer
        SDL_RenderPresent(renderer);

        // Delay to control frame rate
        SDL_Delay(50);  // Small delay for smoother rendering
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
