#ifndef _PTM_TYPES_H_
#define _PTM_TYPES_H_

#include "Container\re_TMap2d.h"

enum PTMformat
{
	PTM_FORMAT_RGB = 0,
	PTM_FORMAT_LUM = 1,
	PTM_FORMAT_LRGB = 2,
	PTM_FORMAT_PTM_LUT = 3,			//not supported
	PTM_FORMAT_PTM_C_LUT = 4,		//not supported
	PTM_FORMAT_JPEG_RGB = 5,
	PTM_FORMAT_JPEG_LRGB = 6,
	PTM_FORMAT_JPEGLS_RGB = 7,
	PTM_FORMAT_JPEGLS_LRGB = 8,
};


//-----------------------------------------------------------------------------
class TryByte
{	
public:
	UCHAR x,y,z;

	TryByte() {x = y = z = 0;}
	TryByte(UCHAR ix,UCHAR iy,UCHAR iz) {x=ix;y=iy;z=iz;}
	friend const Vector operator * (Real s,const TryByte &v)
		{ return Vector(Real(v.x)*s,Real(v.y)*s,Real(v.z)*s);}
	friend const Vector operator * (const TryByte &v, Real s)
		{ return Vector(Real(v.x)*s,Real(v.y)*s,Real(v.z)*s);}
};
/*
//-----------------------------------------------------------------------------
class SixByte
{	
public:
	UCHAR x,y,z;

	SixByte() {x = y = z = 0;}
	SixByte(UCHAR ix,UCHAR iy,UCHAR iz) {x=ix;y=iy;z=iz;}
	friend const Vector operator * (Real s,const TryByte &v)
		{ return Vector(Real(v.x)*s,Real(v.y)*s,Real(v.z)*s);}
	friend const Vector operator * (const TryByte &v, Real s)
		{ return Vector(Real(v.x)*s,Real(v.y)*s,Real(v.z)*s);}
};
*/

//-----------------------------------------------------------------------------
struct Coeffs
{
	Real a0,a1,a2,a3,a4,a5;

	Coeffs() {a0 = a1 = a2 = a3 = a4 = a5 = 0.0;}
	Coeffs(Real in) {a0 = a1 = a2 = a3 = a4 = a5 = in;}
	Coeffs(Real i0,Real i1,Real i2,Real i3,Real i4,Real i5) { a0=i0; a1=i1; a2=i2; a3=i3; a4=i4; a5=i5;}
	Coeffs(const Vector &coeff012, const Vector &coeff345) { a0=coeff012.x; a1=coeff012.y; a2=coeff012.z; a3=coeff345.x; a4=coeff345.y; a5=coeff345.z;}
	Coeffs(_DONTCONSTRUCT v) {}

	friend const Coeffs operator * (Real s,const Coeffs &c)
	{
		return Coeffs(c.a0*s,c.a1*s,c.a2*s,c.a3*s,c.a4*s,c.a5*s);
	}
	friend const Coeffs operator * (const Coeffs &c, Real s)
	{
		return Coeffs(c.a0*s,c.a1*s,c.a2*s,c.a3*s,c.a4*s,c.a5*s);
	}

	friend Coeffs operator * (const Coeffs &c1,const Coeffs &c2)
	{
		return (c1.a0*c2.a0 + c1.a1*c2.a1 + c1.a2*c2.a2 + 
			    c1.a3*c2.a3 + c1.a4*c2.a4 + c1.a5*c2.a5);
	}
};
//-----------------------------------------------------------------------------
inline void  NormalToPTM(Vector &norm, Coeffs &coff)
{	
	Real nu = norm.x;
	Real nv = norm.y;
	Real nw = norm.z;

	/*
	coff.a0 = 1.0/3.0*pi*nw;
	coff.a1 = 1.0/8.0*pi*Sqrt(3.0)*nu;
	coff.a2 = 1.0/8.0*pi*Sqrt(3.0)*nv;
	coff.a3 = 0.0;
	coff.a4 = -1.0/15.0*Sqrt(5.0)*pi*nw;
	coff.a5 = -1.0/15.0*Sqrt(5.0)*pi*nw;
	*/

	coff.a5 = 1.0/3.0*pi*nw;
	coff.a4 = 1.0/8.0*pi*Sqrt(3.0)*nu;
	coff.a3 = 1.0/8.0*pi*Sqrt(3.0)*nv;
	coff.a2 = 0.0;
	coff.a1 = -1.0/15.0*Sqrt(5.0)*pi*nw;
	coff.a0 = -1.0/15.0*Sqrt(5.0)*pi*nw;
	/*
	coff.a5 = coff.a0*(1.0/2.0) - (Sqrt(45.0)/12.0)*(coff.a5+coff.a4);
	coff.a4 = coff.a1*(Sqrt(3.0)/2.0);
	coff.a3 = coff.a2*(Sqrt(3.0)/2.0);
	coff.a2 = coff.a3*(3.0/2.0);
	coff.a1 = coff.a4*(Sqrt(45.0)/4.0); //
	coff.a0 = coff.a5*(Sqrt(45.0)/4.0); //
	*/

	//TODO: !!!!!
	/*
	Real nu2 = norm.x*norm.x;
	Real nv2 = norm.y*norm.y;
	Real nw2 = norm.z*norm.z;
	coff.a0 = 1.0/3.0*nw2	+1.0/3.0*nu2	+1.0/3.0*nv2;
	coff.a1 = 1.0/15.0*nw2	+3.0/5.0*nu2	+1.0/3.0*nv2;
	coff.a2 = 1.0/3.0*nu2	+1.0/15.0*nw2	+3.0/5.0*nv2;
	coff.a3 = 3.0/5.0*nu2	-1.0/5.0*nw2	+3.0/5.0*nv2;
	coff.a4 = 1.0/7.0*nw2	+11.0/21.0*nu2	+1.0/3.0*nv2;
	coff.a5 = 1.0/7.0*nw2	+1.0/3.0*nu2	+11.0/21.0*nv2;

	coff.a0 = coff.a0*(1.0/2.0) - (Sqrt(45.0)/12.0)*(coff.a5+coff.a4);
	coff.a1 = coff.a1*(Sqrt(3.0)/2.0);
	coff.a2 = coff.a2*(Sqrt(3.0)/2.0);
	coff.a3 = coff.a3*(3.0/2.0);
	coff.a4 = coff.a4*(Sqrt(45.0)/4.0); //
	coff.a5 = coff.a5*(Sqrt(45.0)/4.0); //
	*/
}

