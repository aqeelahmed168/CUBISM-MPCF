/*
 *  Convection_SSE.cpp
 *  MPCFcore
 *
 *  Created by Diego Rossinelli on 5/19/11.
 *  Copyright 2011 ETH Zurich. All rights reserved.
 *
 */

#include <xmmintrin.h>
#include <emmintrin.h>
#include <stdio.h>
#include <stdlib.h>

#define WENO_CONSTANTS \
const __m128 one = _mm_set_ps1(1.); \
const __m128 F_4_3 = _mm_set_ps1(4./3.); \
const __m128 F_5_3 = _mm_set_ps1(5./3.); \
const __m128 F_10_3 = _mm_set_ps1(10./3.); \
const __m128 F_11_3 = _mm_set_ps1(11./3.); \
const __m128 F_13_3 = _mm_set_ps1(13./3.); \
const __m128 F_19_3 = _mm_set_ps1(19./3.);  \
const __m128 F_25_3 = _mm_set_ps1(25./3.); \
const __m128 F_31_3 = _mm_set_ps1(31./3.); \
const __m128 mywenoeps = _mm_set_ps1(WENOEPS); \
\
const __m128 M_1_6 = _mm_set_ps1(-1./6.); \
const __m128 F_1_3 = _mm_set_ps1(1./3.); \
const __m128 F_5_6 = _mm_set_ps1(5./6.); \
const __m128 M_7_6 = _mm_set_ps1(-7./6.); \
const __m128 F_11_6 = _mm_set_ps1(11./6.); \
\
const __m128 F_1_10 = _mm_set_ps1(1./10.); \
const __m128 F_6_10 = _mm_set_ps1(6./10.); \
const __m128 F_3_10 = _mm_set_ps1(3./10.);

#ifndef _PREC_DIV_

#ifdef _PREC_DIV_NONE_

#define WENO_OMEGAS \
const __m128 alpha0 = F_1_10*better_rcp(is0*is0); \
const __m128 alpha1 = F_6_10*better_rcp(is1*is1); \
const __m128 alpha2 = F_3_10*better_rcp(is2*is2); \
const __m128 inv_alpha = better_rcp(alpha0+alpha1+alpha2); \
const __m128 omega0=alpha0 * inv_alpha; \
const __m128 omega1=alpha1 * inv_alpha; \
const __m128 omega2= one-omega0-omega1;

#else

#define WENO_OMEGAS \
const __m128 alpha0 = F_1_10/(is0*is0);	\
const __m128 alpha1 = F_6_10/(is1*is1);	\
const __m128 alpha2 = F_3_10/(is2*is2);		   \
const __m128 inv_alpha = better_rcp(alpha0+alpha1+alpha2); \
const __m128 omega0=alpha0 * inv_alpha; \
const __m128 omega1=alpha1 * inv_alpha; \
const __m128 omega2= one-omega0-omega1;
#endif

#else

#define WENO_OMEGAS \
const __m128 alpha0 = F_1_10 / (is0*is0); \
const __m128 alpha1 = F_6_10 / (is1*is1); \
const __m128 alpha2 = F_3_10 / (is2*is2); \
const __m128 inv_alpha = one / (alpha0+alpha1+alpha2); \
const __m128 omega0 = alpha0 * inv_alpha; \
const __m128 omega1 = alpha1 * inv_alpha; \
const __m128 omega2 = one - omega0 - omega1;

#endif

#define WENO_MINUS \
const __m128 is0 = a*(a*F_4_3  - b*F_19_3 + c*F_11_3)	+ b*(b*F_25_3 - c*F_31_3)	+ c*c*F_10_3	+ mywenoeps; \
const __m128 is1 = b*(b*F_4_3  - c*F_13_3 + d*F_5_3)	+ c*(c*F_13_3 - d*F_13_3)	+ d*d*F_4_3		+ mywenoeps; \
const __m128 is2 = c*(c*F_10_3 - d*F_31_3 + e*F_11_3) + d*(d*F_25_3  - e*F_19_3)	+ e*e*F_4_3		+ mywenoeps; \
WENO_OMEGAS \
const __m128 recdata = ( \
omega0 *(F_1_3*a + M_7_6*b + F_11_6*c)+   \
omega1 *(M_1_6*b + F_5_6*c + F_1_3*d) +  \
omega2 *(F_1_3*c + F_5_6*d + M_1_6*e)	);

#define WENO_PLUS \
const __m128 is0 = (d*(d*F_10_3 - e*F_31_3 + f*F_11_3)	+ e*(e*F_25_3  - f*F_19_3)	+ f*f*F_4_3	)	+ mywenoeps; \
const __m128 is1 = (c*(c*F_4_3  - d*F_13_3 + e*F_5_3 )	+ d*(d*F_13_3 - e*F_13_3)	+ e*e*F_4_3	)	+ mywenoeps; \
const __m128 is2 = (b*(b*F_4_3  - c*F_19_3 + d*F_11_3) + c*(c*F_25_3  - d*F_31_3)	+ d*d*F_10_3)	+ mywenoeps; \
WENO_OMEGAS \
const __m128 recdata = ( \
omega0 *(F_1_3*f + M_7_6*e + F_11_6*d)+   \
omega1 *(M_1_6*e + F_5_6*d + F_1_3*c) +  \
omega2 *(F_1_3*d + F_5_6*c + M_1_6*b)	);

#include "Convection_SSE.h"

