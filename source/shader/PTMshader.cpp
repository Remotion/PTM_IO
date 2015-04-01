/////////////////////////////////////////////////////////////
// Shader Plugin  PTMshader
/////////////////////////////////////////////////////////////
// Date: 25.10.2004 Time: 03:02:59
/////////////////////////////////////////////////////////////
// (c) 2004 REMOTION, all rights reserved 
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "re_print.h"
#include "c4d_symbols.h"
#include "XPTMshader.h"

#include "PresetTools.h"
#include "Container\re_TMap2d.h"

#include "MathTools03.h"
#include "re_exVector.h"
#include "Optimize.h"

#include "Container\VectorT.h"
#include "Container\MatrixT.h"

#include "..\PTM_Types.h"
#include "..\PTM_IO.h"

#define DI_PTMSHADER_SHADER_ID 1015747
#define DISKLEVEL 0


//#################################################################
class PTMshaderShaderData : public ShaderData
{
		INSTANCEOF(PTMshaderShaderData,ShaderData)
	private:
		Filename		name;
		Real			bias;
		Real			scale;

		LONG			part;

		TMap2d<TryByte>	 rgb;
		TMap2d<TryByte>	 a012;
		TMap2d<TryByte>	 a345;

		Vector		scale012;
		Vector		scale345;
		Vector		bias012;
		Vector		bias345;

		Real		difsuse_gain;
		LONG		pricnt;
		Bool		iscalc;

	public:
		////inherited from NodeData////
		virtual Bool Init(GeListNode *node);
		//virtual void Free(GeListNode *node);
		//virtual Bool Read(GeListNode *node, HyperFile *hf, LONG level);
		//virtual Bool Write(GeListNode *node, HyperFile *hf);
		//virtual Bool Message(GeListNode *node, LONG type, void *data);
		//virtual Bool CopyTo(NodeData *dest, GeListNode *snode, GeListNode *dnode, LONG flags, AliasTrans *trn);
		//virtual Bool GetDDescription(GeListNode *node, Description *description,LONG &flags);
		//virtual Bool GetDParameter(GeListNode *node, const DescID &id,GeData &t_data,LONG &flags);
		//virtual Bool GetDEnabling(GeListNode *node, const DescID &id,GeData &t_data,LONG flags,const BaseContainer *itemdesc);
		//virtual Bool SetDParameter(GeListNode *node, const DescID &id,const GeData &t_data,LONG &flags);
		//static NodeData *Alloc(void) { return gNew MyNodeData; }
		////inherited from NodeData////

		//virtual	Bool Draw(PluginShader *sh, BaseObject *op, BaseTag *tag, BaseDraw *bd, BaseDrawHelp *bh);
		virtual	LONG GetRenderInfo(PluginShader *sh);
		virtual	LONG InitRender(PluginShader *sh, InitRenderStruct *irs);
		virtual	Vector Output(PluginShader *sh, ChannelData *cd);
		virtual	void FreeRender(PluginShader *sh);
		//
		static NodeData *Alloc(void) { return gNew PTMshaderShaderData; }
};



