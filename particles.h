#pragma once

#ifndef PARTICLES_
#define PARTICLES_H

#include <stdlib.h>
#include <SDL2/SDL.h>

#define WIN_WIDTH 2000
#define WIN_HEIGHT 1300
#define BALL_RADIUS 5
#define COLLISION_LOSS 0.95
#define GRAVITY_X 0
#define GRAVITY_Y 0
#define BALL_ACCELERATION 1
#define BALL_SPEED 1
#define INFLUENCE_RADIUS 100
#define TARGET_DENSITY 0.003
#define P_MULT 1
#define NUM_PARTICLES 250

// Structure definitions
typedef struct {
    int index;
    double position[2];
    double velocity[2];
    double acceleration[2];
    double density;
} Particle;

typedef struct {
    Particle* particles;
    int index;
    int num_particles;
    double max_x;
    double max_y;
    double forces[2];
    double radius;
    double influence_radius;
    double collision_loss;
} Particles;

// Function prototypes
void init_particles(Particles* particles, int num_particles, double max_x, double max_y, double forces[2], double radius, double collision_loss, double influence_radius);
void add_particle(Particles* particles, double max_x, double max_y);
void update_particles(Particles* particles, double dt);
double calculate_density(Particles* particles, double p[2]);
double smoothing_kernel(double r, double dst);
double smoothing_kernel_gradient(double dst, double r);
double convert_density_to_pressure(double density);
double calculate_all_densities(Particles* particles, double** densities);
void draw_densities(SDL_Renderer* renderer, double** densities, double max_density);
void calculateColor(double pressure, double max_pressure, int *red, int *green, int *blue);
double calculate_all_pressures(Particles* particles, double** pressures);
void calculate_pressure_force(Particles* particles, int idx, double pressure_force[2]);
double calculate_shared_pressure(double d_a, double d_b);
void handle_wall_collisions(Particles* particles, Particle* p);
void zero_out_small_velocities(Particle* p);
void getRandomDir(double dir[2]);

#endif /* PARTICLES_H */
