#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "particles.h"
#include "particles.c"

#define WIN_WIDTH 2000
#define WIN_HEIGHT 1300
#define BALL_RADIUS 5
#define COLLISION_LOSS 0.7
#define GRAVITY_X 0
#define GRAVITY_Y 1
#define BALL_ACCELERATION 1
#define BALL_SPEED 1
#define INFLUENCE_RADIUS 200

// Function to handle events
void handle_events(Particles* particles, bool* running, bool* pause) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            *running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_r:
                init_particles(particles, 2, WIN_WIDTH, WIN_HEIGHT, (double[]) { GRAVITY_X, GRAVITY_Y }, BALL_RADIUS, COLLISION_LOSS, INFLUENCE_RADIUS);
                break;
            case SDLK_SPACE:
                *pause = !(*pause);
                break;
            default:
                break;
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
    init_particles(&particles, 500, WIN_WIDTH, WIN_HEIGHT, (double[]) { GRAVITY_X, GRAVITY_Y }, BALL_RADIUS, COLLISION_LOSS, INFLUENCE_RADIUS);

    // Find maximum density
    double max_density = 0;
    double min_density = 100;
    
    // create a 2D array to store densities with allocated memory
    double** densities = (double**)malloc(WIN_WIDTH * sizeof(double*));
    for (int i = 0; i < WIN_WIDTH; ++i) {
        densities[i] = (double*)malloc(WIN_HEIGHT * sizeof(double));
    }
    
    for (int x = 0; x < WIN_WIDTH; ++x) {
        printf("x: %d\n", x);
            for (int y = 0; y < WIN_HEIGHT; ++y) {
                double p[2] = {x, y};
                double density = calculate_density(&particles, p);
                densities[x][y] = density;
                if (density > max_density) {
                    max_density = density;
                }
            }
        }

        printf("max_density: %f\n", max_density);
        printf("min_density: %f\n", min_density);


    // Game loop control
    bool running = true;
    bool pause = true;

    // Main game loop
    while (running) {
        // Handle events
        handle_events(&particles, &running, &pause);

        // Fill the background color
        SDL_SetRenderDrawColor(renderer, 38, 44, 77, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        
        for (int x = 0; x < WIN_WIDTH; ++x) {
            for (int y = 0; y < WIN_HEIGHT; ++y) {
                int color = (int)(255 * densities[x][y] / max_density);
                SDL_SetRenderDrawColor(renderer, color, color, color, SDL_ALPHA_OPAQUE);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        // Draw particles
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


        // Update particles if not paused
        if (!pause) {
            update_particles(&particles, 1);
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