//########################## Init ###############################
Bool PTMshaderShaderData::Init(GeListNode *node)
{
	// initialize settings
	PluginShader	*op	= (PluginShader*)node;
	BaseContainer *data = op->GetDataInstance();
	//...

	iscalc = FALSE;

	return TRUE;
}
//#################################################################
LONG PTMshaderShaderData::GetRenderInfo(PluginShader *sh)
{
	LONG res = 0;

	return SHADER_DUDVREQUIRED;
}
//#################################################################
LONG PTMshaderShaderData::InitRender(PluginShader *sh, InitRenderStruct *irs)
{
	BaseContainer *data = sh->GetDataInstance();
	name = data->GetFilename(XPTM_FILE_NAME);
	part = data->GetLong(XPTM_PART);


	Bool			ok    = FALSE;
	AutoAlloc<BaseFile>	file; if (!file) return LOAD_NOMEM;
	if (!file->Open(name,GE_READ,FILE_NODIALOG,GE_MOTOROLA))  return LOAD_NOMEM;

	//Read PTM Info
	PTMformat	format;
	LONG		width;
	LONG		height;
	LONG		compres;
	Bool		tRGB = FALSE;
	LONG		compSize[18];
	LONG		offsetSize[18];

	String line;
	//Header String -----------------------------------------
	ReadPhrase(file,&line); //if(line!="PTM_1.2")

	//Format String -----------------------------------------
	ReadPhrase(file,&line); 
	if(line=="PTM_FORMAT_RGB")			format = PTM_FORMAT_RGB;		else
	if(line=="PTM_FORMAT_LUM")			format = PTM_FORMAT_LUM;		else
	if(line=="PTM_FORMAT_LRGB")			format = PTM_FORMAT_LRGB;		else
	
	if(line=="PTM_FORMAT_PTM_LUT")		format = PTM_FORMAT_PTM_LUT;	else
	if(line=="PTM_FORMAT_PTM_C_LUT")	format = PTM_FORMAT_PTM_C_LUT;	else
	
	if(line=="PTM_FORMAT_JPEG_RGB")		format = PTM_FORMAT_JPEG_RGB;	else
	if(line=="PTM_FORMAT_JPEG_LRGB")	format = PTM_FORMAT_JPEG_LRGB;	else
	
	if(line=="PTM_FORMAT_JPEGLS_RGB")	format = PTM_FORMAT_JPEGLS_RGB;	else
	if(line=="PTM_FORMAT_JPEGLS_LRGB")	format = PTM_FORMAT_JPEGLS_LRGB;

	//Wrong Type
	if(format==PTM_FORMAT_LUM) return IMAGE_WRONGTYPE;
	//if(format==PTM_FORMAT_RGB || format==PTM_FORMAT_LUM || format==PTM_FORMAT_LRGB) return IMAGE_WRONGTYPE;
	if(format==PTM_FORMAT_PTM_LUT || format==PTM_FORMAT_PTM_C_LUT) return IMAGE_WRONGTYPE;
	if(format==PTM_FORMAT_JPEG_RGB || format==PTM_FORMAT_JPEGLS_RGB) tRGB = TRUE;

	//Image Size (width and height)
	ReadPhrase(file,&line);	width  = line.StringToReal();
	ReadPhrase(file,&line); height = line.StringToReal();

	//Scale and Bias ( Cfinal = (Craw - bias) * scale )
	ReadPhrase(file,&line);	scale012.x = line.StringToReal();
	ReadPhrase(file,&line); scale012.y = line.StringToReal();
	ReadPhrase(file,&line); scale012.z = line.StringToReal();
	ReadPhrase(file,&line); scale345.x = line.StringToReal();
	ReadPhrase(file,&line); scale345.y = line.StringToReal();
	ReadPhrase(file,&line); scale345.z = line.StringToReal();

	ReadPhrase(file,&line);	bias012.x = line.StringToReal()/256.0;
	ReadPhrase(file,&line); bias012.y = line.StringToReal()/256.0;
	ReadPhrase(file,&line); bias012.z = line.StringToReal()/256.0;
	ReadPhrase(file,&line); bias345.x = line.StringToReal()/256.0;
	ReadPhrase(file,&line); bias345.y = line.StringToReal()/256.0;
	ReadPhrase(file,&line); bias345.z = line.StringToReal()/256.0;

	//print(scale012,scale345);
	//print(bias012,bias345);

	data->SetVector(XPTM_SCALE_012,scale012);
	data->SetVector(XPTM_SCALE_345,scale345);
	data->SetVector(XPTM_BIAS_012,bias012);
	data->SetVector(XPTM_BIAS_345,bias345);

	if(format==PTM_FORMAT_JPEG_RGB || format==PTM_FORMAT_JPEG_LRGB
	|| format==PTM_FORMAT_JPEGLS_RGB || format==PTM_FORMAT_JPEGLS_LRGB)
	{
		//Compression Parameter.
		ReadPhrase(file,&line); compres = line.StringToLong(); 
		//print("compres ",compres);

		//Transforms 18(RGB) or 9(LRGB)
		//Transform Constant Name Integer Value
		//No transform NOTHING = 0,
		//Plane Inversion PLANE_INVERSION = 1,
		//Motion Comp. MOTION_COMPENSATION = 2,
		LONG icnt;
		if(tRGB) icnt = 18; else icnt = 9;
		for(LONG c=0; c<icnt; c++) ReadPhrase(file,&line);

		//MotionVectors 36(RGB) or 18(LRGB)
		if(tRGB) icnt = 36; else icnt = 18;
		for(LONG c=0; c<icnt; c++) ReadPhrase(file,&line);

		//Order 18(RGB) or 9(LRGB)
		if(tRGB) icnt = 18; else icnt = 9;
		for(LONG c=0; c<icnt; c++) ReadPhrase(file,&line);

		//Reference Planes 18(RGB) or 9(LRGB)
		if(tRGB) icnt = 18; else icnt = 9;
		for(LONG c=0; c<icnt; c++) ReadPhrase(file,&line);

		//Compressed Size 18(RGB) or 9(LRGB)
		if(tRGB) icnt = 18; else icnt = 9;
		for(LONG c=0; c<icnt; c++){
			ReadPhrase(file,&line);
			compSize[c] = line.StringToLong();
		}

		//Side Information Size 18(RGB) or 9(LRGB)
		if(tRGB) icnt = 18; else icnt = 9;
		for(LONG c=0; c<icnt; c++){ 
			ReadPhrase(file,&line);
			offsetSize[c] = line.StringToLong();
		}
	} // Compresed Only


	LONG startOffset = file->GetPosition();

	// JPEG Channels TODO:
	if(format==PTM_FORMAT_JPEG_RGB || format==PTM_FORMAT_JPEG_LRGB
	|| format==PTM_FORMAT_JPEGLS_RGB || format==PTM_FORMAT_JPEGLS_LRGB)
	{
		/*
		LONG startR;
		LONG startG;
		LONG startB;
		LONG frame = 0;

		if(tRGB){
			startR = startOffset + GetChOff(0+frame,compSize,offsetSize);
			startG = startOffset + GetChOff(6+frame,compSize,offsetSize);
			startB = startOffset + GetChOff(12+frame,compSize,offsetSize);
		}else{
			startR = startOffset + GetChOff(0+frame*3,compSize,offsetSize);
			startG = startOffset + GetChOff(1+frame*3,compSize,offsetSize);
			startB = startOffset + GetChOff(2+frame*3,compSize,offsetSize);
		}

		//print("JPG Read");
		//print(startR,startG,startB);
		if (ReadTryJPG(file,rgb,startR,startG,startB)==IMAGE_OK) ok = TRUE;
		*/

		//NOT SUPPORTED
	}



	//LONG frame = data->GetLong(XPTM_PART);
	// RAW Channels
	if(format==PTM_FORMAT_RGB || format==PTM_FORMAT_LRGB){ //format==PTM_FORMAT_LUM
		//print(width);
		//print(height);
		//if (ReadGrayRAW(file,bmp,width,height,format,frame)==IMAGE_OK) ok = TRUE;
		if (ReadA6RAW(file,a012,a345,width,height)==IMAGE_OK &&
			ReadRGBRAW(file,rgb ,width,height)==IMAGE_OK) ok = TRUE;
		else print("ERROR Raw Read");
	}
	pricnt = 0;

	if(ok) return LOAD_OK;
	else return LOAD_NOMEM;
}

