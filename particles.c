#include "particles.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

// Function to initialize particles
void init_particles(Particles* particles, int num_particles, double max_x, double max_y, double forces[2], double radius, double collision_loss, double influence_radius) {
    // Allocate memory for particles
    particles->particles = (Particle*)malloc(num_particles * sizeof(Particle));
    if (particles->particles == NULL) {
        fprintf(stderr, "Memory allocation failed for particles.\n");
        exit(EXIT_FAILURE); // Or handle the error appropriately
    }
    
    // Initialize particle properties
    particles->index = 0;
    particles->num_particles = num_particles;
    particles->max_x = max_x;
    particles->max_y = max_y;
    particles->forces[0] = forces[0];
    particles->forces[1] = forces[1];
    particles->radius = radius;
    particles->collision_loss = collision_loss;

    // Initialize each particle
    for (int i = 0; i < num_particles; ++i) {
        particles->particles[i].index = i;
        particles->particles[i].position[0] = (double)rand() / RAND_MAX * max_x;
        particles->particles[i].position[1] = (double)rand() / RAND_MAX * max_y;
        particles->particles[i].velocity[0] = 0;
        particles->particles[i].velocity[1] = 0;
        particles->particles[i].acceleration[0] = 0;
        particles->particles[i].acceleration[1] = 0;
        particles->particles[i].forces[0] = forces[0];
        particles->particles[i].forces[1] = forces[1];
        particles->particles[i].radius = radius;
        particles->particles[i].influence_radius = influence_radius;
        particles->particles[i].max_x = max_x;
        particles->particles[i].max_y = max_y;
        particles->particles[i].collision_loss = collision_loss;
        particles->particles[i].particles = particles; // Set pointer to parent Particles structure
    }
}

// Function to add a particle
void add_particle(Particles* particles, double max_x, double max_y, double forces[2], double radius, double collision_loss, double influence_radius) {
    // Reallocate memory for particles to add one more
    Particle* new_particles = (Particle*)realloc(particles->particles, (particles->num_particles + 1) * sizeof(Particle));
    if (new_particles == NULL) {
        fprintf(stderr, "Memory reallocation failed for adding a particle.\n");
        // Optionally handle the error or exit the program
        return;
    }
    
    // Update particles pointer to the new memory block
    particles->particles = new_particles;
    
    // Initialize the new particle
    particles->particles[particles->index].index = particles->index;
    particles->particles[particles->index].position[0] = (double)rand() / RAND_MAX * max_x;
    particles->particles[particles->index].position[1] = (double)rand() / RAND_MAX * max_y;
    particles->particles[particles->index].velocity[0] = 0;
    particles->particles[particles->index].velocity[1] = 0;
    particles->particles[particles->index].acceleration[0] = 0;
    particles->particles[particles->index].acceleration[1] = 0;
    particles->particles[particles->index].forces[0] = forces[0];
    particles->particles[particles->index].forces[1] = forces[1];
    particles->particles[particles->index].radius = radius;
    particles->particles[particles->index].max_x = max_x;
    particles->particles[particles->index].max_y = max_y;
    particles->particles[particles->index].collision_loss = collision_loss;
    particles->particles[particles->index].influence_radius = influence_radius;
    particles->particles[particles->index].particles = particles; // Set pointer to parent Particles structure
    
    // Update particle count
    particles->index++;
    particles->num_particles++;
}

// Function to update particles
void update_particles(Particles* particles, double dt) {
    for (int i = 0; i < particles->num_particles; ++i) {
        // Update position based on velocity
        particles->particles[i].position[0] += particles->particles[i].velocity[0] * dt;
        particles->particles[i].position[1] += particles->particles[i].velocity[1] * dt;
        
        // Apply external forces as acceleration
        particles->particles[i].acceleration[0] = particles->particles[i].forces[0];
        particles->particles[i].acceleration[1] = particles->particles[i].forces[1];

        // Handle wall collisions and apply collision loss
        if (particles->particles[i].position[0] <= particles->particles[i].radius) {
            particles->particles[i].position[0] = particles->particles[i].radius;
            particles->particles[i].velocity[0] *= -particles->particles[i].collision_loss; // Apply collision loss
        }
        if (particles->particles[i].position[0] >= particles->particles[i].max_x - particles->particles[i].radius) {
            particles->particles[i].position[0] = particles->particles[i].max_x - particles->particles[i].radius;
            particles->particles[i].velocity[0] *= -particles->particles[i].collision_loss; // Apply collision loss
        }
        if (particles->particles[i].position[1] <= particles->particles[i].radius) {
            particles->particles[i].position[1] = particles->particles[i].radius;
            particles->particles[i].velocity[1] *= -particles->particles[i].collision_loss; // Apply collision loss
        }
        if (particles->particles[i].position[1] >= particles->particles[i].max_y - particles->particles[i].radius) {
            particles->particles[i].position[1] = particles->particles[i].max_y - particles->particles[i].radius;
            particles->particles[i].velocity[1] *= -particles->particles[i].collision_loss; // Apply collision loss
        }

        // Apply acceleration to velocity
        particles->particles[i].velocity[0] += particles->particles[i].acceleration[0] * dt;
        particles->particles[i].velocity[1] += particles->particles[i].acceleration[1] * dt;

        // Zero out very small velocities to avoid numerical errors
        if (fabs(particles->particles[i].velocity[0]) < 1)
            particles->particles[i].velocity[0] = 0;
        if (fabs(particles->particles[i].velocity[1]) < 1)
            particles->particles[i].velocity[1] = 0;
    }
}

// Function to calculate density
double calculate_density(Particles* particles, double p[2]) {
    double mass = 1;
    double density = 0;

    for (int i = 0; i < particles->num_particles; ++i) {
        double dist = sqrt(pow(particles->particles[i].position[0] - p[0], 2) + pow(particles->particles[i].position[1] - p[1], 2));
        double influence = smoothing_kernel(particles->particles[i].influence_radius, dist);
        density += mass * influence;
    }

    return density;
}

double smoothing_kernel(double r, double dst) {
    double volume = M_PI * pow(r, 8) / 4;
    double v = fmax(0, pow(r,2) - pow(dst,2));
    return pow(v,3) / volume;
}

