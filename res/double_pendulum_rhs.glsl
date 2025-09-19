#ifndef DOUBLE_PENDULUM_RHS_INCLUDED
#define DOUBLE_PENDULUM_RHS_INCLUDED

#define D 4
#include "real.glsl"

// Double Pendulum Parameters
const real t0 = 0.0;
uniform float t_end;
uniform float g;
uniform float l1;
uniform float l2;
uniform float m1;
uniform float m2;
// #define g 9.81
// #define l1 1.0
// #define l2 1.0
// #define m1 1.0
// #define m2 1.0

void dH(rvec2 q, rvec2 p, out rvec2 dH_dq, out rvec2 dH_dp) {

	dH_dq = rvec2(
		-g*l1*(-m1 - m2)*rsin(q[0])
		+ p[0]*(-rpow(l1, 2)*m2*p[0]*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q[0] - q[1]), 2) + rpow(l1, 2)*m2, 2) + l1*l2*m2*p[1]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) + (1.0/2.0)*p[1]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2))
		+ p[1]*(l1*l2*m2*p[0]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) - rpow(l2, 2)*rpow(m2, 2)*p[1]*(m1 + m2)*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q[0] - q[1]), 2) + rpow(l2, 2)*rpow(m2, 2), 2) + (1.0/2.0)*p[0]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2)),

		g*l2*m2*rsin(q[1])
		+ p[0]*(rpow(l1, 2)*m2*p[0]*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q[0] - q[1]), 2) + rpow(l1, 2)*m2, 2) - l1*l2*m2*p[1]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) - 1.0/2.0*p[1]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2))
		+ p[1]*(-l1*l2*m2*p[0]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) + rpow(l2, 2)*rpow(m2, 2)*p[1]*(m1 + m2)*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q[0] - q[1]), 2) + rpow(l2, 2)*rpow(m2, 2), 2) - 1.0/2.0*p[0]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2))
	);

	dH_dp = rvec2(
		p[0]/(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q[0] - q[1]), 2) + rpow(l1, 2)*m2) - p[1]*rcos(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2),
		-p[0]*rcos(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2) + p[1]*(m1 + m2)/(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q[0] - q[1]), 2) + rpow(l2, 2)*rpow(m2, 2))
	);

	return;
}

rvec4 rhs(rvec4 y) {
	real q1 = y[0];
	real q2 = y[1];
	real v1 = y[2];
	real v2 = y[3];

	return rvec4(
		v1,
		v2,
		(-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))/(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q1 - q2), 2) + rpow(l1, 2)*m2) - (-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2),
		(m1 + m2)*(-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))/(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q1 - q2), 2) + rpow(l2, 2)*rpow(m2, 2)) - (-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2)
	);
}

rvec2 rhs_1(rvec4 y) {
	return y.zw;
}

rvec2 rhs_2(rvec4 y) {
	real q1 = y[0];
	real q2 = y[1];
	real v1 = y[2];
	real v2 = y[3];

	return rvec2(
		(-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))/(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q1 - q2), 2) + rpow(l1, 2)*m2) - (-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2),
		(m1 + m2)*(-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))/(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q1 - q2), 2) + rpow(l2, 2)*rpow(m2, 2)) - (-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2)
	);
}

#endif