//###############################################################################
inline Vector TangentVec( const Vector &uvA, const Vector &uvB, const Vector &uvC
			      ,const Vector & pA, const Vector & pB, const Vector &pC
			      ,const Vector & nA, const Vector & nB, const Vector &nC)
{
	//TODO:
	Vector	vAB;
	Vector	vAC;
	Vector	n;
	Vector	res;
	Vector	temp_norm0, temp_norm1;
	Vector	vProjAB, vProjAC;
	Real	dot_tmp0, dot_tmp1;
	Real	duAB, duAC, dvAB, dvAC;

	vAB = pB - pA;
	vAC = pC - pA;
	n = nA;

	dot_tmp0 = n * vAB;
	dot_tmp1 = n * vAC;

	temp_norm0 = n;
	temp_norm1 = n;

	temp_norm0 *= dot_tmp0;
	temp_norm1 *= dot_tmp1;

	vProjAB = vAB - temp_norm0;
	vProjAC = vAC - temp_norm1;

	duAB = uvB.x - uvA.x;
	duAC = uvC.x - uvA.x;
	dvAB = uvB.y - uvA.y;
	dvAC = uvC.y - uvA.y;

	if (duAC * dvAB > duAB * dvAC)	{
		duAC = -duAC;
		duAB = -duAB;
	}
	vProjAB *= duAC;
	vProjAC *= duAB;

	res = vProjAB - vProjAC;
	return !res;	
}
//---------------------------------------------
inline PolyVector GetTangents(PolyVector pv_uvw, PolyVector pv_norm, PolyVector pv_pos, Bool is_triangle)
{
	PolyVector tang;

	Vector p1,p2,p3;
	Vector n1,n2,n3;
	Vector uv1,uv2,uv3;

	//Vector tang_a, tang_b, tang_c, tang_d;
	//TODO ???????????????

	if(is_triangle)
	{
		//ABC
		uv1 = pv_uvw.a;  
		uv2 = pv_uvw.b;   
		uv3 = pv_uvw.c;

		n1  = pv_norm.a; 
		n2  = pv_norm.b;  
		n3  = pv_norm.c;

		p1  = pv_pos.a; 
		p2  = pv_pos.b;   
		p3  = pv_pos.c;

		tang.a = TangentVec(uv1,uv2,uv3,p1,p2,p3,n1,n2,n3);
		

		//BCA
		uv1 = pv_uvw.b;  
		uv2 = pv_uvw.c;   
		uv3 = pv_uvw.a;

		n1  = pv_norm.b; 
		n2  = pv_norm.c;  
		n3  = pv_norm.a;

		p1  = pv_pos.b; 
		p2  = pv_pos.c;   
		p3  = pv_pos.a;

		tang.b = TangentVec(uv1,uv2,uv3,p1,p2,p3,n1,n2,n3);


		//CAB
		uv1 = pv_uvw.c;  
		uv2 = pv_uvw.a;   
		uv3 = pv_uvw.b;

		n1  = pv_norm.c; 
		n2  = pv_norm.a;  
		n3  = pv_norm.b;

		p1  = pv_pos.c; 
		p2  = pv_pos.a;   
		p3  = pv_pos.b;

		tang.c = tang.d = TangentVec(uv1,uv2,uv3,p1,p2,p3,n1,n2,n3);
	}else{
		//ABD
		uv1 = pv_uvw.a;  
		uv2 = pv_uvw.b;   
		uv3 = pv_uvw.d;

		n1  = pv_norm.a; 
		n2  = pv_norm.b;  
		n3  = pv_norm.d;

		p1  = pv_pos.a; 
		p2  = pv_pos.b;   
		p3  = pv_pos.d;

		tang.a = TangentVec(uv1,uv2,uv3,p1,p2,p3,n1,n2,n3);
		

		//BCA
		uv1 = pv_uvw.b;  
		uv2 = pv_uvw.c;   
		uv3 = pv_uvw.a;

		n1  = pv_norm.b; 
		n2  = pv_norm.c;  
		n3  = pv_norm.a;

		p1  = pv_pos.b; 
		p2  = pv_pos.c;   
		p3  = pv_pos.a;

		tang.b = TangentVec(uv1,uv2,uv3,p1,p2,p3,n1,n2,n3);


		//CDB
		uv1 = pv_uvw.c;  
		uv2 = pv_uvw.d;   
		uv3 = pv_uvw.b;

		n1  = pv_norm.c; 
		n2  = pv_norm.d;  
		n3  = pv_norm.b;

		p1  = pv_pos.c; 
		p2  = pv_pos.d;   
		p3  = pv_pos.b;

		tang.c = TangentVec(uv1,uv2,uv3,p1,p2,p3,n1,n2,n3);


		//DAC
		uv1 = pv_uvw.d;  
		uv2 = pv_uvw.a;   
		uv3 = pv_uvw.c;

		n1  = pv_norm.d; 
		n2  = pv_norm.a;  
		n3  = pv_norm.c;

		p1  = pv_pos.d; 
		p2  = pv_pos.a;   
		p3  = pv_pos.c;

		tang.d = TangentVec(uv1,uv2,uv3,p1,p2,p3,n1,n2,n3);
	}

	return tang;
}
//---------------------------------------------
Bool CalcBiTang(VolumeData *vd, Vector &binormal, Vector &tangente)
{
	//Re Polygon Object Tangent Calculation
	if(vd && vd->op && vd->op->type == O_POLYGON && vd->op->padr && vd->op->uvwcnt>0){
		LONG lpoly = vd->lhit - vd->Obj_to_ID(vd->op); //Local Polygon ID

		PolyVector pv_uvw;
		vd->GetUVW(vd->op, vd->tex->uvwind, lpoly, &pv_uvw);
		PolyVector pv_norm;
		vd->GetNormals(vd->op, lpoly, &pv_norm);

		RayPolygon rp_pos;
		rp_pos = vd->op->vadr[lpoly];

		PolyVector pv_pos;
		pv_pos.a = vd->op->padr[rp_pos.a]*vd->op->mg; //??? 
		pv_pos.b = vd->op->padr[rp_pos.b]*vd->op->mg; //???   
		pv_pos.c = vd->op->padr[rp_pos.c]*vd->op->mg; //???   
		pv_pos.d = vd->op->padr[rp_pos.d]*vd->op->mg; //???  


		Bool istri = (rp_pos.c==rp_pos.d); //Tri
		PolyVector tang = GetTangents(pv_uvw,pv_norm,pv_pos,istri);


		//Real r,s,t;
		//Bool snd = vd->GetRS(vd->lhit,vd->p,&r,&s);
		//t = 1.0-r-s;
		//if (snd)  binormal = tang.a*t + tang.d*r + tang.c*s;
		//else	  binormal = tang.a*t + tang.b*r + tang.c*s;

		//Is this better?
		RayPolyWeight wgt;
		vd->GetWeights(vd->lhit,vd->p, &wgt);
		binormal = tang.a*wgt.wa + tang.b*wgt.wb + tang.c*wgt.wc + tang.d*wgt.wd;

		Vector	normal = vd->n;
		binormal = -binormal;
		tangente = binormal % normal;

		return TRUE;
	}
	return FALSE;
}