void Convection_SSE::_sse_convert_aligned(const float * const gptfirst, const int gptfloats, const int rowgpts,
                                          float * const rho, float * const u, float * const v, float * const w, float * const p, float * const G
                                          , float * const P
                                          )
{
	const __m128 F_1_2 = _mm_set_ps1(0.5f);
	const __m128 M_1_2 = _mm_set_ps1(-0.5f);
	const __m128 F_1 = _mm_set_ps1(1);
	
#define DESTID (dx + (InputSOA::PITCH)*dy)
	
	for(int dy=0; dy<_BLOCKSIZE_+6; dy++)
	{
		const float * const in = gptfirst + dy*gptfloats*rowgpts -gptfloats;
		
		for(int dx=0; dx<_BLOCKSIZE_+8; dx+=4)
		{
			const int WID = (dx + (int)(dx==0))*gptfloats;
			const int CID = (dx+1)*gptfloats;
			const int EID = (dx+3 - (int)(dx==_BLOCKSIZE_+4))*gptfloats;
			
			__m128 dataA0 = _mm_load_ps(in + WID);
			__m128 dataA1 = _mm_load_ps(in + WID + 4);
			__m128 dataB0 = _mm_load_ps(in + CID );
			__m128 dataB1 = _mm_load_ps(in + CID + 4);
			__m128 dataC0 = _mm_load_ps(in + CID + gptfloats);
			__m128 dataC1 = _mm_load_ps(in + CID + gptfloats + 4);
			__m128 dataD0 = _mm_load_ps(in + EID );
			__m128 dataD1 = _mm_load_ps(in + EID + 4);
			
			_MM_TRANSPOSE4_PS(dataA0, dataB0, dataC0, dataD0);
			
			_mm_store_ps(rho + DESTID, dataA0);
#ifdef _PREC_DIV_
			const __m128 inv_rho = F_1/dataA0;
#else
			const __m128 inv_rho = better_rcp(dataA0);
#endif
			_mm_store_ps(u + DESTID, dataB0*inv_rho);
			_mm_store_ps(v + DESTID, dataC0*inv_rho);
			_mm_store_ps(w + DESTID, dataD0*inv_rho);
			
			_MM_TRANSPOSE4_PS(dataA1, dataB1, dataC1, dataD1);
			
			_mm_store_ps(G + DESTID, dataB1);
            
			_mm_store_ps(P + DESTID, dataC1);
			_mm_store_ps(p + DESTID,
                         (dataA1 - (dataB0*dataB0 + dataC0*dataC0 + dataD0*dataD0)*(F_1_2*inv_rho) - dataC1)/dataB1);
		}
	}
	
#undef DESTID
}

void Convection_SSE::_sse_convert(const float * const gptfirst, const int gptfloats, const int rowgpts,
								  float * const rho, float * const u, float * const v, float * const w, float * const p, float * const G
                                  , float * const P
                                  )
{
	const __m128 F_1_2 = _mm_set_ps1(0.5f);
	const __m128 M_1_2 = _mm_set_ps1(-0.5f);
	const __m128 F_1 = _mm_set_ps1(1);
	
#define DESTID (dx + (InputSOA::PITCH)*dy)
	
	for(int dy=0; dy<_BLOCKSIZE_+6; dy++)
	{
		const float * const in = gptfirst + dy*gptfloats*rowgpts -gptfloats;
		
		for(int dx=0; dx<_BLOCKSIZE_+8; dx+=4)
		{
			const int WID = (dx + (int)(dx==0))*gptfloats;
			const int CID = (dx+1)*gptfloats;
			const int EID = (dx+3 - (int)(dx==_BLOCKSIZE_+4))*gptfloats;
			
			__m128 dataA0 = _mm_loadu_ps(in + WID);
			__m128 dataA1 = _mm_loadu_ps(in + WID + 4);
			__m128 dataB0 = _mm_loadu_ps(in + CID );
			__m128 dataB1 = _mm_loadu_ps(in + CID + 4);
			__m128 dataC0 = _mm_loadu_ps(in + CID + gptfloats);
			__m128 dataC1 = _mm_loadu_ps(in + CID + gptfloats + 4);
			__m128 dataD0 = _mm_loadu_ps(in + EID );
			__m128 dataD1 = _mm_loadu_ps(in + EID + 4);
			
			_MM_TRANSPOSE4_PS(dataA0, dataB0, dataC0, dataD0);
			
			_mm_store_ps(rho + DESTID, dataA0);
#ifdef _PREC_DIV_
			const __m128 inv_rho = F_1/dataA0;
#else
			const __m128 inv_rho = better_rcp(dataA0);
#endif
			_mm_store_ps(u + DESTID, dataB0*inv_rho);
			_mm_store_ps(v + DESTID, dataC0*inv_rho);
			_mm_store_ps(w + DESTID, dataD0*inv_rho);
			
			_MM_TRANSPOSE4_PS(dataA1, dataB1, dataC1, dataD1);
			
			_mm_store_ps(G + DESTID, dataB1);
			
			_mm_store_ps(P + DESTID, dataC1);
			_mm_store_ps(p + DESTID, (dataA1 - (dataB0*dataB0 + dataC0*dataC0 + dataD0*dataD0)*(F_1_2*inv_rho) - dataC1)/dataB1);
		}
	}
	
#undef DESTID
}

void Convection_SSE::_sse_xweno_minus(const float * const in, float * const out) const
{
	WENO_CONSTANTS
	
	static const int SX = InputSOA::PITCH;
	
	for(int dy=0; dy<TempSOA::NY; dy++)
		for(int dx=0; dx<TempSOA::NX; dx+=4)
		{
			const __m128 W = _mm_load_ps(&in[dx + SX*dy]);
			const __m128 C = _mm_load_ps(&in[dx+4 + SX*dy]);
			const __m128 E = _mm_load_ps(&in[dx+8 + SX*dy]);
			
			const __m128 a = _mm_shuffle_ps(W, _mm_shuffle_ps(W,C, _MM_SHUFFLE(0,0,3,3)), _MM_SHUFFLE(3,0,2,1));
			const __m128 b = _mm_shuffle_ps(W, C, _MM_SHUFFLE(1,0,3,2));
			const __m128 c = _mm_shuffle_ps(_mm_shuffle_ps(W,C, _MM_SHUFFLE(0,0,3,3)), C, _MM_SHUFFLE(2,1,3,0));;
			const __m128 d = C;
			const __m128 e = _mm_shuffle_ps(C, _mm_shuffle_ps(C,E, _MM_SHUFFLE(0,0,3,3)), _MM_SHUFFLE(3,0,2,1));
			
			WENO_MINUS
			
			_mm_store_ps(&out[dx + TempSOA::PITCH*dy], recdata);
		}
}

void Convection_SSE::_sse_xweno_pluss(const float * const in, float * const out) const
{
	WENO_CONSTANTS
	
	static const int SX = InputSOA::PITCH;
	
	for(int dy=0; dy<TempSOA::NY; dy++)
		for(int dx=0; dx<TempSOA::NX; dx+=4)
		{
			const __m128 W = _mm_load_ps(&in[dx + SX*dy]);
			const __m128 C = _mm_load_ps(&in[dx+4 + SX*dy]);
			const __m128 E = _mm_load_ps(&in[dx+8 + SX*dy]);
			
			const __m128 b = _mm_shuffle_ps(W, C, _MM_SHUFFLE(1,0,3,2));
			const __m128 c = _mm_shuffle_ps(_mm_shuffle_ps(W,C, _MM_SHUFFLE(0,0,3,3)), C, _MM_SHUFFLE(2,1,3,0));
			const __m128 d = C;
			const __m128 e = _mm_shuffle_ps(C, _mm_shuffle_ps(C,E, _MM_SHUFFLE(0,0,3,3)), _MM_SHUFFLE(3,0,2,1));
			const __m128 f = _mm_shuffle_ps(C, E, _MM_SHUFFLE(1,0,3,2));
			
			WENO_PLUS
			
			_mm_store_ps(&out[dx + TempSOA::PITCH*dy], recdata);
		}
}

