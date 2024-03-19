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
    for (int x = 0; x < WIN_WIDTH; ++x) {
        printf("x: %d\n", x);
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
    return max_density;
}

double calculate_all_pressures(Particles* particles, double** pressures) {
    double max_pressure = 0;
    double total_pressure = 0; // Variable to store total pressure
    for (int x = 0; x < WIN_WIDTH; ++x) {
        printf("x: %d\n", x);
        for (int y = 0; y < WIN_HEIGHT; ++y) {
            double p[2] = {x, y};
            double density = calculate_density(particles, p);
            double pressure = convert_density_to_pressure(density);
            pressures[x][y] = pressure;
            total_pressure += pressure; // Accumulate pressure
            if (fabs(pressure) > max_pressure) {
               max_pressure = fabs(pressure);
            }
        }
    }
    // Calculate mean pressure
    double mean_pressure = total_pressure / (WIN_WIDTH * WIN_HEIGHT);
    printf("Mean Pressure: %lf\n", mean_pressure);
    printf("Max Pressure: %lf\n", max_pressure);
    return max_pressure;
}

void draw_densities(SDL_Renderer* renderer, double** densities, double max_density) {
    for (int x = 0; x < WIN_WIDTH; ++x) {
        for (int y = 0; y < WIN_HEIGHT; ++y) {
            //int color = (int)(255 * densities[x][y] / max_density);
            //SDL_SetRenderDrawColor(renderer, color, color, color, SDL_ALPHA_OPAQUE);
            int r,g,b;
            calculateColor(densities[x][y], max_density, &r, &g, &b);
            SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void draw_pressures(SDL_Renderer* renderer, double** pressures, double max_pressure) {
    for (int x = 0; x < WIN_WIDTH; ++x) {
        for (int y = 0; y < WIN_HEIGHT; ++y) {
            int r,g,b;
            calculateColor(pressures[x][y], max_pressure, &r, &g, &b);
            SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

// Function to calculate the color based on pressure
void calculateColor(double pressure, double max_pressure, int *red, int *green, int *blue) {
    // Normalize the pressure value between -1 and 1
    double normalizedPressure = pressure / max_pressure; // Adjust this scale factor as needed
    
    // Initialize RGB values
    *red = 255;
    *green = 255;
    *blue = 255;
    
    // Interpolate between blue, white, and red based on pressure
    if (normalizedPressure < 0) {
        *red = 255 + (int)(normalizedPressure * 255);
        *green = 255 + (int)(normalizedPressure * 255);
    } else if (normalizedPressure > 0) {
        *blue = 255 - (int)(normalizedPressure * 255);
        *green = 255 - (int)(normalizedPressure * 255);
    }
}