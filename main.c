#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>
#include <SDL2/SDL.h>
#include "particles.h"
#include "particles.c"
#include "aux_functions.c"

bool x = false;
int frames = 0;

// Function to handle events
void handle_events(Particles* particles, bool* running, bool* pause, bool* draw_density, bool* draw_pressure, bool* draw_radius, double** densities, double** pressures, SDL_Renderer* renderer) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            *running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_r:
                init_particles(particles, NUM_PARTICLES, WIN_WIDTH, WIN_HEIGHT, (double[]) { GRAVITY_X, GRAVITY_Y }, BALL_RADIUS, COLLISION_LOSS, INFLUENCE_RADIUS);
                break;
            case SDLK_SPACE:
                *pause = !(*pause);
                frames = 0;
                break;
            case SDLK_d:
                if ((*pause)) {
                    if(*draw_density == true) {
                        *draw_density = false;
                    }
                    else {
                        *draw_density = true;
                        double max_density = calculate_all_densities(particles, densities);
                        draw_densities(renderer, densities, max_density);
                    }
                }
                break;
            case SDLK_p:
                if ((*pause)) {
                    if(*draw_pressure == true) {
                        *draw_pressure = false;
                    }
                    else {
                        *draw_pressure = true;
                        double minmax[2];
                        calculate_all_pressures(particles, pressures, minmax);
                        printf("Min Pressure: %lf\n", minmax[0]);
                        printf("Max Pressure: %lf\n", minmax[1]);
                        draw_pressures(renderer, pressures, minmax);
                    }
                }
                break;
            default:
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                int x,y;
                SDL_GetMouseState(&x, &y);
                double p[2];
                p[0] = x / 1.0;
                p[1] = y / 1.0;
                printf("Density at (%f, %f): %lf\n", p[0], p[1], calculate_density(particles, p));
                }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                int x,y;
                SDL_GetMouseState(&x, &y);
                double p[2];
                p[0] = x / 1.0;
                p[1] = y / 1.0;
                if(*draw_radius == true) {
                        *draw_radius = false;
                        x = true;
                    }
                    else if (x){
                        *draw_radius = true;
                        x = false;
                        paint_each_point_within_radius(renderer, particles, p);
                    }
                }
            break;
        default:
            break;
        }
    }
}


int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Create a window
    SDL_Window* win = SDL_CreateWindow("Bouncing Ball", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!win) {
        printf("SDL window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    

    // Initialize particles
    Particles particles;
    init_particles(&particles, NUM_PARTICLES, WIN_WIDTH, WIN_HEIGHT, (double[]) { GRAVITY_X, GRAVITY_Y }, BALL_RADIUS, COLLISION_LOSS, INFLUENCE_RADIUS);

    
    // create a 2D array to store densities with allocated memory
    double** densities = (double**)malloc(WIN_WIDTH * sizeof(double*));
    double** pressures = (double**)malloc(WIN_WIDTH * sizeof(double*));
    for (int i = 0; i < WIN_WIDTH; ++i) {
        densities[i] = (double*)malloc(WIN_HEIGHT * sizeof(double));
        pressures[i] = (double*)malloc(WIN_HEIGHT * sizeof(double));
    }

    double frame_start_time = SDL_GetTicks();


    // Game loop control
    bool running = true;
    bool pause = true;
    bool draw_density = false;
    bool draw_pressure = false;
    bool draw_radius = false;

    // Main game loop
    while (running) {
        // Handle events
        handle_events(&particles, &running, &pause, &draw_density, &draw_pressure, &draw_radius, densities, pressures, renderer);

        // Fill the background color
        if(!draw_density && !draw_pressure && !draw_radius) {
            SDL_SetRenderDrawColor(renderer, 38, 44, 77, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);
        }

        // Draw particles
        if(!draw_radius){
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
            for (int i = 0; i < particles.num_particles; ++i) {
                int centerX = (int)particles.particles[i].position[0];
                int centerY = (int)particles.particles[i].position[1];
                int radius = BALL_RADIUS;

                for (int y = -radius; y <= radius; ++y) {
                    for (int x = -radius; x <= radius; ++x) {
                        if (x*x + y*y <= radius*radius) {
                            SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                        }
                    }
                }
            }
        }



        // Update particles if not paused
        if (!pause) {
            update_particles(&particles, 1, frames);
        }

        frames++;
        if (SDL_GetTicks() - frame_start_time >= 5000) {
            float elapsed_seconds = (SDL_GetTicks() - frame_start_time) / 1000.0;
            float fps = frames / elapsed_seconds;
            printf("FPS: %.2f\n", fps);
            frame_start_time = SDL_GetTicks();
            frames = 0;
        }

        // Delay and update display
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    // Cleanup and quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