void Convection_SSE::_sse_yweno_minus(const float * const in, float * const out)
{
	WENO_CONSTANTS
	
	static const int SX = InputSOA::PITCH;
	
	for(int dy=0; dy<TempSOA::NY; dy+=4)
	{
		const float * ptr = &in[dy];
		
		for(int dx=0; dx<TempSOA::NX; dx++)
		{
			const __m128 a = _mm_load_ps(ptr + dx*SX);
			const __m128 b = _mm_load_ps(ptr + dx*SX + SX);
			const __m128 c = _mm_load_ps(ptr + dx*SX + 2*SX);
			const __m128 d = _mm_load_ps(ptr + dx*SX + 3*SX);
			const __m128 e = _mm_load_ps(ptr + dx*SX + 4*SX);
			
			WENO_MINUS
			
			_mm_store_ps(&tmp[dx][0], recdata);
		}
		
		for(int dx=0; dx<TempSOA::NX-1; dx+=4)
		{
			__m128 data0 = _mm_load_ps(&tmp[dx][0]);
			__m128 data1 = _mm_load_ps(&tmp[dx+1][0]);
			__m128 data2 = _mm_load_ps(&tmp[dx+2][0]);
			__m128 data3 = _mm_load_ps(&tmp[dx+3][0]);
			
			_MM_TRANSPOSE4_PS(data0, data1, data2, data3);
			
			_mm_store_ps(&out[dx + TempSOA::PITCH*dy], data0);
			_mm_store_ps(&out[dx + TempSOA::PITCH*(dy+1)], data1);
			_mm_store_ps(&out[dx + TempSOA::PITCH*(dy+2)], data2);
			_mm_store_ps(&out[dx + TempSOA::PITCH*(dy+3)], data3);
		}
		
		{
			out[TempSOA::NX-1 + TempSOA::PITCH*dy] = tmp[TempSOA::NX-1][0];
			out[TempSOA::NX-1 + TempSOA::PITCH*(dy+1)] = tmp[TempSOA::NX-1][1];
			out[TempSOA::NX-1 + TempSOA::PITCH*(dy+2)] = tmp[TempSOA::NX-1][2];
			out[TempSOA::NX-1 + TempSOA::PITCH*(dy+3)] = tmp[TempSOA::NX-1][3];
		}
	}
}

void Convection_SSE::_sse_yweno_pluss(const float * const in, float * const out)
{
	WENO_CONSTANTS
	
	static const int SX = InputSOA::PITCH;
	
	for(int dy=0; dy<TempSOA::NY; dy+=4)
	{
		const float * ptr = &in[dy];
		
		for(int dx=0; dx<TempSOA::NX; dx++)
		{
			const __m128 b = _mm_load_ps(ptr + dx*SX + SX);
			const __m128 c = _mm_load_ps(ptr + dx*SX + 2*SX);
			const __m128 d = _mm_load_ps(ptr + dx*SX + 3*SX);
			const __m128 e = _mm_load_ps(ptr + dx*SX + 4*SX);
			const __m128 f = _mm_load_ps(ptr + dx*SX + 5*SX);
			
			WENO_PLUS
			
			_mm_store_ps(&tmp[dx][0], recdata);
		}
		
		for(int dx=0; dx<TempSOA::NX-1; dx+=4)
		{
			__m128 data0 = _mm_load_ps(&tmp[dx][0]);
			__m128 data1 = _mm_load_ps(&tmp[dx+1][0]);
			__m128 data2 = _mm_load_ps(&tmp[dx+2][0]);
			__m128 data3 = _mm_load_ps(&tmp[dx+3][0]);
			
			_MM_TRANSPOSE4_PS(data0, data1, data2, data3);
			
			_mm_store_ps(&out[dx + TempSOA::PITCH*dy], data0);
			_mm_store_ps(&out[dx + TempSOA::PITCH*(dy+1)], data1);
			_mm_store_ps(&out[dx + TempSOA::PITCH*(dy+2)], data2);
			_mm_store_ps(&out[dx + TempSOA::PITCH*(dy+3)], data3);
		}
		
		{
			out[TempSOA::NX-1 + TempSOA::PITCH*dy] = tmp[TempSOA::NX-1][0];
			out[TempSOA::NX-1 + TempSOA::PITCH*(dy+1)] = tmp[TempSOA::NX-1][1];
			out[TempSOA::NX-1 + TempSOA::PITCH*(dy+2)] = tmp[TempSOA::NX-1][2];
			out[TempSOA::NX-1 + TempSOA::PITCH*(dy+3)] = tmp[TempSOA::NX-1][3];
		}
		
	}
}

void Convection_SSE::_sse_zweno_minus(const float * const a_, const float * const b_,
									  const float * const c_, const float * const d_,
									  const float * const e_ , float * const out) const
{
	WENO_CONSTANTS
	
	static const int SX = InputSOA::PITCH;
	
	for(int dy=0; dy<TempSOA::NY; dy++)
		for(int dx=0; dx<TempSOA::NX; dx+=4)
		{
			const __m128 a = _mm_load_ps(a_ + dx + SX*dy);
			const __m128 b = _mm_load_ps(b_ + dx + SX*dy);
			const __m128 c = _mm_load_ps(c_ + dx + SX*dy);
			const __m128 d = _mm_load_ps(d_ + dx + SX*dy);
			const __m128 e = _mm_load_ps(e_ + dx + SX*dy);
			
			WENO_MINUS
			
			_mm_store_ps(&out[dx + TempSOA::PITCH*dy], recdata);
		}
}

void Convection_SSE::_sse_zweno_pluss(const float * const b_, const float * const c_,
									  const float * const d_, const float * const e_,
									  const float * const f_ , float * const out) const
{
	WENO_CONSTANTS
	
	static const int SX = InputSOA::PITCH;
	
	for(int dy=0; dy<TempSOA::NY; dy++)
		for(int dx=0; dx<TempSOA::NX; dx+=4)
		{
			const __m128 b = _mm_load_ps(b_ + dx + SX*dy);
			const __m128 c = _mm_load_ps(c_ + dx + SX*dy);
			const __m128 d = _mm_load_ps(d_ + dx + SX*dy);
			const __m128 e = _mm_load_ps(e_ + dx + SX*dy);
			const __m128 f = _mm_load_ps(f_ + dx + SX*dy);
			
			WENO_PLUS
			
			_mm_store_ps(&out[dx + TempSOA::PITCH*dy], recdata);
		}
}

