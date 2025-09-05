#ifndef REAL_INCLUDED
#define REAL_INCLUDED

// Double vs float
// #define USE_DOUBLE
// ! Double is almost unusable, since GLSL does not provide sine and cosine for double precision
#ifdef USE_DOUBLE
	#define real double
	#define rvec2 dvec2
	#define rmat2 dmat2
	#define rvec3 dvec3
	#define rmat3 dmat3
	#define rvec4 dvec4
	#define rmat4 dmat4
	#define rsin(x) sina_9((x))  // i think this is shit
	#define rcos(x) cosa_9((x))  // i think this is shit
	#define rpow(x, exp) (x)*(x) // double(pow(float((x)), (exp))) // ! This is a hack

	// sin approximation, error < 5e-9
	double sina_9(double x)
	{
		//minimax coefs for sin for 0..pi/2 range
		const double a3 = -1.666665709650470145824129400050267289858e-1LF;
		const double a5 =  8.333017291562218127986291618761571373087e-3LF;
		const double a7 = -1.980661520135080504411629636078917643846e-4LF;
		const double a9 =  2.600054767890361277123254766503271638682e-6LF;

		const double m_2_pi = 0.636619772367581343076LF;
		const double m_pi_2 = 1.57079632679489661923LF;

		double y = abs(x * m_2_pi);
		double q = floor(y);
		int quadrant = int(q);

		double t = (quadrant & 1) != 0 ? 1 - y + q : y - q;
		t *= m_pi_2;

		double t2 = t * t;
		double r = fma(fma(fma(fma(a9, t2, a7), t2, a5), t2, a3), t2*t, t);

		r = x < 0 ? -r : r;

		return (quadrant & 2) != 0 ? -r : r;
	}

	//sin approximation, error < 2e-11
	double sina_11(double x)
	{
		//minimax coefs for sin for 0..pi/2 range
		const double a3 = -1.666666660646699151540776973346659104119e-1LF;
		const double a5 =  8.333330495671426021718370503012583606364e-3LF;
		const double a7 = -1.984080403919620610590106573736892971297e-4LF;
		const double a9 =  2.752261885409148183683678902130857814965e-6LF;
		const double ab = -2.384669400943475552559273983214582409441e-8LF;

		const double m_2_pi = 0.636619772367581343076LF;
		const double m_pi_2 = 1.57079632679489661923LF;

		double y = abs(x * m_2_pi);
		double q = floor(y);
		int quadrant = int(q);

		double t = (quadrant & 1) != 0 ? 1 - y + q : y - q;
		t *= m_pi_2;

		double t2 = t * t;
		double r = fma(fma(fma(fma(fma(ab, t2, a9), t2, a7), t2, a5), t2, a3),
			t2*t, t);

		r = x < 0 ? -r : r;

		return (quadrant & 2) != 0 ? -r : r;
	}

	//cos approximation, error < 5e-9
	double cosa_9(double x)
	{
		//sin(x + PI/2) = cos(x)
		return sina_9(x + 1.57079632679489661923LF);
	}

	//cos approximation, error < 2e-11
	double cosa_11(double x)
	{
		//sin(x + PI/2) = cos(x)
		return sina_11(x + 1.57079632679489661923LF);
	}
#else
	precision highp float;
	#define real float
	#define rvec2 vec2
	#define rmat2 mat2
	#define rvec3 vec3
	#define rmat3 mat3
	#define rvec4 vec4
	#define rmat4 mat4
	#define rsin(x) sin((x))
	#define rcos(x) cos((x))
	#define rpow(x, y) pow((x), (y))
#endif

#endif