//-----------------------------------------------------------------------------
inline Real CalcLuminance(Real lu,Real lv,Coeffs c)
{
	return c.a0*lu*lu + c.a1*lv*lv + c.a2*lu*lv + c.a3*lu + c.a4*lv + c.a5;
}
//-----------------------------------------------------------------------------
inline void DiffuseGain(Real lu,Real lv, Real g, Coeffs c, Coeffs &n)
{
	n.a0 = c.a0*g;
	n.a1 = c.a1*g;
	n.a2 = c.a2*g;
	n.a3 = (1.0-g)*(2.0*c.a0+lu+c.a2*lv)+c.a3;
	n.a4 = (1.0-g)*(2.0*c.a1+lu+c.a2*lv)+c.a4;
	n.a5 = (1.0-g)*(c.a0*lu*lu+c.a1*lv*lv+c.a2*lu*lv)+(c.a3-n.a3)*lu+(c.a4-n.a4)*lu+c.a5;
}
//-----------------------------------------------------------------------------
inline Vector CalcNormal(Coeffs c)
{
	Vector norm;

	//Real ny = 0.5*(2.0*a1*lv*lv+a2*lu*lv+2.0*a4*lv+a5)/lv; 	//Real nx = 0.5*(2.0*a0*lu*lu+a2*lu*lv+2.0*a3*lu+a5)/lu;

	// Old Methode ???
	Real div = (4.0*c.a0*c.a1-c.a2*c.a2); //if(div==0.0) div=0.1; 
	//if(div<=0.0) div=0.01;
	if(div<=0.0) div=-div;
	Real lu = (c.a2*c.a4-2.0*c.a1*c.a3) / div;
	Real lv = (c.a2*c.a3-2.0*c.a0*c.a4) / div; 

	norm.x = lu;
	norm.y = lv;

	/*
	Real len = Sqrt(lu*lu-lv*lv);
	lu = lu / len;
	lv = lv / len;
	*/

	norm.z = (1.0-lu*lu-lv*lv);
	if(norm.z>=0.0) norm.z = Sqrt(norm.z);
	else  norm.z = Sqrt(-norm.z);
	//else norm.z = 0.5; //???

	return (!norm);
	//return norm;
	
}
//-----------------------------------------------------------------------------
inline Vector CalcNormal(Vector &coeff012, Vector &coeff345)
{
	return CalcNormal(Coeffs(coeff012,coeff345));
	/*
	Real a0 = coeff012.x;
	Real a1 = coeff012.y;
	Real a2 = coeff012.z;
	Real a3 = coeff345.x;
	Real a4 = coeff345.y;
	Real a5 = coeff345.z;

	Vector norm;

	Real div = (4.0*a0*a1-a2*a2); if(div<=0.0) div=0.01;
	Real lu = (a2*a4-2.0*a1*a3) / div;
	Real lv = (a2*a3-2.0*a0*a4) / div; 

	norm.x = lu;
	norm.y = lv;

	norm.z = (1.0-lu*lu-lv*lv);
	if(norm.z>0.0) norm.z = Sqrt(norm.z);
	else norm.z = 0.5; //???

	return (!norm);
	*/
}

//-----------------------------------------------------------------------------
inline Real CalcLuminance(Real lu,Real lv,Vector &coeff012, Vector &coeff345)
{
	Real a0 = coeff012.x;
	Real a1 = coeff012.y;
	Real a2 = coeff012.z;
	Real a3 = coeff345.x;
	Real a4 = coeff345.y;
	Real a5 = coeff345.z;

	return a0*lu*lu + a1*lv*lv + a2*lu*lv + a3*lu + a4*lv + a5;
}
//-----------------------------------------------------------------------------
inline Real Phong(Vector l,Vector h,Vector n,Real ambient, Real diff, Real spec)
{
	return ambient + diff*n*l + spec*n*h;
}

//-----------------------------------------------------------------------------
inline LONG GetChOff(LONG chanel,LONG compSize[],LONG offsetSize[])
{
	if(chanel>=16 || chanel<0) return 0; //Error

	LONG chanelOffset = 0;
	for(LONG c=0; c<chanel; c++) chanelOffset += compSize[c] + offsetSize[c];
	return chanelOffset;
}

//-----------------------------------------------------------------------------
inline LONG GetRawOff(LONG chanel,LONG width, LONG height)
{
	if(chanel<0 || chanel>=6*3) return 0; //no PTM_FORMAT_LUM support else (chanel>=8*3)
	return chanel*width*height;
}




#endif