void Convection_SSE::_sse_u_hllc(const float * const in_v_minus, const float * const in_v_plus, const float * const in_s_minus, const float * const in_s_plus, const float * const in_u_star, float * const out_u_hllc)
{
    const __m128 v_minus = _mm_load_ps(in_v_minus);
    const __m128 v_plus = _mm_load_ps(in_v_plus);
    const __m128 s_minus = _mm_load_ps(in_s_minus);
    const __m128 s_plus = _mm_load_ps(in_s_plus);
    const __m128 u_star = _mm_load_ps(in_u_star);
    
    const __m128 flag_pos = _mm_cmpgt_ps(u_star, _mm_setzero_ps());
    const __m128 not_flag_neg = _mm_cmpge_ps(u_star, _mm_setzero_ps());
    const __m128 flagother = _mm_andnot_ps(flag_pos, not_flag_neg);
    
    const __m128 signum = _3ORPS_(_mm_and_ps(flag_pos, _mm_set_ps1(1.)), _mm_andnot_ps(not_flag_neg, _mm_set_ps1(-1.)), _mm_and_ps(flagother, _mm_setzero_ps()));
    
    const __m128 xi_minus = (s_minus-v_minus)/(s_minus-u_star);
    const __m128 xi_plus  = (s_plus -v_plus )/(s_plus -u_star);
    
    const __m128 term1 = _mm_set_ps1(0.5)*(_mm_set_ps1(1.)+signum)*(v_minus+s_minus*(xi_minus-_mm_set_ps1(1.)));
    const __m128 term2 = _mm_set_ps1(0.5)*(_mm_set_ps1(1.)-signum)*(v_plus +s_plus *(xi_plus -_mm_set_ps1(1.)));
    
    _mm_store_ps(out_u_hllc,term1+term2);
}

void Convection_SSE::_sse_hlle_rho(const float * const rm, const float * const rp,
								   const float * const vm, const float * const vp,
								   const float * const am, const float * const ap,
								   float * const out)
{
	static const int P = TempSOA::PITCH;
	
#define ID (ix + P*iy)
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
			const __m128 flagminus	= _mm_cmpgt_ps(_mm_load_ps(am + ID),_mm_setzero_ps());
			const __m128 nonflagplus  = _mm_cmpge_ps(_mm_load_ps(ap + ID),_mm_setzero_ps());
			const __m128 flagother	= _mm_andnot_ps(flagminus, nonflagplus);
			
			const __m128 rminus = _mm_load_ps(rm + ID);
			const __m128 rpluss = _mm_load_ps(rp + ID);
			
			const __m128 fminus = _mm_load_ps(vm + ID)*rminus;
			const __m128 fpluss = _mm_load_ps(vp + ID)*rpluss;
			
#define aminus _mm_load_ps(am + ID)
#define apluss _mm_load_ps(ap + ID)
			
#ifdef _PREC_DIV_
			const __m128 fother = (apluss*fminus-aminus*fpluss+aminus*apluss*(rpluss-rminus))/(apluss-aminus);
#else
			const __m128 fother = (apluss*fminus-aminus*fpluss+aminus*apluss*(rpluss-rminus))*better_rcp(apluss-aminus);
#endif
			_mm_store_ps(out + ID, _3ORPS_(_mm_and_ps(flagminus, fminus), _mm_andnot_ps(nonflagplus, fpluss), _mm_and_ps(flagother, fother)));
		}
#undef ID
#undef aminus
#undef apluss
}

void Convection_SSE::_sse_hllc_rho(const float * const rm, const float * const rp,
								   const float * const vm, const float * const vp,
								   const float * const sm, const float * const sp,
								   const float * const star, float * const out)
{
	static const int P = TempSOA::PITCH;
	
#define ID (ix + P*iy)
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
            const __m128 rho_minus = _mm_load_ps(rm + ID);
			const __m128 rho_plus  = _mm_load_ps(rp + ID);
            
            const __m128 v_minus = _mm_load_ps(vm + ID);
			const __m128 v_plus  = _mm_load_ps(vp + ID);
            
            const __m128 s_minus = _mm_load_ps(sm + ID);
            const __m128 s_plus  = _mm_load_ps(sp + ID);
            const __m128 u_star = _mm_load_ps(star + ID);
            
            const __m128 q_star_minus = rho_minus*(s_minus-v_minus)/(s_minus-u_star);
            const __m128 q_star_plus  = rho_plus *(s_plus -v_plus )/(s_plus -u_star);
            
            const __m128 fminus = v_minus*rho_minus;
			const __m128 fplus  = v_plus*rho_plus;
            
            const __m128 f_star_minus = fminus + s_minus*(q_star_minus-rho_minus);
            const __m128 f_star_plus  = fplus  + s_plus *(q_star_plus -rho_plus );
            
            const __m128 flagminus	= _mm_cmpgt_ps(s_minus,_mm_setzero_ps());
			const __m128 flagplus  = _mm_cmplt_ps(s_plus,_mm_setzero_ps());
			const __m128 flagother_minus = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmpge_ps(u_star,_mm_setzero_ps()),_mm_cmple_ps(s_minus,_mm_setzero_ps())));
            const __m128 flagother_plus  = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmplt_ps(u_star,_mm_setzero_ps()),_mm_cmpge_ps(s_plus,_mm_setzero_ps())));
            
			_mm_store_ps(out + ID, _4ORPS_(_mm_and_ps(flagminus, fminus), _mm_and_ps(flagplus, fplus), _mm_and_ps(flagother_minus, f_star_minus), _mm_and_ps(flagother_plus,f_star_plus)));
		}
#undef ID
}

