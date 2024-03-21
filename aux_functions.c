double smoothing_kernel(double r, double dst) {
    if(dst >= r) return 0;
    double volume = M_PI * pow(r, 4) / 6;
    return (r - dst) * (r - dst) / volume;
}

double smoothing_kernel_gradient(double dst, double r) {
    if (dst >= r) return 0;
    double scale = 12 / (M_PI * pow(r,4));
    return scale * (dst - r);
}

double convert_density_to_pressure(double density) {
    double density_error = density - TARGET_DENSITY;
    double pressure = P_MULT * density_error;
    return pressure;
}

double calculate_shared_pressure(double d_a, double d_b) {
    double p_a = convert_density_to_pressure(d_a);
    double p_b = convert_density_to_pressure(d_b);
    return (p_a + p_b) / 2;
}

double calculate_all_densities(Particles* particles, double** densities) {
    double max_density = -DBL_MAX;
    double total_density = 0; // Variable to store total density
    #pragma omp parallel for reduction(+:total_density) reduction(max:max_density)
    for (int x = 0; x < WIN_WIDTH; ++x) {
        for (int y = 0; y < WIN_HEIGHT; ++y) {
            double p[2] = {x, y};
            double density = calculate_density(particles, p);
            densities[x][y] = density;
            total_density += density; // Accumulate density
            if (density > max_density) {
                max_density = density;
            }
        }
    }
    // Calculate mean density
    double mean_density = total_density / (WIN_WIDTH * WIN_HEIGHT);
    printf("Mean Density: %lf\n", mean_density);
    printf("Max Density: %lf\n", max_density);
    printf("Total Density: %lf\n", total_density);
    return max_density;
}

void calculate_all_pressures(Particles* particles, double** pressures, double minmax[2]) {
    double max_pressure = -DBL_MAX;
    double total_pressure = 0; // Variable to store total pressure
    double min_pressure = DBL_MAX;
    #pragma omp parallel for reduction(+:total_pressure) reduction(max:max_pressure) reduction(min:min_pressure)
    for (int x = 0; x < WIN_WIDTH; ++x) {
        for (int y = 0; y < WIN_HEIGHT; ++y) {
            double p[2] = {x, y};
            double density = calculate_density(particles, p);
            double pressure = convert_density_to_pressure(density);
            pressures[x][y] = pressure;
            total_pressure += pressure; // Accumulate pressure
            if (pressure > max_pressure) {
               max_pressure = pressure;
            }
            if (pressure < min_pressure) {
                min_pressure = pressure;
            }
        }
    }
    // Calculate mean pressure
    double mean_pressure = total_pressure / (WIN_WIDTH * WIN_HEIGHT);
    printf("Mean Pressure: %lf\n", mean_pressure);
    printf("Max Pressure: %lf\n", max_pressure);

    minmax[0] = min_pressure;
    minmax[1] = max_pressure;
}

void draw_densities(SDL_Renderer* renderer, double** densities, double max_density) {
    #pragma omp parallel for 
    for (int x = 0; x < WIN_WIDTH; ++x) {
        for (int y = 0; y < WIN_HEIGHT; ++y) {
            int color = (int)(255 * densities[x][y] / max_density);
            SDL_SetRenderDrawColor(renderer, color, color, color, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void draw_pressures(SDL_Renderer* renderer, double** pressures, double minmax[2]) {
    #pragma omp parallel for
    for (int x = 0; x < WIN_WIDTH; ++x) {
        for (int y = 0; y < WIN_HEIGHT; ++y) {
            int r,g,b;
            calculateColor(pressures[x][y], minmax, &r, &g, &b);
            SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void calculateColor(double pressure, double minmax[2], int *red, int *green, int *blue) {
    // Normalize the pressure value between 0 and 1
    double normalizedPressure = (pressure - minmax[0]) / (minmax[1] - minmax[0]);
    // Initialize RGB values
    *red = 255;
    *green = 255;
    *blue = 255;
    
    // Interpolate between blue, white, and red based on pressure
    if (normalizedPressure < 0.5) {
        *red = (int)(255 * normalizedPressure * 2);
        *green = (int)(255 * normalizedPressure * 2);
        *blue = 255;
    } else {
        *red = 255;
        *green = (int)((1-(normalizedPressure - 0.5) * 2) * 255);
        *blue = (int)((1-(normalizedPressure - 0.5) * 2) * 255);
    }
}

void position_to_cell_coord(double position[2], int cell[2], double influence_radius) {
    cell[0] = (int)(position[0] / influence_radius);
    cell[1] = (int)(position[1] / influence_radius);
}

uint hash_cell(int x, int y) {
    return (uint)(x * 15823 + y * 9737333);
}

uint get_key_from_hash(uint hash, int n) {
    return (uint)(hash % n);
}

// Function to find the maximum value
int getMax(Particles* particles, int n) {
    int mx = particles->spatial_lookup[0].cell_key;
    for (int i = 1; i < n; i++) {
        int key = particles->spatial_lookup[i].cell_key;
        if (key > mx)
            mx = key;
    }
    return mx;
}

// Function to perform counting sort
void countSort(Particles* particles, int n, int exp) {
    // Output array
    Entry* output = (Entry*)malloc(n * sizeof(Entry));
    if (output == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int count[10] = { 0 };

    // Store count of occurrences in count[]
    for (int i = 0; i < n; i++)
        count[(particles->spatial_lookup[i].cell_key / exp) % 10]++;

    // Change count[i] so that count[i] now contains actual position of this digit in output[]
    for (int i = 1; i < 10; i++)
        count[i] += count[i - 1];

    // Build the output array
    for (int i = n - 1; i >= 0; i--) {
        int idx = --count[(particles->spatial_lookup[i].cell_key / exp) % 10];
        output[idx].cell_key = particles->spatial_lookup[i].cell_key;
        output[idx].idx = particles->spatial_lookup[i].idx; // Move the idx along with cell_key
    }

    // Copy the output array to particles->spatial_lookup[], so that particles->spatial_lookup[] now contains sorted numbers according to current digit
    for (int i = 0; i < n; i++) {
        particles->spatial_lookup[i].cell_key = output[i].cell_key;
        particles->spatial_lookup[i].idx = output[i].idx;
    }

    free(output);
}

// Function to perform radix sort
void radixsort(Particles* particles) {
    int n = particles->num_particles;
    int m = getMax(particles, n);
 
    for (int exp = 1; m / exp > 0; exp *= 10)
        countSort(particles, n, exp);
}

bool check_sorted(Particles* particles){
    for(int i = 0; i < particles->num_particles - 1; i++){
        if(particles->spatial_lookup[i].cell_key > particles->spatial_lookup[i + 1].cell_key){
            return false;
        }
    }
    return true;
}

void paint_each_point_within_radius(SDL_Renderer* renderer, Particles* particles, double sample_point[2]){
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
                if (particles->spatial_lookup[k].cell_key != key){
                    break;
                }
                int particle_index = particles->spatial_lookup[k].idx;
                double dst = hypot(sample_point[0] - particles->particles[particle_index].position[0], sample_point[1] - particles->particles[particle_index].position[1]);
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                for (int i = 0; i < particles->num_particles; ++i) {
                    int centerX = (int)particles->particles[particle_index].position[0];
                    int centerY = (int)particles->particles[particle_index].position[1];
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

            
        }
    }
}