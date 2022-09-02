#pragma once
#include "rednose/helpers/common_ekf.h"
extern "C" {
void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_err_fun(double *nom_x, double *delta_x, double *out_1694732445564659799);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_425293828362209991);
void car_H_mod_fun(double *state, double *out_7930286674743613659);
void car_f_fun(double *state, double dt, double *out_4344037338675685761);
void car_F_fun(double *state, double dt, double *out_5293371990253890476);
void car_h_25(double *state, double *unused, double *out_6157449178476545177);
void car_H_25(double *state, double *unused, double *out_2441201791329547719);
void car_h_24(double *state, double *unused, double *out_7099854435825100532);
void car_H_24(double *state, double *unused, double *out_8562867689391287577);
void car_h_30(double *state, double *unused, double *out_8651845366978984089);
void car_H_30(double *state, double *unused, double *out_77131167177700908);
void car_h_26(double *state, double *unused, double *out_7484101565902842352);
void car_H_26(double *state, double *unused, double *out_6182705110203603943);
void car_h_27(double *state, double *unused, double *out_3701132213428165031);
void car_H_27(double *state, double *unused, double *out_9143661433257580828);
void car_h_29(double *state, double *unused, double *out_3425938151143659142);
void car_H_29(double *state, double *unused, double *out_6458666777142763733);
void car_h_28(double *state, double *unused, double *out_6305537534236862682);
void car_H_28(double *state, double *unused, double *out_4495036505577437482);
void car_h_31(double *state, double *unused, double *out_206144799825270684);
void car_H_31(double *state, double *unused, double *out_6808913212436955419);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}