void Convection_SSE::_sse_hllc_phi(const float * const phim, const float * const phip,
								   const float * const vm, const float * const vp,
								   const float * const sm, const float * const sp,
								   const float * const star, float * const out)
{
	static const int P = TempSOA::PITCH;
	
#define ID (ix + P*iy)
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
            const __m128 phi_minus = _mm_load_ps(phim + ID);
			const __m128 phi_plus  = _mm_load_ps(phip + ID);
            
            const __m128 v_minus = _mm_load_ps(vm + ID);
			const __m128 v_plus  = _mm_load_ps(vp + ID);
            
            const __m128 s_minus = _mm_load_ps(sm + ID);
            const __m128 s_plus  = _mm_load_ps(sp + ID);
            const __m128 u_star = _mm_load_ps(star + ID);
            
            const __m128 q_star_minus = phi_minus*(s_minus-v_minus)/(s_minus-u_star);
            const __m128 q_star_plus  = phi_plus *(s_plus -v_plus )/(s_plus -u_star);
            
            const __m128 fminus = v_minus*phi_minus;
			const __m128 fplus  = v_plus*phi_plus;
            
            const __m128 f_star_minus = fminus + s_minus*(q_star_minus-phi_minus);
            const __m128 f_star_plus  = fplus  + s_plus *(q_star_plus -phi_plus );
            
            const __m128 flagminus	= _mm_cmpgt_ps(s_minus,_mm_setzero_ps());
			const __m128 flagplus  = _mm_cmplt_ps(s_plus,_mm_setzero_ps());
			const __m128 flagother_minus = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmpge_ps(u_star,_mm_setzero_ps()),_mm_cmple_ps(s_minus,_mm_setzero_ps())));
            const __m128 flagother_plus  = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmplt_ps(u_star,_mm_setzero_ps()),_mm_cmpge_ps(s_plus,_mm_setzero_ps())));
            
			_mm_store_ps(out + ID, _4ORPS_(_mm_and_ps(flagminus, fminus), _mm_and_ps(flagplus, fplus), _mm_and_ps(flagother_minus, f_star_minus), _mm_and_ps(flagother_plus,f_star_plus)));
		}
#undef ID
}

void Convection_SSE::_sse_hlle_vel(const float * const rm, const float * const rp,
								   const float * const vm, const float * const vp,
								   const float * const vdm, const float * const vdp,
								   const float * const am, const float * const ap,
								   float * const out)
{
	static const int P = TempSOA::PITCH;
	
#define ID (ix + P*iy)
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
			const __m128 flagminus	= _mm_cmpgt_ps(_mm_load_ps(am + ID),_mm_setzero_ps());
			const __m128 nonflagplus  = _mm_cmpge_ps(_mm_load_ps(ap + ID),_mm_setzero_ps());
			const __m128 flagother	= _mm_andnot_ps(flagminus, nonflagplus);
			
			const __m128 uminus = _mm_load_ps(vm + ID)*_mm_load_ps(rm + ID);
			const __m128 upluss = _mm_load_ps(vp + ID)*_mm_load_ps(rp + ID);
			
			const __m128 fminus = _mm_load_ps(vdm + ID)*uminus;
			const __m128 fpluss = _mm_load_ps(vdp + ID)*upluss;
			
#define aminus _mm_load_ps(am + ID)
#define apluss _mm_load_ps(ap + ID)
			
#ifdef _PREC_DIV_
			const __m128 fother = (apluss*fminus-aminus*fpluss+aminus*apluss*(upluss-uminus))/(apluss-aminus);
#else
			const __m128 fother = (apluss*fminus-aminus*fpluss+aminus*apluss*(upluss-uminus))*better_rcp(apluss-aminus);
#endif
			_mm_store_ps(out + ID, _3ORPS_(_mm_and_ps(flagminus, fminus),_mm_andnot_ps(nonflagplus, fpluss), _mm_and_ps(flagother, fother)));
		}
#undef ID
#undef aminus
#undef apluss
}

void Convection_SSE::_sse_hllc_vel(const float * const rm, const float * const rp,
								   const float * const vm, const float * const vp,
								   const float * const vdm, const float * const vdp,
								   const float * const sm, const float * const sp,
                                   const float * const star,
								   float * const out)
{
	static const int P = TempSOA::PITCH;
	
#define ID (ix + P*iy)
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
            const __m128 rho_minus = _mm_load_ps(rm + ID);
			const __m128 rho_plus  = _mm_load_ps(rp + ID);
            
            const __m128 v_minus = _mm_load_ps(vm + ID);
			const __m128 v_plus  = _mm_load_ps(vp + ID);
            
            const __m128 vd_minus = _mm_load_ps(vdm + ID);
			const __m128 vd_plus  = _mm_load_ps(vdp + ID);
            
            const __m128 s_minus = _mm_load_ps(sm + ID);
            const __m128 s_plus  = _mm_load_ps(sp + ID);
            const __m128 u_star = _mm_load_ps(star + ID);
            
            const __m128 q_star_minus = rho_minus*vd_minus*(s_minus-v_minus)/(s_minus-u_star);
            const __m128 q_star_plus  = rho_plus *vd_plus *(s_plus -v_plus )/(s_plus -u_star);
            
            const __m128 fminus = v_minus*vd_minus*rho_minus;
			const __m128 fplus  = v_plus *vd_plus *rho_plus ;
            
            const __m128 f_star_minus = fminus + s_minus*(q_star_minus-rho_minus*vd_minus);
            const __m128 f_star_plus  = fplus  + s_plus *(q_star_plus -rho_plus *vd_plus );
            
            const __m128 flagminus	= _mm_cmpgt_ps(s_minus,_mm_setzero_ps());
			const __m128 flagplus  = _mm_cmplt_ps(s_plus,_mm_setzero_ps());
			const __m128 flagother_minus = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmpge_ps(u_star,_mm_setzero_ps()),_mm_cmple_ps(s_minus,_mm_setzero_ps())));
            const __m128 flagother_plus  = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmplt_ps(u_star,_mm_setzero_ps()),_mm_cmpge_ps(s_plus,_mm_setzero_ps())));
			
            _mm_store_ps(out + ID, _4ORPS_(_mm_and_ps(flagminus, fminus), _mm_and_ps(flagplus, fplus), _mm_and_ps(flagother_minus, f_star_minus), _mm_and_ps(flagother_plus,f_star_plus)));
		}
#undef ID
}

