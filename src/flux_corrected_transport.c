#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))



/* Transport quantities using hte flux-corrected method described in:
   Boris, J. P., & Book, D. L. (1976). Solution of continuity equations
   by the method of flux-corrected transport. Controlled Fusion, 85-129*/
void flux_corr_method(
    double * u, /* Quantity to tranport.*/
    double * v, /* Velocity.*/
    int arrlen, /* Length of the U, V, and U_trnsp arrays.*/
    double dt, /* Time step [s].*/
    double dx, /* Grid cell width [m].*/
    double * u_trnsp /* Transported quantity.*/
)
{
    /* Calculate the nondimensional transport coefficients at the cell interfaces. */
    /* Assume fixed boundaries.*/
    double epsilon[arrlen + 1];
    epsilon[0] = v[0]*dt/dx;
    epsilon[arrlen] = v[arrlen - 1]*dt/dx;
    int i;
    for (i=1; i<=arrlen - 1; ++i)
    {
        epsilon[i] = 0.5*(v[i] + v[i - 1])*dt/dx;
    }

    /* Calculate the nondimensional diffusion coefficients. */
    double nu[arrlen + 1];
    for (i=0; i<=arrlen; ++i)
    {
        nu[i] = (1. + 2.*epsilon[i]*epsilon[i])/6.;
    }

    /* Calculate the nondimensional antidiffsion coefficients. */
    double mu[arrlen + 1];
    for (i=0; i<=arrlen; ++i)
    {
        mu[i] = (1. - epsilon[i]*epsilon[i])/6.;
    }

    /* Apply the boundary conditions. */
    double u_tilde[arrlen + 3];
    u_tilde[0] = u[0];
    u_tilde[arrlen - 1] = u[arrlen - 1];
    u_tilde[arrlen] = u[arrlen - 1]; /* Ghost cell outside the boundary.*/
    u_tilde[arrlen + 1] = u[arrlen - 1]; /* Ghost cell outside the boundary.*/
    u_tilde[arrlen + 2] = u[arrlen - 1]; /* Ghost cell outside the boundary.*/

    /* Equation 14 - transported and diffused values. */
    for (i=1; i<=arrlen - 2; ++i)
    {
        u_tilde[i] = u[i] -
            0.5*(epsilon[i + 1]*(u[i + 1] + u[i]) - epsilon[i]*(u[i] + u[i - 1])) +
            (nu[i + 1]*(u[i + 1] - u[i]) - nu[i]*(u[i] - u[i - 1]));
    }

    /* Equation 18 - raw antiduffision fluxes. */
    double phi[arrlen + 1];
    phi[0] = 0.;
    phi[arrlen] = 0.;
    for (i=1; i<=arrlen - 1; ++i)
    {
        /* Raw antiduffsion fluxes.*/
        phi[i] = mu[i]*(u_tilde[i] - u_tilde[i - 1]);
    }

    /* Equation 20 - correct fluxes.*/
    double phi_tilde[arrlen + 1];
    double s = 1.;
    phi_tilde[0] = s*MAX(0., MIN(fabs(phi[0]), MIN(s*(u_tilde[1] - u_tilde[0]), 0.)));
    phi_tilde[1] = s*MAX(0., MIN(fabs(phi[1]), MIN(s*(u_tilde[2] - u_tilde[1]), 0.)));
    for (i=1; i<=arrlen - 1; ++i)
    {
        /* |s| = 1, and sign depends on u_tilde[i + 1] >= u_tilde[i].*/
        if (u_tilde[i + 1] >= u_tilde[i])
        {
            s = 1.;
        }
        else
        {
            s = -1.;
        }

        double min_arg1 =
            MIN(s*(u_tilde[i + 2] - u_tilde[i + 1]), s*(u_tilde[i] - u_tilde[i - 1]));
        phi_tilde[i + 1] = s*MAX(0., MIN(fabs(phi[i + 1]), min_arg1));
    }

    /* Equation 19 - indicated antidiffusion.*/
    for (i=0; i<=arrlen - 1; ++i)
    {
        u_trnsp[i] = u_tilde[i] - phi_tilde[i + 1] + phi_tilde[i];
    }
    return;
}


