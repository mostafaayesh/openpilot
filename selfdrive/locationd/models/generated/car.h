#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_err_fun(double *nom_x, double *delta_x, double *out_8911319037980493856);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8969136827719042954);
void car_H_mod_fun(double *state, double *out_4762860400538887752);
void car_f_fun(double *state, double dt, double *out_80352928749016344);
void car_F_fun(double *state, double dt, double *out_3695230431015749968);
void car_h_25(double *state, double *unused, double *out_7467080157952321188);
void car_H_25(double *state, double *unused, double *out_5268194842739184464);
void car_h_24(double *state, double *unused, double *out_7173217557932906563);
void car_H_24(double *state, double *unused, double *out_6602977424378849051);
void car_h_30(double *state, double *unused, double *out_7460221781436584822);
void car_H_30(double *state, double *unused, double *out_6261858889478750397);
void car_h_26(double *state, double *unused, double *out_5003598452966825041);
void car_H_26(double *state, double *unused, double *out_1526691523865128240);
void car_h_27(double *state, double *unused, double *out_1220629100492147720);
void car_H_27(double *state, double *unused, double *out_2964092583795519483);
void car_h_29(double *state, double *unused, double *out_945435038207641831);
void car_H_29(double *state, double *unused, double *out_5649087239910336578);
void car_h_28(double *state, double *unused, double *out_8223391804285213499);
void car_H_28(double *state, double *unused, double *out_7612717511475662829);
void car_h_31(double *state, double *unused, double *out_8586992177302535901);
void car_H_31(double *state, double *unused, double *out_5298840804616144892);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}