void Convection_SSE::_sse_hlle_pvel(const float * const rm, const float * const rp,
									const float * const vm, const float * const vp,
									const float * const pm, const float * const pp,
									const float * const am, const float * const ap,
									float * const out)
{
	static const int P = TempSOA::PITCH;
#define ID (ix + P*iy)
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
#define myvminus _mm_load_ps(vm + ID)
#define myvpluss _mm_load_ps(vp + ID)
			
			const __m128 uminus = myvminus*_mm_load_ps(rm + ID);
			const __m128 upluss = myvpluss*_mm_load_ps(rp + ID);
			
			const __m128 fminus = myvminus*uminus + _mm_load_ps(pm + ID);
			const __m128 fpluss = myvpluss*upluss + _mm_load_ps(pp + ID);
			
#define aminus _mm_load_ps(am + ID)
#define apluss _mm_load_ps(ap + ID)
			
#define flagminus	_mm_cmpgt_ps(aminus,_mm_setzero_ps())
#define nonflagplus _mm_cmpge_ps(apluss,_mm_setzero_ps())
#define flagother	_mm_andnot_ps(flagminus, nonflagplus)
			
#ifdef _PREC_DIV_
#define fother (apluss*fminus-aminus*fpluss+aminus*apluss*(upluss-uminus))/(apluss-aminus)
#else
#define fother (apluss*fminus-aminus*fpluss+aminus*apluss*(upluss-uminus))*better_rcp(apluss-aminus)
#endif
			_mm_store_ps(out + ID, _3ORPS_(_mm_and_ps(flagminus, fminus), _mm_andnot_ps(nonflagplus, fpluss), _mm_and_ps(flagother, fother)));
		}
#undef myvminus
#undef myvpluss
#undef ID
#undef aminus
#undef apluss
#undef flagminus
#undef nonflagplus
#undef flagother
#undef fother
}

void Convection_SSE::_sse_hllc_pvel(const float * const rm, const float * const rp,
									const float * const vm, const float * const vp,
									const float * const pm, const float * const pp,
									const float * const sm, const float * const sp,
                                    const float * const star,
									float * const out)
{
	static const int P = TempSOA::PITCH;
#define ID (ix + P*iy)
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
            const __m128 rho_minus = _mm_load_ps(rm + ID);
			const __m128 rho_plus  = _mm_load_ps(rp + ID);
            
            const __m128 v_minus = _mm_load_ps(vm + ID);
			const __m128 v_plus  = _mm_load_ps(vp + ID);
            
            const __m128 p_minus = _mm_load_ps(pm + ID);
            const __m128 p_plus  = _mm_load_ps(pp + ID);
            
            const __m128 s_minus = _mm_load_ps(sm + ID);
            const __m128 s_plus  = _mm_load_ps(sp + ID);
            const __m128 u_star = _mm_load_ps(star + ID);
            
            const __m128 q_star_minus = rho_minus*u_star*(s_minus-v_minus)/(s_minus-u_star);
            const __m128 q_star_plus  = rho_plus *u_star*(s_plus -v_plus )/(s_plus -u_star);
            
            const __m128 fminus = v_minus*v_minus*rho_minus + p_minus;
			const __m128 fplus  = v_plus *v_plus *rho_plus  + p_plus;
            
            const __m128 f_star_minus = fminus + s_minus*(q_star_minus-rho_minus*v_minus);
            const __m128 f_star_plus  = fplus  + s_plus *(q_star_plus -rho_plus *v_plus );
            
            const __m128 flagminus	= _mm_cmpgt_ps(s_minus,_mm_setzero_ps());
			const __m128 flagplus  = _mm_cmplt_ps(s_plus,_mm_setzero_ps());
			const __m128 flagother_minus = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmpge_ps(u_star,_mm_setzero_ps()),_mm_cmple_ps(s_minus,_mm_setzero_ps())));
            const __m128 flagother_plus  = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmplt_ps(u_star,_mm_setzero_ps()),_mm_cmpge_ps(s_plus,_mm_setzero_ps())));
			
            _mm_store_ps(out + ID, _4ORPS_(_mm_and_ps(flagminus, fminus), _mm_and_ps(flagplus, fplus), _mm_and_ps(flagother_minus, f_star_minus), _mm_and_ps(flagother_plus,f_star_plus)));
        }
#undef ID
}

void Convection_SSE::_sse_hlle_e(const float * const rm, const float * const rp,
								 const float * const vdm, const float * const vdp,
								 const float * const v1m, const float * const v1p,
								 const float * const v2m, const float * const v2p,
								 const float * const pm, const float * const pp,
								 const float * const Gm, const float * const Gp,
                                 const float * const Pm, const float * const Pp,
								 const float * const am, const float * const ap,
								 float * const out)
{
	static const int P = TempSOA::PITCH;
#define ID (ix + P*iy)
	
	const __m128 F_1_2 = _mm_set_ps1(0.5);
	const __m128 M_1_2 = _mm_set_ps1(-0.5);
	const __m128 F_1 = _mm_set_ps1(1);
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
#define rminus  _mm_load_ps(rm + ID)
#define vdminus  _mm_load_ps(vdm + ID)
#define v1minus  _mm_load_ps(v1m + ID)
#define v2minus  _mm_load_ps(v2m + ID)
#define pminus  _mm_load_ps(pm + ID)
#define Gminus _mm_load_ps(Gm + ID)
#define Pminus _mm_load_ps(Pm +ID)
            
            const __m128 eminus = pminus*Gminus + F_1_2*rminus*(vdminus*vdminus + v1minus*v1minus + v2minus*v2minus) + Pminus;
            
#define rplus  _mm_load_ps(rp + ID)
#define  vdplus  _mm_load_ps(vdp + ID)
#define  v1plus  _mm_load_ps(v1p + ID)
#define v2plus  _mm_load_ps(v2p + ID)
#define pplus  _mm_load_ps(pp + ID)
#define Gplus  _mm_load_ps(Gp + ID)
#define Pplus _mm_load_ps(Pp +ID)
            
            const __m128 eplus = pplus*Gplus + F_1_2*rplus*(vdplus*vdplus + v1plus*v1plus + v2plus*v2plus) + Pplus;
            
			const __m128 fminus = vdminus*(pminus + eminus);
			const __m128 fpluss = vdplus *(pplus + eplus);
			
#define aminus _mm_load_ps(am + ID)
#define apluss _mm_load_ps(ap + ID)
			
#define flagminus	_mm_cmpgt_ps(aminus,_mm_setzero_ps())
#define nonflagplus _mm_cmpge_ps(apluss,_mm_setzero_ps())
#define flagother	_mm_andnot_ps(flagminus, nonflagplus)
			
#ifdef _PREC_DIV_
#define fother	(apluss*fminus-aminus*fpluss+aminus*apluss*(eplus-eminus))/(apluss-aminus)
#else
#define fother	(apluss*fminus-aminus*fpluss+aminus*apluss*(eplus-eminus))*better_rcp(apluss-aminus)
#endif
			_mm_store_ps(out + ID, _3ORPS_(_mm_and_ps(flagminus, fminus), _mm_andnot_ps(nonflagplus, fpluss), _mm_and_ps(flagother, fother)));
		}