/* Write out a snapshot of the gas state.*/
void write_snapshot(
    FILE * output, /* Output file.*/
    double time, /* Time of the snapshot.*/
    double * grid, /* Grid cell center locations.*/
    double * velocity, /* Velocity at the grid cell centers.*/
    double * density, /* Density at the grid cell centers.*/
    int grid_size /* Size of the grid, velocity, and density arrays.*/
)
{
    int i;
    for (i=0; i<grid_size; ++i)
    {
        fprintf(output, "%e,%e,%e,%e\n", time, grid[i], velocity[i], density[i]);
    }
    return;
}


int main(int argc, char ** argv)
{
    (void)argc;
    (void)argv;

    /* Define the spatial grid.*/
    int const num_grid_cells = 1000; /* [m]. */
    double const grid_cell_width = 1.; /* [m]. */
    double grid[num_grid_cells];
    int i;
    for (i=0; i<num_grid_cells; ++i)
    {
        grid[i] = i*grid_cell_width;
    }

    /* Define the temporal grid.*/
    double time = 0.; /* [s].*/
    int num_time_steps = 250;

    /* Define constants.*/
    double const initial_velocity = 10.; /* [m s-1]. */
    double const c_max = 0.4; /* Maximum courant number. */

    /* Define the variables.*/
    double density[num_grid_cells]; /* [kg m-3]. */
    memset(density, 0, sizeof(double)*num_grid_cells);
    double density_next[num_grid_cells]; /* [kg m-3]. */

    double momentum[num_grid_cells]; /* [kg m-2 s-1]. */
    memset(momentum, 0, sizeof(double)*num_grid_cells);
    double momentum_next[num_grid_cells]; /* [kg m-2 s-1]. */

    double velocity[num_grid_cells]; /* [kg m-2 s-1]. */
    memset(velocity, 0, sizeof(double)*num_grid_cells);

    /* Set the initial conditions.*/
    for (i=50; i<150; ++i)
    {
        velocity[i] = initial_velocity; /* [m s-1].*/
        density[i] = 25.; /* [kg m-3].*/
        momentum[i] = density[i]*velocity[i];
    }

    /* Open the output dataset.*/
    char * output_path = "wave-output.csv";
    FILE * output = fopen(output_path, "w");
    if (output == NULL)
    {
        fprintf(stderr, "[Error(%s: %d)] - failed to open %s.\n",
                __FILE__, __LINE__, output_path);
        return EXIT_FAILURE;
    }

    /* Write out the initial snapshot.*/
    write_snapshot(output, time, grid, velocity, density, num_grid_cells);

    for (i=0; i<num_time_steps; ++i)
    {
         /* Calulate the time step.*/
         double dt = c_max*grid_cell_width/initial_velocity;

        /* Calculate the density and momentum at the next time.*/
        flux_corr_method(density, velocity, num_grid_cells, dt, grid_cell_width,
                         density_next);
        flux_corr_method(momentum, velocity, num_grid_cells, dt, grid_cell_width,
                         momentum_next);

        /* Copy the output to the input and update the velocity. */
        int j;
        for (j=0; j<num_grid_cells; ++j)
        {
            density[j] = density_next[j];
            momentum[j] = momentum_next[j];
            if (density[j] > 0.)
            {
                velocity[j] = momentum[j] / density[j];
            }
            else
            {
                velocity[j] = 0.;
            }
        }

        /* Iterate the time and write out the snapshot.*/
        time += dt;
        write_snapshot(output, time, grid, velocity, density, num_grid_cells);
    }

    /* Close the output dataset.*/
    fclose(output);

    return EXIT_SUCCESS;
}
