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
    particles->spatial_lookup = malloc(num_particles * sizeof(Entry));
    particles->start_indices = malloc(num_particles * sizeof(int));

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

void update_particles(Particles* particles, double dt, int frames) {
    #pragma omp parallel for
    for (int i = 0; i < particles->num_particles; i++){
        Particle* p = &particles->particles[i];
        p->density = calculate_density(particles, p->position);
    }

    if (frames / 3 == 0) update_spatial_lookup(particles);

    #pragma omp parallel for
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

    for_each_point_within_radius(particles, p, pressure_force, idx);
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


void update_spatial_lookup(Particles* particles) {
    int cell[2];
    uint cell_key;
    uint key;
    uint key_prev;
    for(int i = 0; i < particles->num_particles; i++) {
        position_to_cell_coord(particles->particles[i].position, cell, particles->influence_radius);
        cell_key = get_key_from_hash(hash_cell(cell[0], cell[1]), particles->num_particles);
        particles->spatial_lookup[i].idx = i;
        particles->spatial_lookup[i].cell_key = cell_key;
        particles->start_indices[i] = INT_MAX;
    }

    radixsort(particles);
    if (!check_sorted(particles)) {
        printf("Not sorted\n");
        exit(1);
    }

    for(int i = 0; i < particles->num_particles; i++) {
        key = particles->spatial_lookup[i].cell_key;
        key_prev = i == 0 ? UINT_MAX : particles->spatial_lookup[i - 1].cell_key;
        if (key != key_prev) {
            particles->start_indices[key] = i;
        }
    }

}

void for_each_point_within_radius(Particles* particles, double sample_point[2], double pressure_force[2], int idx){
    int centre[2];
    position_to_cell_coord(sample_point, centre, particles->influence_radius);
    double dir[2];

    double offset_x[3] = {0, -1, 1};
    double offset_y[3] = {0, -1, 1};

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            uint key = get_key_from_hash(hash_cell(centre[0] + offset_x[i], centre[1] + offset_y[j]), particles->num_particles);
            int cell_start_index = particles->start_indices[key];

            for(int k = cell_start_index; k < particles->num_particles; k++){
                if (particles->spatial_lookup[k].cell_key != key) break;
                int particle_index = particles->spatial_lookup[k].idx;
                double dst = hypot(sample_point[0] - particles->particles[particle_index].position[0], sample_point[1] - particles->particles[particle_index].position[1]);

                if (dst <= particles->influence_radius){
                    if(particle_index == idx) continue;
                    Particle* current = &particles->particles[particle_index];

                    double offset[2] = {current->position[0] - sample_point[0], current->position[1] - sample_point[1]};
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

                    pressure_force[0] += -dir[0] * slope * shared_pressure / density;
                    pressure_force[1] += -dir[1] * slope * shared_pressure / density;
                }
            }

            
        }
    }
}