#undef ID
#undef rminus
#undef vdminus
#undef v1minus
#undef v2minus
#undef pminus
#undef Gminus
#undef rplus
#undef vdplus
#undef v1plus
#undef v2plus
#undef pplus
#undef Gplus
#undef aminus
#undef apluss
#undef flagminus
#undef nonflagplus
#undef flagother
#undef fother
    
#undef Pminus
#undef Pplus
}

void Convection_SSE::_sse_hllc_e(const float * const rm, const float * const rp,
								 const float * const vdm, const float * const vdp,
								 const float * const v1m, const float * const v1p,
								 const float * const v2m, const float * const v2p,
								 const float * const pm, const float * const pp,
								 const float * const Gm, const float * const Gp,
                                 const float * const Pm, const float * const Pp,
								 const float * const sm, const float * const sp,
                                 const float * const star,
								 float * const out)
{
	static const int P = TempSOA::PITCH;
#define ID (ix + P*iy)
	
	const __m128 F_1_2 = _mm_set_ps1(0.5);
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
            const __m128 rho_minus = _mm_load_ps(rm + ID);
			const __m128 rho_plus  = _mm_load_ps(rp + ID);

            const __m128 v_minus = _mm_load_ps(vdm + ID);
			const __m128 v_plus  = _mm_load_ps(vdp + ID);
            
            const __m128 v1_minus = _mm_load_ps(v1m + ID);
			const __m128 v1_plus  = _mm_load_ps(v1p + ID);

            const __m128 v2_minus = _mm_load_ps(v2m + ID);
			const __m128 v2_plus  = _mm_load_ps(v2p + ID);

            const __m128 p_minus = _mm_load_ps(pm + ID);
            const __m128 p_plus  = _mm_load_ps(pp + ID);

            const __m128 P_minus = _mm_load_ps(Pm + ID);
            const __m128 P_plus  = _mm_load_ps(Pp + ID);
            
            const __m128 G_minus = _mm_load_ps(Gm + ID);
            const __m128 G_plus  = _mm_load_ps(Gp + ID);
            
            const __m128 s_minus = _mm_load_ps(sm + ID);
            const __m128 s_plus  = _mm_load_ps(sp + ID);
            const __m128 u_star = _mm_load_ps(star + ID);
                        
            const __m128 e_minus = p_minus*G_minus + F_1_2*rho_minus*(v_minus*v_minus + v1_minus*v1_minus + v2_minus*v2_minus) + P_minus;
            const __m128 e_plus  = p_plus *G_plus  + F_1_2*rho_plus *(v_plus *v_plus  + v1_plus *v1_plus  + v2_plus *v2_plus)  + P_plus;
            
            const __m128 q_star_minus = (s_minus-v_minus)/(s_minus-u_star)*(e_minus + (u_star-v_minus)*(rho_minus*u_star+p_minus/(s_minus-v_minus)));
            const __m128 q_star_plus  = (s_plus -v_plus )/(s_plus -u_star)*(e_plus  + (u_star-v_plus )*(rho_plus *u_star+p_plus /(s_plus -v_plus )));
            
            const __m128 fminus = v_minus*(e_minus + p_minus);
			const __m128 fplus  = v_plus *(e_plus  + p_plus );
            
            const __m128 f_star_minus = fminus + s_minus*(q_star_minus- e_minus);
            const __m128 f_star_plus  = fplus  + s_plus *(q_star_plus - e_plus );
            
            const __m128 flagminus	= _mm_cmpgt_ps(s_minus,_mm_setzero_ps());
			const __m128 flagplus  = _mm_cmplt_ps(s_plus,_mm_setzero_ps());
			const __m128 flagother_minus = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmpge_ps(u_star,_mm_setzero_ps()),_mm_cmple_ps(s_minus,_mm_setzero_ps())));
            const __m128 flagother_plus  = _mm_andnot_ps(_mm_or_ps(flagminus, flagplus), _mm_and_ps(_mm_cmplt_ps(u_star,_mm_setzero_ps()),_mm_cmpge_ps(s_plus,_mm_setzero_ps())));
			
            _mm_store_ps(out + ID, _4ORPS_(_mm_and_ps(flagminus, fminus), _mm_and_ps(flagplus, fplus), _mm_and_ps(flagother_minus, f_star_minus), _mm_and_ps(flagother_plus,f_star_plus)));
        }
#undef ID
}

void Convection_SSE::_sse_char_vel(const float * const rm, const float * const rp,
								   const float * const vm, const float * const vp,
								   const float * const pm, const float * const pp,
								   const float * const Gm, const float * const Gp,
                                   const float * const Pm, const float * const Pp,
								   float * const outm, float * const outp)
{
	/*const __m128 F_1_2 = _mm_set_ps1(0.5);
	const __m128 M_1_2 = _mm_set_ps1(-0.5);
	const __m128 F_1 = _mm_set_ps1(1);
	*/
	static const int P = TempSOA::PITCH;
	
#define ID (ix + P*iy)
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
			/*const __m128 cminus = _mm_sqrt_ps((F_1/_mm_load_ps(Gm + ID)+F_1)* _mm_max_ps((_mm_load_ps(pm + ID)+_mm_load_ps(Pm + ID))/_mm_load_ps(rm + ID), _mm_setzero_ps()));
			const __m128 cplus  = _mm_sqrt_ps((F_1/_mm_load_ps(Gp + ID)+F_1)* _mm_max_ps((_mm_load_ps(pp + ID)+_mm_load_ps(Pp + ID))/_mm_load_ps(rp + ID), _mm_setzero_ps()));*/
            
            const __m128 cminus = _mm_sqrt_ps(((_mm_load_ps(pm + ID)+_mm_load_ps(Pm + ID))/_mm_load_ps(Gm + ID) + _mm_load_ps(pm + ID))/_mm_load_ps(rm + ID));
			const __m128 cplus  = _mm_sqrt_ps(((_mm_load_ps(pp + ID)+_mm_load_ps(Pp + ID))/_mm_load_ps(Gp + ID) + _mm_load_ps(pp + ID))/_mm_load_ps(rp + ID));
            
            _mm_store_ps(outm + ID, _mm_min_ps(_mm_load_ps(vm + ID) - cminus, _mm_setzero_ps()));
            _mm_store_ps(outp + ID, _mm_max_ps(_mm_load_ps(vp + ID) + cplus , _mm_setzero_ps()));
		}
