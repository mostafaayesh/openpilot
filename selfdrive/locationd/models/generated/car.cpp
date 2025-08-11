#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_8911319037980493856) {
   out_8911319037980493856[0] = delta_x[0] + nom_x[0];
   out_8911319037980493856[1] = delta_x[1] + nom_x[1];
   out_8911319037980493856[2] = delta_x[2] + nom_x[2];
   out_8911319037980493856[3] = delta_x[3] + nom_x[3];
   out_8911319037980493856[4] = delta_x[4] + nom_x[4];
   out_8911319037980493856[5] = delta_x[5] + nom_x[5];
   out_8911319037980493856[6] = delta_x[6] + nom_x[6];
   out_8911319037980493856[7] = delta_x[7] + nom_x[7];
   out_8911319037980493856[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_8969136827719042954) {
   out_8969136827719042954[0] = -nom_x[0] + true_x[0];
   out_8969136827719042954[1] = -nom_x[1] + true_x[1];
   out_8969136827719042954[2] = -nom_x[2] + true_x[2];
   out_8969136827719042954[3] = -nom_x[3] + true_x[3];
   out_8969136827719042954[4] = -nom_x[4] + true_x[4];
   out_8969136827719042954[5] = -nom_x[5] + true_x[5];
   out_8969136827719042954[6] = -nom_x[6] + true_x[6];
   out_8969136827719042954[7] = -nom_x[7] + true_x[7];
   out_8969136827719042954[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_4762860400538887752) {
   out_4762860400538887752[0] = 1.0;
   out_4762860400538887752[1] = 0.0;
   out_4762860400538887752[2] = 0.0;
   out_4762860400538887752[3] = 0.0;
   out_4762860400538887752[4] = 0.0;
   out_4762860400538887752[5] = 0.0;
   out_4762860400538887752[6] = 0.0;
   out_4762860400538887752[7] = 0.0;
   out_4762860400538887752[8] = 0.0;
   out_4762860400538887752[9] = 0.0;
   out_4762860400538887752[10] = 1.0;
   out_4762860400538887752[11] = 0.0;
   out_4762860400538887752[12] = 0.0;
   out_4762860400538887752[13] = 0.0;
   out_4762860400538887752[14] = 0.0;
   out_4762860400538887752[15] = 0.0;
   out_4762860400538887752[16] = 0.0;
   out_4762860400538887752[17] = 0.0;
   out_4762860400538887752[18] = 0.0;
   out_4762860400538887752[19] = 0.0;
   out_4762860400538887752[20] = 1.0;
   out_4762860400538887752[21] = 0.0;
   out_4762860400538887752[22] = 0.0;
   out_4762860400538887752[23] = 0.0;
   out_4762860400538887752[24] = 0.0;
   out_4762860400538887752[25] = 0.0;
   out_4762860400538887752[26] = 0.0;
   out_4762860400538887752[27] = 0.0;
   out_4762860400538887752[28] = 0.0;
   out_4762860400538887752[29] = 0.0;
   out_4762860400538887752[30] = 1.0;
   out_4762860400538887752[31] = 0.0;
   out_4762860400538887752[32] = 0.0;
   out_4762860400538887752[33] = 0.0;
   out_4762860400538887752[34] = 0.0;
   out_4762860400538887752[35] = 0.0;
   out_4762860400538887752[36] = 0.0;
   out_4762860400538887752[37] = 0.0;
   out_4762860400538887752[38] = 0.0;
   out_4762860400538887752[39] = 0.0;
   out_4762860400538887752[40] = 1.0;
   out_4762860400538887752[41] = 0.0;
   out_4762860400538887752[42] = 0.0;
   out_4762860400538887752[43] = 0.0;
   out_4762860400538887752[44] = 0.0;
   out_4762860400538887752[45] = 0.0;
   out_4762860400538887752[46] = 0.0;
   out_4762860400538887752[47] = 0.0;
   out_4762860400538887752[48] = 0.0;
   out_4762860400538887752[49] = 0.0;
   out_4762860400538887752[50] = 1.0;
   out_4762860400538887752[51] = 0.0;
   out_4762860400538887752[52] = 0.0;
   out_4762860400538887752[53] = 0.0;
   out_4762860400538887752[54] = 0.0;
   out_4762860400538887752[55] = 0.0;
   out_4762860400538887752[56] = 0.0;
   out_4762860400538887752[57] = 0.0;
   out_4762860400538887752[58] = 0.0;
   out_4762860400538887752[59] = 0.0;
   out_4762860400538887752[60] = 1.0;
   out_4762860400538887752[61] = 0.0;
   out_4762860400538887752[62] = 0.0;
   out_4762860400538887752[63] = 0.0;
   out_4762860400538887752[64] = 0.0;
   out_4762860400538887752[65] = 0.0;
   out_4762860400538887752[66] = 0.0;
   out_4762860400538887752[67] = 0.0;
   out_4762860400538887752[68] = 0.0;
   out_4762860400538887752[69] = 0.0;
   out_4762860400538887752[70] = 1.0;
   out_4762860400538887752[71] = 0.0;
   out_4762860400538887752[72] = 0.0;
   out_4762860400538887752[73] = 0.0;
   out_4762860400538887752[74] = 0.0;
   out_4762860400538887752[75] = 0.0;
   out_4762860400538887752[76] = 0.0;
   out_4762860400538887752[77] = 0.0;
   out_4762860400538887752[78] = 0.0;
   out_4762860400538887752[79] = 0.0;
   out_4762860400538887752[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_80352928749016344) {
   out_80352928749016344[0] = state[0];
   out_80352928749016344[1] = state[1];
   out_80352928749016344[2] = state[2];
   out_80352928749016344[3] = state[3];
   out_80352928749016344[4] = state[4];
   out_80352928749016344[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_80352928749016344[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_80352928749016344[7] = state[7];
   out_80352928749016344[8] = state[8];
}
void F_fun(double *state, double dt, double *out_3695230431015749968) {
   out_3695230431015749968[0] = 1;
   out_3695230431015749968[1] = 0;
   out_3695230431015749968[2] = 0;
   out_3695230431015749968[3] = 0;
   out_3695230431015749968[4] = 0;
   out_3695230431015749968[5] = 0;
   out_3695230431015749968[6] = 0;
   out_3695230431015749968[7] = 0;
   out_3695230431015749968[8] = 0;
   out_3695230431015749968[9] = 0;
   out_3695230431015749968[10] = 1;
   out_3695230431015749968[11] = 0;
   out_3695230431015749968[12] = 0;
   out_3695230431015749968[13] = 0;
   out_3695230431015749968[14] = 0;
   out_3695230431015749968[15] = 0;
   out_3695230431015749968[16] = 0;
   out_3695230431015749968[17] = 0;
   out_3695230431015749968[18] = 0;
   out_3695230431015749968[19] = 0;
   out_3695230431015749968[20] = 1;
   out_3695230431015749968[21] = 0;
   out_3695230431015749968[22] = 0;
   out_3695230431015749968[23] = 0;
   out_3695230431015749968[24] = 0;
   out_3695230431015749968[25] = 0;
   out_3695230431015749968[26] = 0;
   out_3695230431015749968[27] = 0;
   out_3695230431015749968[28] = 0;
   out_3695230431015749968[29] = 0;
   out_3695230431015749968[30] = 1;
   out_3695230431015749968[31] = 0;
   out_3695230431015749968[32] = 0;
   out_3695230431015749968[33] = 0;
   out_3695230431015749968[34] = 0;
   out_3695230431015749968[35] = 0;
   out_3695230431015749968[36] = 0;
   out_3695230431015749968[37] = 0;
   out_3695230431015749968[38] = 0;
   out_3695230431015749968[39] = 0;
   out_3695230431015749968[40] = 1;
   out_3695230431015749968[41] = 0;
   out_3695230431015749968[42] = 0;
   out_3695230431015749968[43] = 0;
   out_3695230431015749968[44] = 0;
   out_3695230431015749968[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_3695230431015749968[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_3695230431015749968[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3695230431015749968[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_3695230431015749968[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_3695230431015749968[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_3695230431015749968[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_3695230431015749968[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_3695230431015749968[53] = -9.8100000000000005*dt;
   out_3695230431015749968[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_3695230431015749968[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_3695230431015749968[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3695230431015749968[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3695230431015749968[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_3695230431015749968[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_3695230431015749968[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_3695230431015749968[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_3695230431015749968[62] = 0;
   out_3695230431015749968[63] = 0;
   out_3695230431015749968[64] = 0;
   out_3695230431015749968[65] = 0;
   out_3695230431015749968[66] = 0;
   out_3695230431015749968[67] = 0;
   out_3695230431015749968[68] = 0;
   out_3695230431015749968[69] = 0;
   out_3695230431015749968[70] = 1;
   out_3695230431015749968[71] = 0;
   out_3695230431015749968[72] = 0;
   out_3695230431015749968[73] = 0;
   out_3695230431015749968[74] = 0;
   out_3695230431015749968[75] = 0;
   out_3695230431015749968[76] = 0;
   out_3695230431015749968[77] = 0;
   out_3695230431015749968[78] = 0;
   out_3695230431015749968[79] = 0;
   out_3695230431015749968[80] = 1;
}
void h_25(double *state, double *unused, double *out_7467080157952321188) {
   out_7467080157952321188[0] = state[6];
}
void H_25(double *state, double *unused, double *out_5268194842739184464) {
   out_5268194842739184464[0] = 0;
   out_5268194842739184464[1] = 0;
   out_5268194842739184464[2] = 0;
   out_5268194842739184464[3] = 0;
   out_5268194842739184464[4] = 0;
   out_5268194842739184464[5] = 0;
   out_5268194842739184464[6] = 1;
   out_5268194842739184464[7] = 0;
   out_5268194842739184464[8] = 0;
}
void h_24(double *state, double *unused, double *out_7173217557932906563) {
   out_7173217557932906563[0] = state[4];
   out_7173217557932906563[1] = state[5];
}
void H_24(double *state, double *unused, double *out_6602977424378849051) {
   out_6602977424378849051[0] = 0;
   out_6602977424378849051[1] = 0;
   out_6602977424378849051[2] = 0;
   out_6602977424378849051[3] = 0;
   out_6602977424378849051[4] = 1;
   out_6602977424378849051[5] = 0;
   out_6602977424378849051[6] = 0;
   out_6602977424378849051[7] = 0;
   out_6602977424378849051[8] = 0;
   out_6602977424378849051[9] = 0;
   out_6602977424378849051[10] = 0;
   out_6602977424378849051[11] = 0;
   out_6602977424378849051[12] = 0;
   out_6602977424378849051[13] = 0;
   out_6602977424378849051[14] = 1;
   out_6602977424378849051[15] = 0;
   out_6602977424378849051[16] = 0;
   out_6602977424378849051[17] = 0;
}
void h_30(double *state, double *unused, double *out_7460221781436584822) {
   out_7460221781436584822[0] = state[4];
}
void H_30(double *state, double *unused, double *out_6261858889478750397) {
   out_6261858889478750397[0] = 0;
   out_6261858889478750397[1] = 0;
   out_6261858889478750397[2] = 0;
   out_6261858889478750397[3] = 0;
   out_6261858889478750397[4] = 1;
   out_6261858889478750397[5] = 0;
   out_6261858889478750397[6] = 0;
   out_6261858889478750397[7] = 0;
   out_6261858889478750397[8] = 0;
}
void h_26(double *state, double *unused, double *out_5003598452966825041) {
   out_5003598452966825041[0] = state[7];
}
void H_26(double *state, double *unused, double *out_1526691523865128240) {
   out_1526691523865128240[0] = 0;
   out_1526691523865128240[1] = 0;
   out_1526691523865128240[2] = 0;
   out_1526691523865128240[3] = 0;
   out_1526691523865128240[4] = 0;
   out_1526691523865128240[5] = 0;
   out_1526691523865128240[6] = 0;
   out_1526691523865128240[7] = 1;
   out_1526691523865128240[8] = 0;
}
void h_27(double *state, double *unused, double *out_1220629100492147720) {
   out_1220629100492147720[0] = state[3];
}
void H_27(double *state, double *unused, double *out_2964092583795519483) {
   out_2964092583795519483[0] = 0;
   out_2964092583795519483[1] = 0;
   out_2964092583795519483[2] = 0;
   out_2964092583795519483[3] = 1;
   out_2964092583795519483[4] = 0;
   out_2964092583795519483[5] = 0;
   out_2964092583795519483[6] = 0;
   out_2964092583795519483[7] = 0;
   out_2964092583795519483[8] = 0;
}
void h_29(double *state, double *unused, double *out_945435038207641831) {
   out_945435038207641831[0] = state[1];
}
void H_29(double *state, double *unused, double *out_5649087239910336578) {
   out_5649087239910336578[0] = 0;
   out_5649087239910336578[1] = 1;
   out_5649087239910336578[2] = 0;
   out_5649087239910336578[3] = 0;
   out_5649087239910336578[4] = 0;
   out_5649087239910336578[5] = 0;
   out_5649087239910336578[6] = 0;
   out_5649087239910336578[7] = 0;
   out_5649087239910336578[8] = 0;
}
void h_28(double *state, double *unused, double *out_8223391804285213499) {
   out_8223391804285213499[0] = state[0];
}
void H_28(double *state, double *unused, double *out_7612717511475662829) {
   out_7612717511475662829[0] = 1;
   out_7612717511475662829[1] = 0;
   out_7612717511475662829[2] = 0;
   out_7612717511475662829[3] = 0;
   out_7612717511475662829[4] = 0;
   out_7612717511475662829[5] = 0;
   out_7612717511475662829[6] = 0;
   out_7612717511475662829[7] = 0;
   out_7612717511475662829[8] = 0;
}
void h_31(double *state, double *unused, double *out_8586992177302535901) {
   out_8586992177302535901[0] = state[8];
}
void H_31(double *state, double *unused, double *out_5298840804616144892) {
   out_5298840804616144892[0] = 0;
   out_5298840804616144892[1] = 0;
   out_5298840804616144892[2] = 0;
   out_5298840804616144892[3] = 0;
   out_5298840804616144892[4] = 0;
   out_5298840804616144892[5] = 0;
   out_5298840804616144892[6] = 0;
   out_5298840804616144892[7] = 0;
   out_5298840804616144892[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_8911319037980493856) {
  err_fun(nom_x, delta_x, out_8911319037980493856);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8969136827719042954) {
  inv_err_fun(nom_x, true_x, out_8969136827719042954);
}
void car_H_mod_fun(double *state, double *out_4762860400538887752) {
  H_mod_fun(state, out_4762860400538887752);
}
void car_f_fun(double *state, double dt, double *out_80352928749016344) {
  f_fun(state,  dt, out_80352928749016344);
}
void car_F_fun(double *state, double dt, double *out_3695230431015749968) {
  F_fun(state,  dt, out_3695230431015749968);
}
void car_h_25(double *state, double *unused, double *out_7467080157952321188) {
  h_25(state, unused, out_7467080157952321188);
}
void car_H_25(double *state, double *unused, double *out_5268194842739184464) {
  H_25(state, unused, out_5268194842739184464);
}
void car_h_24(double *state, double *unused, double *out_7173217557932906563) {
  h_24(state, unused, out_7173217557932906563);
}
void car_H_24(double *state, double *unused, double *out_6602977424378849051) {
  H_24(state, unused, out_6602977424378849051);
}
void car_h_30(double *state, double *unused, double *out_7460221781436584822) {
  h_30(state, unused, out_7460221781436584822);
}
void car_H_30(double *state, double *unused, double *out_6261858889478750397) {
  H_30(state, unused, out_6261858889478750397);
}
void car_h_26(double *state, double *unused, double *out_5003598452966825041) {
  h_26(state, unused, out_5003598452966825041);
}
void car_H_26(double *state, double *unused, double *out_1526691523865128240) {
  H_26(state, unused, out_1526691523865128240);
}
void car_h_27(double *state, double *unused, double *out_1220629100492147720) {
  h_27(state, unused, out_1220629100492147720);
}
void car_H_27(double *state, double *unused, double *out_2964092583795519483) {
  H_27(state, unused, out_2964092583795519483);
}
void car_h_29(double *state, double *unused, double *out_945435038207641831) {
  h_29(state, unused, out_945435038207641831);
}
void car_H_29(double *state, double *unused, double *out_5649087239910336578) {
  H_29(state, unused, out_5649087239910336578);
}
void car_h_28(double *state, double *unused, double *out_8223391804285213499) {
  h_28(state, unused, out_8223391804285213499);
}
void car_H_28(double *state, double *unused, double *out_7612717511475662829) {
  H_28(state, unused, out_7612717511475662829);
}
void car_h_31(double *state, double *unused, double *out_8586992177302535901) {
  h_31(state, unused, out_8586992177302535901);
}
void car_H_31(double *state, double *unused, double *out_5298840804616144892) {
  H_31(state, unused, out_5298840804616144892);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
