#pragma once

#ifndef PARTICLES_H
#define PARTICLES_H

#include <stdlib.h>

// Structure definitions
typedef struct {
    int index;
    double position[2];
    double velocity[2];
    double acceleration[2];
    double forces[2];
    double radius;
    double max_x;
    double max_y;
    double collision_loss;
    double influence_radius;
    struct Particles* particles;
} Particle;

typedef struct {
    Particle* particles;
    int index;
    int num_particles;
    double max_x;
    double max_y;
    double forces[2];
    double radius;
    double collision_loss;
} Particles;

// Function prototypes
void init_particles(Particles* particles, int num_particles, double max_x, double max_y, double forces[2], double radius, double collision_loss, double influence_radius);
void add_particle(Particles* particles, double max_x, double max_y, double forces[2], double radius, double collision_loss, double influence_radius);
void update_particles(Particles* particles, double dt);
double calculate_density(Particles* particles, double p[2]);
double smoothing_kernel(double r, double dst);

#endif /* PARTICLES_H */