#undef ID
}

void Convection_SSE::_sse_char_vel_hllc(const float * const rm, const float * const rp,
								   const float * const vm, const float * const vp,
								   const float * const pm, const float * const pp,
								   const float * const Gm, const float * const Gp,
                                   const float * const Pm, const float * const Pp,
								   float * const outm, float * const outp, float * const out_star)
{
	const __m128 F_1_2 = _mm_set_ps1(0.5);
	const __m128 F_1 = _mm_set_ps1(1);
    const __m128 F_2 = _mm_set_ps1(2);
	
	static const int P = TempSOA::PITCH;
	
#define ID (ix + P*iy)
	
	for(int iy=0; iy<TempSOA::NY; iy++)
		for(int ix=0; ix<TempSOA::NX; ix+=4)
		{
            const __m128 rho_minus = _mm_load_ps(rm + ID);
			const __m128 rho_plus  = _mm_load_ps(rp + ID);
            
            const __m128 v_minus = _mm_load_ps(vm + ID);
			const __m128 v_plus  = _mm_load_ps(vp + ID);
            
            const __m128 p_minus = _mm_load_ps(pm + ID);
            const __m128 p_plus  = _mm_load_ps(pp + ID);
            
            const __m128 P_minus = _mm_load_ps(Pm + ID);
            const __m128 P_plus  = _mm_load_ps(Pp + ID);
            
            const __m128 G_minus = _mm_load_ps(Gm + ID);
            const __m128 G_plus  = _mm_load_ps(Gp + ID);
            
            const __m128 a_minus = _mm_sqrt_ps((F_1/G_minus+F_1)* _mm_max_ps((p_minus+P_minus/G_minus/(F_1/G_minus+F_1))*(F_1/rho_minus), _mm_setzero_ps()));
			const __m128 a_plus  = _mm_sqrt_ps((F_1/G_plus +F_1)* _mm_max_ps((p_plus +P_plus /G_plus /(F_1/G_plus +F_1))*(F_1/rho_plus ), _mm_setzero_ps()));
            
            const __m128 rho_hat = F_1_2*(rho_minus+rho_plus);
            const __m128 a_hat   = F_1_2*(a_minus+a_plus);
            const __m128 rho_hat_a_hat = rho_hat*a_hat;
            
            const __m128 u_star = F_1_2*(v_minus+v_plus+(p_minus-p_plus)/rho_hat_a_hat);
            const __m128 p_star = F_1_2*(p_minus+p_plus+(v_minus-v_plus)*rho_hat_a_hat);
            
            const __m128 flag_m = _mm_cmple_ps(p_star,p_minus);
            const __m128 flag_p = _mm_cmple_ps(p_star,p_plus );
            
            const __m128 q_minus = _mm_or_ps(_mm_andnot_ps(flag_m, _mm_sqrt_ps(F_1+F_1_2*(F_2*G_minus+F_1)/(G_minus+F_1)*(p_star/p_minus-F_1))), _mm_and_ps(flag_m, F_1));
            const __m128 q_plus  = _mm_or_ps(_mm_andnot_ps(flag_p, _mm_sqrt_ps(F_1+F_1_2*(F_2*G_plus +F_1)/(G_plus +F_1)*(p_star/p_plus -F_1))), _mm_and_ps(flag_p, F_1));
            
            const __m128 s_minus = v_minus - a_minus*q_minus;
            const __m128 s_plus  = v_plus  + a_plus *q_plus;
            
            _mm_store_ps(out_star + ID, u_star);
            _mm_store_ps(outm + ID, _mm_min_ps(s_minus, _mm_setzero_ps()));
            _mm_store_ps(outp + ID, _mm_max_ps(s_plus , _mm_setzero_ps()));
        }
#undef ID
}
void Convection_SSE::_sse_xrhsadd(const float * const f, float * const r)
{
	for(int iy=0; iy<OutputSOA::NY; iy++)
		for(int ix=0; ix<OutputSOA::NX; ix+=4)
			_mm_store_ps(r + ix + OutputSOA::PITCH*iy,
						 _mm_loadu_ps(f + ix + 1 + TempSOA::PITCH*iy) - _mm_load_ps(f + ix + TempSOA::PITCH*iy));
}

void Convection_SSE::_sse_yrhsadd(const float * const f, float * const r)
{
	static const int SP = TempSOA::PITCH;
	static const int DP = OutputSOA::PITCH;
	
	for(int iy=0; iy<OutputSOA::NY; iy+=4)
		for(int ix=0; ix<OutputSOA::NX; ix+=4)
		{
			__m128 rhs0 = _mm_loadu_ps(f + iy +  1 + SP*ix) - _mm_load_ps(f+ iy + SP*ix);
			__m128 rhs1 = _mm_loadu_ps(f + iy +  1 + SP*ix+SP) - _mm_load_ps(f+ iy + SP*ix+SP);
			__m128 rhs2 = _mm_loadu_ps(f + iy +  1 + SP*ix+2*SP) - _mm_load_ps(f+ iy + SP*ix+2*SP);
			__m128 rhs3 = _mm_loadu_ps(f + iy +  1 + SP*ix+3*SP) - _mm_load_ps(f+ iy + SP*ix+3*SP);
			
			_MM_TRANSPOSE4_PS(rhs0, rhs1, rhs2, rhs3);
			
			_mm_store_ps(r + ix + DP*iy, _mm_load_ps(r + ix + DP*iy) + rhs0);
			_mm_store_ps(r + ix + DP*iy+DP, _mm_load_ps(r + ix + DP*iy+DP) + rhs1);
			_mm_store_ps(r + ix + DP*iy+2*DP, _mm_load_ps(r + ix + DP*iy+2*DP) + rhs2);
			_mm_store_ps(r + ix + DP*iy+3*DP, _mm_load_ps(r + ix + DP*iy+3*DP) + rhs3);
		}
}

void Convection_SSE::_sse_zrhsadd(const float * const fb, const float * const ff, float * const r)
{	
	for(int iy=0; iy<OutputSOA::NY; iy++)
		for(int ix=0; ix<OutputSOA::NX; ix+=4)
			_mm_store_ps(r + ix + OutputSOA::PITCH*iy, _mm_load_ps(r + ix + OutputSOA::PITCH*iy) + 
						 _mm_load_ps(ff + ix + TempSOA::PITCH*iy) - _mm_load_ps(fb + ix + TempSOA::PITCH*iy));
}