//#################################################################
Vector PTMshaderShaderData::Output(PluginShader *sh, ChannelData *cd)
{
	Vector  uv = Repeat(cd->p,1.0); //TODO remove Mirror FX !!!
	//Vector  uv;
	//uv.x = RMod(cd->p.x, 1.0f);
	//uv.y = RMod(cd->p.y, 1.0f);
	//uv.z = RMod(cd->p.z, 1.0f);

	if(part==1) return rgb.GetBilinearPTMv(uv);
	else if(part==2) return a012.GetBilinearPTMv(uv);
	else if(part==3) return a345.GetBilinearPTMv(uv);
	else if(part==4){
		Vector coeff012 = (a012.GetBilinearPTMv(uv) - bias012) ^ scale012;
		Vector coeff345 = (a345.GetBilinearPTMv(uv) - bias345) ^ scale345;

		Vector norm = CalcNormal(coeff012,coeff345);

		return (norm+1.0)*0.5;
	}
	else if(part==5) return (a012.GetBilinearPTMv(uv) - bias012) ^ scale012;
	else if(part==6) return (a345.GetBilinearPTMv(uv) - bias345) ^ scale345;

	if(!cd->vd){
		// Editor Vorschau !!!
		Vector col;
		Vector coeff012 = (a012.GetBilinearPTMv(uv) - bias012) ^ scale012;
		Vector coeff345 = (a345.GetBilinearPTMv(uv) - bias345) ^ scale345;

		Vector lightTBN;

		lightTBN = Vector(0.1,0.5,0.0);
		lightTBN.z = 1.0;

		Vector lu2_lv2_lulv;
		// Prepare higher-order terms
		lu2_lv2_lulv.x = lightTBN.x * lightTBN.x;
		lu2_lv2_lulv.y = lightTBN.y * lightTBN.y;
		lu2_lv2_lulv.z = lightTBN.x * lightTBN.y;

		col = Dot(lu2_lv2_lulv, coeff012) + Dot(lightTBN, coeff345);
		if(col<0.0) col = 0.0;

		return col ^ rgb.GetBilinearPTMv(uv);
	}

	//if(!cd->vd) return rgb.GetBilinearPTMv(cd->p);

	VolumeData *vd = cd->vd; 

	Vector	pos = vd->p;
	Vector	normal = vd->bumpn;	//Surface shading Vector
	Vector	tangente; //Tangente U
	Vector	binormal; //Binormal V //binormal = -binormal; //dont use it

	if(!CalcBiTang(vd,binormal,tangente))	 //???????
	{
		vd->GetDUDV(vd->tex,pos,normal,normal,0,FALSE,&tangente,&binormal); //Remo: 05.05.2007
		//vd->GetDUDV(vd->tex,pos,normal,&tangente,&binormal); //Wrong Tangente !!!!
			binormal = -binormal; //???
	}
	//return (tangente+1.0)*0.5;

	Vector  rayo = SV(vd->ray->p); //Origin
	Vector  viewVec = !(rayo-pos);

	Vector lu2_lv2_lulv;
	Real diff,spec;
	Vector coll;
	Vector lightDir,lightTBN,reflectVec;
	Vector nHalf;
	Vector col = Vector(0.0,0.0,0.0);
	Vector coeff012;
	Vector coeff345;

	// read higher-order coeffs from texture and unbias
	coeff012 = (a012.GetBilinearPTMv(uv) - bias012) ^ scale012;
	// read lower-order coeffs from texture and unbias 
	coeff345 = (a345.GetBilinearPTMv(uv) - bias345) ^ scale345;

	Real	zz;
	LONG	lightCount = vd->GetLightCount();

	Vector ilum = vd->Illuminance3Ex(&pos,ILLUMINATE_SHADOW_ON); //Remo: 05.05.2007
	// Lights
	for(LONG c=0; c<lightCount; c++){
		RayLight *vLight = vd->GetLight(c); //Light Info

		// DEFUSE >>>>>>>>>>>>>>>>>>>>>>
		lightDir = !(vLight->m.off - pos);// Normalize light direction
		//vd->Illuminate(vLight,&coll,&lightDir,pos,normal,ILLUMINATE_SHADOW_OFF,0);
		// Transform light direction to Normal Space
		lightTBN = Vector(lightDir*tangente,lightDir*binormal,lightDir*normal);

		// z-extrapolation
		zz = lightTBN.z;
		if (zz <= 0.0){ //?????????
			lightTBN = !Vector(lightTBN.x,lightTBN.y,0.0);
			lightTBN *= (1.0 - zz);
		}
		lightTBN.z = 1.0;

		// Prepare higher-order terms
		lu2_lv2_lulv.x = lightTBN.x * lightTBN.x; //lu^2
		lu2_lv2_lulv.y = lightTBN.y * lightTBN.y; //lv^2
		lu2_lv2_lulv.z = lightTBN.x * lightTBN.y; //lu*lv

		// Evaluate polynomial
		diff = Dot(lu2_lv2_lulv, coeff012) + Dot(lightTBN, coeff345);
		if(diff<0.0) diff=0.0;
		col += diff * vLight->color;

		
		// SPECULAR >>>>>>>>>>>>>>>>>>>>>>
		nHalf = !(viewVec + lightDir);
		nHalf = Vector(nHalf*tangente,nHalf*binormal,nHalf*normal);
		//reflectVec = Reflect(viewVec,normal);
		//nHalf = Vector(reflectVec*tangente,reflectVec*binormal,reflectVec*normal);
		
		Real hzz = nHalf.z;
		if (hzz < 0.0){ //?????????
			nHalf = !Vector(nHalf.x,nHalf.y,0.0);
			nHalf *= (1.0 - hzz);
		}
		nHalf.z = 1.0;

		lu2_lv2_lulv.x = nHalf.x * nHalf.x;
		lu2_lv2_lulv.y = nHalf.y * nHalf.y;
		lu2_lv2_lulv.z = nHalf.x * nHalf.y;

		spec = Dot(lu2_lv2_lulv, coeff012) + Dot(nHalf, coeff345);
		if(spec<0.0) spec=0.0;
		col += spec * vLight->color * Pow(Max(Dot(reflectVec, viewVec), 0.0),8.0f);
		
	}

	// Multiply by rgb factor
	col = col ^ ilum ^ rgb.GetBilinearPTMv(uv);
	   
	return col;
}

//#################################################################
void PTMshaderShaderData::FreeRender(PluginShader *sh)
{

}
//#################################################################
Bool RegisterPTMshaderShader(void)
{
	String name=GeLoadString(IDS_PTMSHADER); if (!name.Content()) return TRUE;
	return RegisterShaderPlugin(DI_PTMSHADER_SHADER_ID,name,0,PTMshaderShaderData::Alloc,"XPTMshader",DISKLEVEL);
}
