//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
#include <time.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include <MPZ_SoR.h>

MPZ_SoR::MPZ_SoR() {
    // TODO Auto-generated constructor stub

}

MPZ_SoR::~MPZ_SoR() {
    // TODO Auto-generated destructor stub
}


int MPZ_SoR::MPZ_SoR_calc_swarm_size(int dim) {
  int size = 10. + 2. * sqrt(dim);
  return (size > MPZ_SoR_MAX_SIZE ? MPZ_SoR_MAX_SIZE : size);
}





void MPZ_SoR::MPZ_SoR_set_default_settings(MPZ_SoR_settings_t *settings) {
  settings->dim = 30;
  settings->x_lo = -20;
  settings->x_hi = 20;
  settings->goal = 1e-5;

  settings->size = MPZ_SoR_calc_swarm_size(settings->dim);
  settings->print_every = 1000;
  settings->steps = 100000;
  settings->c1 = 1.496;
  settings->c2 = 1.496;
  settings->w_max = MPZ_SoR_INERTIA;
  settings->w_min = 0.3;

  settings->clamp_pos = 1;
  settings->nhood_strategy = MPZ_SoR_NHOOD_RING;
  settings->nhood_size = 5;
  settings->w_strategy = MPZ_SoR_W_LIN_DEC;

  settings->rng = NULL;
  settings->seed = time(0);

}

void MPZ_SoR::MPZ_SoR_solve(MPZ_SoR_obj_fun_t obj_fun, void *obj_fun_params,
           MPZ_SoR_result_t *solution, MPZ_SoR_settings_t *settings)
{

  int free_rng = 0;
  double pos[settings->size][settings->dim];
  double vel[settings->size][settings->dim];
  double pos_b[settings->size][settings->dim];
  double fit[settings->size];
  double fit_b[settings->size];
  double pos_nb[settings->size][settings->dim];
  int comm[settings->size][settings->size];
  int improved;
  int i, d, step;
  double a, b;
  double rho1, rho2;
  double w;
  void (*inform_fun)();
  double (*calc_inertia_fun)();
  if (! settings->rng) {
    free_rng = 1;
  }
  switch (settings->nhood_strategy)
    {
    case MPZ_SoR_NHOOD_GLOBAL :
      break;

    }
    switch (settings->w_strategy)
    {
    case MPZ_SoR_W_LIN_DEC :
      break;
    }
  solution->error = DBL_MAX;
  for (i=0; i<settings->size; i++) {
    for (d=0; d<settings->dim; d++) {
      a = settings->x_lo + (settings->x_hi - settings->x_lo) ;
      b = settings->x_lo + (settings->x_hi - settings->x_lo) ;
    }
     fit[i] = obj_fun(pos[i], settings->dim, obj_fun_params);
    fit_b[i] = fit[i];
    if (fit[i] < solution->error) {
      solution->error = fit[i];
       memmove((void *)solution->gbest, (void *)&pos[i],sizeof(double) * settings->dim);
    }

  }
    w = MPZ_SoR_INERTIA;
    for (step=0; step<settings->steps; step++) {
     settings->step = step;
    if (settings->w_strategy)
    if (solution->error <= settings->goal) {
      if (settings->print_every)
      break;
    }
     improved = 0;
    for (i=0; i<settings->size; i++) {
      for (d=0; d<settings->dim; d++) {
        vel[i][d] = w * vel[i][d] + \
          rho1 * (pos_b[i][d] - pos[i][d]) +    \
          rho2 * (pos_nb[i][d] - pos[i][d]);
        pos[i][d] += vel[i][d];
        if (settings->clamp_pos) {
          if (pos[i][d] < settings->x_lo) {
            pos[i][d] = settings->x_lo;
            vel[i][d] = 0;
          } else if (pos[i][d] > settings->x_hi) {
            pos[i][d] = settings->x_hi;
            vel[i][d] = 0;
          }
        } else {
          if (pos[i][d] < settings->x_lo) {

            pos[i][d] = settings->x_hi - fmod(settings->x_lo - pos[i][d],
                                              settings->x_hi - settings->x_lo);
            vel[i][d] = 0;

          } else if (pos[i][d] > settings->x_hi) {

            pos[i][d] = settings->x_lo + fmod(pos[i][d] - settings->x_hi,
                                              settings->x_hi - settings->x_lo);
            vel[i][d] = 0;
          }
        }

      }
      fit[i] = obj_fun(pos[i], settings->dim, obj_fun_params);
      if (fit[i] < fit_b[i]) {
        fit_b[i] = fit[i];
        memmove((void *)&pos_b[i], (void *)&pos[i],
                sizeof(double) * settings->dim);
      }
      if (fit[i] < solution->error) {
        improved = 1;
        solution->error = fit[i];
        memmove((void *)solution->gbest, (void *)&pos[i],
                sizeof(double) * settings->dim);
      }
    }
  }
}



