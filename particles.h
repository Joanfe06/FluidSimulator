#pragma once

#ifndef PARTICLES_
#define PARTICLES_H

#include <stdlib.h>
#include <SDL2/SDL.h>
#include <omp.h>
#include <limits.h>

#define WIN_WIDTH 2000
#define WIN_HEIGHT 1300
#define BALL_RADIUS 5
#define COLLISION_LOSS 0.95
#define GRAVITY_X 0
#define GRAVITY_Y 0
#define BALL_ACCELERATION 1
#define BALL_SPEED 1
#define INFLUENCE_RADIUS 100
#define TARGET_DENSITY 0.0275
#define P_MULT 0.5
#define NUM_PARTICLES 4000

// Structure definitions

typedef struct {
    int idx;
    uint cell_key;
} Entry;

typedef struct {
    int index;
    double position[2];
    double velocity[2];
    double acceleration[2];
    double density;
    int prev_cell[2];
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
    Entry* spatial_lookup;
    int* start_indices;
} Particles;


// Function prototypes
void init_particles(Particles* particles, int num_particles, double max_x, double max_y, double forces[2], double radius, double collision_loss, double influence_radius);
void add_particle(Particles* particles, double max_x, double max_y);
void update_particles(Particles* particles, double dt, int frames);
double calculate_density(Particles* particles, double p[2]);
double smoothing_kernel(double r, double dst);
double smoothing_kernel_gradient(double dst, double r);
double convert_density_to_pressure(double density);
double calculate_all_densities(Particles* particles, double** densities);
void draw_densities(SDL_Renderer* renderer, double** densities, double max_density);
void calculateColor(double pressure, double minmax[2], int *red, int *green, int *blue);
void calculate_all_pressures(Particles* particles, double** pressures, double minmax[2]);
void calculate_pressure_force(Particles* particles, int idx, double pressure_force[2]);
double calculate_shared_pressure(double d_a, double d_b);
void handle_wall_collisions(Particles* particles, Particle* p);
void getRandomDir(double dir[2]);
void update_spatial_lookup(Particles* particles);
int getMax(Particles* particles, int n);
void countSort(Particles* particles, int n, int exp);
void radixsort(Particles* particles);
void position_to_cell_coord(double position[2], int cell[2], double influence_radius);
uint hash_cell(int x, int y);
uint get_key_from_hash(uint hash, int n);
void for_each_point_within_radius(Particles* particles, double sample_point[2], double pressure_force[2], int idx);
bool check_sorted(Particles* particles);

#endif /* PARTICLES_H */
