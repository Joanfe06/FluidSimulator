#include "particles.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define _USE_MATH_DEFINES

void init_particles(Particles* particles, int num_particles, double max_x, double max_y, double forces[2], double radius, double collision_loss, double influence_radius) {
    particles->particles = malloc(num_particles * sizeof(Particle));
    if (particles->particles == NULL) {
        perror("Memory allocation failed for particles.");
        exit(EXIT_FAILURE);
    }
    
    particles->index = 0;
    particles->num_particles = num_particles;
    particles->max_x = max_x;
    particles->max_y = max_y;
    particles->forces[0] = forces[0];
    particles->forces[1] = forces[1];
    particles->radius = radius;
    particles->influence_radius = influence_radius;
    particles->collision_loss = collision_loss;

    for (int i = 0; i < num_particles; i++) {
        Particle* p = &particles->particles[i];
        p->index = i;
        p->position[0] = (double)rand() / RAND_MAX * max_x;
        p->position[1] = (double)rand() / RAND_MAX * max_y;
        p->velocity[0] = p->velocity[1] = 0;
        p->acceleration[0] = p->acceleration[1] = 0;
        p->density = 0;
    }
}

void add_particle(Particles* particles, double max_x, double max_y) {
    Particle* new_particles = realloc(particles->particles, (particles->num_particles + 1) * sizeof(Particle));
    if (new_particles == NULL) {
        perror("Memory reallocation failed for adding a particle.");
        return;
    }
    
    particles->particles = new_particles;
    
    Particle* p = &particles->particles[particles->index];
    p->index = particles->index;
    p->position[0] = (double)rand() / RAND_MAX * max_x;
    p->position[1] = (double)rand() / RAND_MAX * max_y;
    p->velocity[0] = p->velocity[1] = 0;
    p->acceleration[0] = p->acceleration[1] = 0;
    p->density = 0;
    
    particles->index++;
    particles->num_particles++;
}

void update_particles(Particles* particles, double dt) {
    for (int i = 0; i < particles->num_particles; i++){
        Particle* p = &particles->particles[i];
        p->density = calculate_density(particles, p->position);
    }
    for (int i = 0; i < particles->num_particles; i++) {
        Particle* p = &particles->particles[i];
        p->velocity[0] += particles->forces[0] * dt;
        p->velocity[1] += particles->forces[1] * dt;

        double pressure_force[2];
        calculate_pressure_force(particles, i, pressure_force);
        p->acceleration[0] = pressure_force[0] / p->density;
        p->acceleration[1] = pressure_force[1] / p->density;

        p->velocity[0] += p->acceleration[0] * dt;
        p->velocity[1] += p->acceleration[1] * dt;

        p->position[0] += p->velocity[0] * dt;
        p->position[1] += p->velocity[1] * dt;

        handle_wall_collisions(particles, p);
    }
}

double calculate_density(Particles* particles, double p[2]) {
    double mass = 1;
    double density = 0;

    for (int i = 0; i < particles->num_particles; i++) {
        Particle* current = &particles->particles[i];
        double dist = hypot(current->position[0] - p[0], current->position[1] - p[1]);
        double influence = smoothing_kernel(particles->influence_radius, dist);
        density += mass * influence;
    }

    return density;
}

void calculate_pressure_force(Particles* particles, int idx, double pressure_force[2]) {
    pressure_force[0] = pressure_force[1] = 0;
    double mass = 1;
    double p[2] = {particles->particles[idx].position[0], particles->particles[idx].position[1]};
    double dir[2];

    for (int i = 0; i < particles->num_particles; ++i) {
        if(i == idx) continue;
        Particle* current = &particles->particles[i];

        double offset[2] = {current->position[0] - p[0], current->position[1] - p[1]};
        double dst = hypot(offset[0], offset[1]);
        if (dst == 0){
            getRandomDir(dir);
        } else {
            dir[0] = offset[0] / dst;
            dir[1] = offset[1] / dst;
        }
        double slope = smoothing_kernel_gradient(dst, particles->influence_radius);
        double density = current->density;
        double shared_pressure = calculate_shared_pressure(current->density, particles->particles[idx].density);

        pressure_force[0] += -dir[0] * slope * shared_pressure * mass / density;
        pressure_force[1] += -dir[1] * slope * shared_pressure * mass / density;
    }
}

void handle_wall_collisions(Particles* particles, Particle* p) {
    if (p->position[0] <= particles->radius) {
        p->position[0] = particles->radius;
        p->velocity[0] *= -particles->collision_loss;
    }
    if (p->position[0] >= particles->max_x - particles->radius) {
        p->position[0] = particles->max_x - particles->radius;
        p->velocity[0] *= -particles->collision_loss;
    }
    if (p->position[1] <= particles->radius) {
        p->position[1] = particles->radius;
        p->velocity[1] *= -particles->collision_loss;
    }
    if (p->position[1] >= particles->max_y - particles->radius) {
        p->position[1] = particles->max_y - particles->radius;
        p->velocity[1] *= -particles->collision_loss;
    }
}

void getRandomDir(double dir[2]) {
    dir[0] = (double)rand() / RAND_MAX;
    dir[1] = 1.0 - dir[0];
}
