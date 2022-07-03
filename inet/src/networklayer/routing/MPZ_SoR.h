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

#ifndef MPZ_SoR_H_
#define MPZ_SoR_H_

class MPZ_SoR {
public:
    MPZ_SoR();
    virtual ~MPZ_SoR();


#define MPZ_SoR_MAX_SIZE 100
#define MPZ_SoR_INERTIA 0.7298
#define MPZ_SoR_NHOOD_GLOBAL 0
#define MPZ_SoR_NHOOD_RING 1
#define MPZ_SoR_NHOOD_RANDOM 2
#define MPZ_SoR_W_CONST 0
#define MPZ_SoR_W_LIN_DEC 1

typedef struct {

  double error;
  double *gbest;

} MPZ_SoR_result_t;

typedef double (*MPZ_SoR_obj_fun_t)(double *, int, void *);


typedef struct {

  int dim;
  double x_lo;
  double x_hi;
  double goal;

  int size;
  int print_every;
  int steps;
  int step;
  double c1;
  double c2;
  double w_max;
  double w_min;

  int clamp_pos;
  int nhood_strategy;
  int nhood_size;
  int w_strategy;

  void *rng;
  long seed;

} MPZ_SoR_settings_t;



int MPZ_SoR_calc_swarm_size(int dim);

void MPZ_SoR_set_default_settings(MPZ_SoR_settings_t *settings);


void MPZ_SoR_solve(MPZ_SoR_obj_fun_t obj_fun, void *obj_fun_params,
           MPZ_SoR_result_t *solution, MPZ_SoR_settings_t *settings);


};

#endif /* MPZ_SoR_H_ */
