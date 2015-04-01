/////////////////////////////////////////////////////////////
// Shader Plugin  PTMgen
/////////////////////////////////////////////////////////////
// Date: 31.10.2004 Time: 02:04:13
/////////////////////////////////////////////////////////////
// (c) 2004 REMOTION, all rights reserved 
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "re_print.h"
#include "c4d_symbols.h"
#include "XPTMgen.h"

#include "PresetTools.h"
#include "Container\re_TMap2d.h"

#include "MathTools03.h"
#include "re_exVector.h"
#include "Optimize.h"

#include "Container\VectorT.h"
#include "Container\MatrixT.h"

#include "..\PTM_Types.h"
#include "..\PTM_IO.h"

#include "re_plane.h"

#define DI_PTMGEN_SHADER_ID 1015748
#define DISKLEVEL 0

//=============================================================================
class PTM_Thread : public Thread
{
public:
	PTM_Thread(void) { 	calculating = FALSE; t_rgb=t_a012=t_a345=t_normal= NULL; }
	virtual ~PTM_Thread(void) {}

	virtual void Main(void);

	virtual const CHAR *GetThreadName(void) { return "PTM_Thread"; }

	Bool SetData(LONG sb_modus_i, Filename name_i, Bool iscalc_i, 
				TMap2d<Vector> *t_rgb_i,  TMap2d<Vector> *t_a012_i, 
				TMap2d<Vector> *t_a345_i, TMap2d<Vector> *t_normal_i);
	Bool GetData(Vector &t_scale012_o, Vector &t_scale345_o, 
				 Vector &t_bias012_o,  Vector &t_bias345_o);


	LONG			sb_modus;
	Filename		name;
	Bool			iscalc;
	Bool			calculating;

	TMap2d<Vector>	*t_rgb;
	TMap2d<Vector>	*t_a012;
	TMap2d<Vector>	*t_a345;
	TMap2d<Vector>	*t_normal;

	Vector			t_scale012;
	Vector			t_scale345;
	Vector			t_bias012;
	Vector			t_bias345;
};
//-----------------------------------------------------------------------------
Bool PTM_Thread::SetData(LONG sb_modus_i, Filename name_i, Bool iscalc_i, 
						 TMap2d<Vector> *t_rgb_i,  TMap2d<Vector> *t_a012_i, 
						 TMap2d<Vector> *t_a345_i, TMap2d<Vector> *t_normal_i)
{
	//if(!calculating || iscalc_i)
	if(!calculating)
	{
		iscalc = FALSE;
		sb_modus = sb_modus_i;
		name = name_i;
		
		t_rgb = t_rgb_i;
		t_a012 = t_a012_i;
		t_a345 = t_a345_i;
		t_normal = t_normal_i;

		return TRUE;
	}
	return FALSE;
}
//-----------------------------------------------------------------------------
Bool PTM_Thread::GetData(Vector &t_scale012_o, Vector &t_scale345_o, 
						 Vector &t_bias012_o,  Vector &t_bias345_o)
{
	if(!calculating)
	{
		t_scale012_o = t_scale012;
		t_scale345_o = t_scale345;
		t_bias012_o  = t_bias012;
		t_bias345_o  = t_bias345;
		
		return iscalc;
	}

	return FALSE;
}
//-----------------------------------------------------------------------------
void PTM_Thread::Main(void)
{
	calculating = TRUE;
	// CREATE PTM >>>>>>>>>>>>>>
	if(!iscalc && t_rgb && t_a012 && t_a345 && t_normal)
	{
		print("------ PTM Create ------");
		OTimer tim;
		String line;

		AutoAlloc<BaseFile>	file; if (!file) return;
		if (!file->Open(name,GE_READ,FILE_NODIALOG,GE_MOTOROLA)) return;

		LONG n = 13; //TEST ???????????
		ReadPhrase(file,&line);	n  = line.StringToReal();
		if(n<6) {  print("Not enough images to create PTM! ",n);  return; }

		LONG	c;
		//String	res_name = "Result",comp_name;
		String	*nameArr = bNew String[n+1]; //if(!nameArr) return;
		Vector	*lulvArr = (Vector*)GeAlloc(n*sizeof(Vector)); //if(!lulvArr) return;
		Vector	*rgbArr = (Vector*)GeAlloc(n*sizeof(Vector)); //if(!rgbArr) return;
		LONG	*indArr = (LONG *)GeAlloc(n*sizeof(LONG )); //if(!indArr) return;
		BaseBitmap **bmps = (BaseBitmap**)GeAlloc(n*sizeof(BaseBitmap*)); //if(!bmps) goto ERROR;

		if(nameArr && lulvArr && rgbArr && indArr && bmps)
		{
			// Read "*.lp" File
			Vector lulv;
			for(c=0; c<n; c++){
				//Image Name
				ReadPhrase(file,&line);
				nameArr[c] = line;
				//print(line);
				//Lu
				ReadPhrase(file,&line);
				lulv.x = line.StringToReal();	
				//Lv
				ReadPhrase(file,&line);
				lulv.y = line.StringToReal();
				//Lz
				ReadPhrase(file,&line);
				lulv.z = line.StringToReal();
				lulvArr[ c] =  lulv;
			}

			Filename mmtf;
			for(c=0; c<n; c++){
				bmps[c] = BaseBitmap::Alloc();
				if(bmps[c]){
					//From Array
					mmtf = name;
					mmtf.SetFile(nameArr[c]);
					if (!bmps[c]->Init(mmtf)){ print("Open Error ",c); }
				}
			}
			//Create Result Name
			Filename res_name = name;
			bDelete(nameArr); //Free

			// Init Normal Calculation
			LONG norm_n = n;
			//norm_n = 3; //TEST ONLY

			MatrixT<LReal>  Vn(norm_n,norm_n);
			VectorT<LReal>  Sn(norm_n);
			// Init Normal Matrix
			MatrixT<LReal>  NM(3,norm_n);
			MatrixT<LReal>  iNM(norm_n,3); //Inverse Matrix of U 
			//Init Normal Matrix
			for(c=0; c<norm_n; c++){
				lulv = lulvArr[c];

				NM(0,c) = lulv.x;
				NM(1,c) = lulv.y;
				NM(2,c) = lulv.z;
			}
			//Invert Normal Matrix
			Bool res1 = Svd2(NM,Sn,Vn);
			FixSvd(Sn,NM,Vn);
			Pinv(iNM,NM,Sn,Vn); //P Inverse


			// Temp Matrizen
			MatrixT<LReal>  V(n,n);
			VectorT<LReal>  S(n);
			// Init Polynomenal Matrix
			MatrixT<LReal>  LM(6,n);
			MatrixT<LReal>  iLM(n,6); //Inverse Matrix of U 
			//Init LuLv Matrix
			for(c=0; c<n; c++){
				lulv = lulvArr[c];
				Real lu = lulv.x;
				Real lv = lulv.y;
				Real lw = lulv.z;
				//Real lw = Sqrt(1.0-lu*lu-lv*lv); //TEST ???

				LM(0,c) = lu*lu;
				LM(1,c) = lv*lv;
				LM(2,c) = lu*lv;
				LM(3,c) = lu;
				LM(4,c) = lv;
				LM(5,c) = 1.0; //PTM_1.2
				//LM(5,c) = lw; //PTM_RE //TEST ?????
			}
			//Invert Polynomenal Matrix
			Bool res2 = Svd2(LM,S,V);
			FixSvd(S,LM,V);
			Pinv(iLM,LM,S,V); //P Inverse

			//DO
			UWORD r,g,b;
			VectorT<LReal>  COEFF(6);
			VectorT<LReal>  NORM(3);
			VectorT<LReal>  LUM(n);
			VectorT<LReal>  LUMn(norm_n);

			LONG xres = 0; 
			if(bmps[0]) xres = bmps[0]->GetBw();
			LONG yres = 0; 
			if(bmps[0]) yres = bmps[0]->GetBh();

			//Init Maps
			t_rgb->Init(xres,yres);
			t_a012->Init(xres,yres);
			t_a345->Init(xres,yres);
			t_normal->Init(xres,yres);

			//Real aveLum;
			Real	lum;
			Vector	rgb,nrgb;
			Vector	hsv;
			Vector	tt012;
			Vector	tt345;
			Vector	norm;

			MinMax  lummm;
			lummm.Init();

			tim.PrintS("Init ");
			tim.Start();

			//TEMP
			TMap2d<Vector>	rgb_temp;
			rgb_temp.Init(xres,yres);

			StatusSetText("Calculate Color Map:");
			// Calculate Average Color
			for(LONG y=0; y<yres; y++){
				StatusSetBar(y*100 / yres); //TEST
				for(LONG x=0; x<xres; x++){
					nrgb = 0.0;
					for(c=0; c<n; c++) 
					{
						bmps[c]->GetPixel(x,y,&r,&g,&b);
						rgb.x = Real(r)/256.0;
						rgb.y = Real(g)/256.0;
						rgb.z = Real(b)/256.0;

						rgbArr[c] = rgb;
						nrgb += rgb;
					}
					nrgb = nrgb / Real(n);

					rgb_temp.SetPixel(x,y,nrgb);
				}
			}

			tim.PrintS("Colors ");
			tim.Start();

			Vector rgb_average = rgb_temp.CalcAverage();
			Vector rgb_max = rgb_temp.CalcMax();
			Vector rgb_min = rgb_temp.CalcMin();

			Real cr_max = Max(Max(rgb_max.x,rgb_max.y),rgb_max.z);
			Vector rgb_scale = 1.0 / cr_max;
			
			rgb_temp.Scale(rgb_scale); //???

			//print("rgb_average ",rgb_average);
			//print("rgb_max ",rgb_max);
			//print("rgb_min ",rgb_min);
			//print("rgb_scale ",rgb_scale);

			Vector n_rgb_max = rgb_temp.CalcMax();
			Vector n_rgb_min = rgb_temp.CalcMin();
			//print("n_rgb_max ",n_rgb_max);
			//print("n_rgb_min ",n_rgb_min);

			StatusSetText("Calculate PTM Maps:");
			// Calculate (DO FASTRER !!)
			for(LONG y=0; y<yres; y++)
			{
				StatusSetBar(y*100 / yres); //TEST
				for(LONG x=0; x<xres; x++)
				{
					nrgb = rgb_temp.GetPixel(x,y);//nrgb = Cut(nrgb,0.0,1.0); //???????????
					// Luminance
					for(c=0; c<n; c++){
						//Get Akt Color
						bmps[c]->GetPixel(x,y,&r,&g,&b);
						rgb.x = Real(r)/256.0;
						rgb.y = Real(g)/256.0;
						rgb.z = Real(b)/256.0;

						//Methode 1 ->
						rgb = rgb / nrgb; //OK 1
						lum = VectorGray(rgb);	//OK 1

						lummm.AddPoint(rgb);			
						LUM[c] = lum;

						if(c<norm_n){
							if(lum<0.2) lum = 0.2;
							//else if(lum>0.8) lum = VectorGray(nrgb);
							LUMn[c] = lum;//Normal
						}
					}
					Mult(iLM,LUM,COEFF);

					tt012.x = COEFF[0];
					tt012.y = COEFF[1];
					tt012.z = COEFF[2];
					tt345.x = COEFF[3];
					tt345.y = COEFF[4];
					tt345.z = COEFF[5];

					t_rgb->SetPixel(x,y,nrgb);
					t_a012->SetPixel(x,y,tt012);
					t_a345->SetPixel(x,y,tt345);

					//Normal
					Mult(iNM,LUMn,NORM);
					norm.x = NORM[0];
					norm.y = NORM[1];
					norm.z = NORM[2];
					norm = !norm;
					t_normal->SetPixel(x,y,norm);
				}
			}// Calculate

			tim.PrintS("Calc ");
			tim.Start();

			//SAVE
			if(sb_modus==1){
				Vector min012 = t_a012->CalcMin();
				Vector min345 = t_a345->CalcMin();
				Vector max012 = t_a012->CalcMax();
				Vector max345 = t_a345->CalcMax();

				if(min345.z>0.0) min345.z = 0.0; //???

				//Auto Scale
				t_scale012 = max012 - min012;
				t_scale345 = max345 - min345;

				//Auto Bias
				t_bias012 = -(min012 / t_scale012);
				t_bias345 = -(min345 / t_scale345);

				//data->SetVector(XPTG_SCALE_012,t_scale012);
				//data->SetVector(XPTG_SCALE_345,t_scale345);		
				//data->SetVector(XPTG_BIAS_012,t_bias012);		
				//data->SetVector(XPTG_BIAS_345,t_bias345);	

				//print("Scale012 ",t_scale012);
				//print("Scale345 ",t_scale345);
				//print("Bias012 ",t_bias012);
				//print("Bias345 ",t_bias345);
			}else if(sb_modus==2){
				//Default Scale
				t_scale012 = Vector(2.0,2.0,2.0);
				t_scale345 = Vector(2.0,2.0,2.0);
				//Default Bias
				t_bias012 = Vector(0.65,0.65,0.55);
				t_bias345 = Vector(0.60,0.60,0.0);

				//data->SetVector(XPTG_SCALE_012,t_scale012);
				//data->SetVector(XPTG_SCALE_345,t_scale345);		
				//data->SetVector(XPTG_BIAS_012,t_bias012);		
				//data->SetVector(XPTG_BIAS_345,t_bias345);
			}
			
			//name.SetFile(String("GenTest1"));
			//name.SetFile(String(res_name));
			res_name.SetSuffix("ptm");

			WriteLRGB(res_name,*t_rgb,*t_a012,*t_a345,t_scale012,t_scale345,t_bias012,t_bias345);
		
	
		}

		//Free
		for(LONG c=0; c<n; c++){
			BaseBitmap::Free(bmps[c]);
		}
		GeFree(bmps);
		GeFree(rgbArr);
		GeFree(lulvArr);
		GeFree(indArr);

		tim.PrintS("PTM Ready ! ");
		StatusSetText("PTM Ready ! ");
		StatusClear();

		iscalc = TRUE;

		t_rgb=t_a012=t_a345=t_normal= NULL; //???
	}
	calculating = FALSE;
	// <<<<<<<<<<<<<< CREATE PTM <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}

//#################################################################
class PTMgenShaderData : public ShaderData
{
		INSTANCEOF(PTMgenShaderData,ShaderData)
	private:
		Filename		name;
		Real			bias;
		Real			scale;

		LONG			part;
		Bool			iscalc;

		TMap2d<Vector>	 t_rgb;
		TMap2d<Vector>	 t_a012;
		TMap2d<Vector>	 t_a345;
		TMap2d<Vector>	 t_normal;

		Vector			t_scale012;
		Vector			t_scale345;
		Vector			t_bias012;
		Vector			t_bias345;

		MinMax			minmax_rgb;
		MinMax			minmax1;
		MinMax			minmax2;
		MinMax			minmax3;

		LONG			sb_modus;
		Vector			min012;
		Vector			min345;
		Vector			max012;
		Vector			max345;

		PTM_Thread		ptm_calculator;
	public:
		////inherited from NodeData////
		virtual Bool Init(GeListNode *node);
		virtual void Free(GeListNode *node);
		//virtual Bool Read(GeListNode *node, HyperFile *hf, LONG level);
		//virtual Bool Write(GeListNode *node, HyperFile *hf);
		virtual Bool Message(GeListNode *node, LONG type, void *data);
		virtual Bool CopyTo(NodeData *dest, GeListNode *snode, GeListNode *dnode, LONG flags, AliasTrans *trn);
		//virtual Bool GetDDescription(GeListNode *node, Description *description,LONG &flags);
		//virtual Bool GetDParameter(GeListNode *node, const DescID &id,GeData &t_data,LONG &flags);
		virtual Bool GetDEnabling(GeListNode *node, const DescID &id,GeData &t_data,LONG flags,const BaseContainer *itemdesc);
		//virtual Bool SetDParameter(GeListNode *node, const DescID &id,const GeData &t_data,LONG &flags);
		//static NodeData *Alloc(void) { return gNew MyNodeData; }
		////inherited from NodeData////

		//virtual	Bool Draw(PluginShader *sh, BaseObject *op, BaseTag *tag, BaseDraw *bd, BaseDrawHelp *bh);
		virtual	LONG GetRenderInfo(PluginShader *sh);
		virtual	LONG InitRender(PluginShader *sh, InitRenderStruct *irs);
		virtual	Vector Output(PluginShader *sh, ChannelData *cd);
		virtual	void FreeRender(PluginShader *sh);
		//
		static NodeData *Alloc(void) { return gNew PTMgenShaderData; }
};
//########################## Init ###############################
Bool PTMgenShaderData::Init(GeListNode *node)
{
	// initialize settings
	PluginShader	  *op	= (PluginShader*)node;
	BaseContainer *data = op->GetDataInstance();

	data->SetVector(XPTG_SCALE_012,Vector(2.0,2.0,2.0));
	data->SetVector(XPTG_SCALE_345,Vector(2.0,2.0,2.0));		

	data->SetVector(XPTG_BIAS_012,Vector(0.65,0.65,0.55));		
	data->SetVector(XPTG_BIAS_345,Vector(0.60,0.60,0.0));

	iscalc = TRUE;

	//...
	return TRUE;
}
//########################## Free ###############################
void PTMgenShaderData::Free(GeListNode *node)
{
	//...
}
//#################################################################
LONG PTMgenShaderData::GetRenderInfo(PluginShader *sh)
{
	LONG res = 0;

	return SHADER_DUDVREQUIRED;
}
//########################## CopyTo ###############################
Bool PTMgenShaderData::CopyTo(NodeData *dest, GeListNode *snode, GeListNode *dnode, LONG flags, AliasTrans *trn)
{	
	PTMgenShaderData *dData = (PTMgenShaderData*)dest;
	//dData->iscalc = iscalc;
	dData->iscalc = TRUE;

	t_rgb.CopyTo(dData->t_rgb);
	t_a012.CopyTo(dData->t_a012);
	t_a345.CopyTo(dData->t_a345);
	t_normal.CopyTo(dData->t_normal);

	dData->t_scale012 = t_scale012;
	dData->t_scale345 = t_scale345;
	dData->t_bias012 = t_bias012;
	dData->t_bias345 = t_bias345;

	return TRUE;
}
//########################## Message ###############################
Bool PTMgenShaderData::Message(GeListNode *node, LONG type, void *data)
{
	PluginShader  *sh	= (PluginShader*)node;
	BaseContainer *bdata = sh->GetDataInstance();

	if (type==MSG_DESCRIPTION_COMMAND)
	{
		DescriptionCommand *dc = (DescriptionCommand*) data;
		if (dc->id[0].id==XPTG_RECALCULATE){

		if(ptm_calculator.SetData(sb_modus,name,iscalc,&t_rgb,&t_a012,&t_a345,&t_normal))
		{
			ptm_calculator.Start(TRUE);
		}
		iscalc = ptm_calculator.GetData(t_scale012,t_scale345,t_bias012,t_bias345);
			//iscalc = FALSE;
			//print("GGG");
		}
	}
	//...
	return TRUE;
}
//########################## GetDEnabling ###############################
Bool PTMgenShaderData::GetDEnabling(GeListNode *node, const DescID &id,GeData &t_data,LONG flags,const BaseContainer *itemdesc)
{
	BaseContainer &data = *((PluginShader*)node)->GetDataInstance();
	switch (id[0].id)
	{
		case XPTG_SCALE_012:	
		case XPTG_SCALE_345:	
		case XPTG_BIAS_012:	
		case XPTG_BIAS_345:	
			return data.GetLong(XPTG_SCALE_BIAS_MD)==0;
	}
	return SUPER::GetDEnabling(node, id, t_data, flags, itemdesc);
}

//#################################################################
LONG PTMgenShaderData::InitRender(PluginShader *sh, InitRenderStruct *irs)
{
	/*
		Plane plane(Vector(1,1,1),0.5);
		plane.Normalize();

		Vector col = Vector(0.0,0.99,0.99);
		LONG site = plane.WhichSide(col);
		print(site);

		Vector ppp = plane.MapToPlane(Vector(1,1,1),col);
		print(ppp);
	*/

	//Thread


	BaseContainer *data = sh->GetDataInstance();
	name = data->GetFilename(XPTG_FILE_NAME); if(!name.Content()) return LOAD_NOTFOUND;
	part = data->GetLong(XPTG_PART);
	sb_modus = data->GetLong(XPTG_SCALE_BIAS_MD);

	t_scale012 = data->GetVector(XPTG_SCALE_012);
	t_scale345 = data->GetVector(XPTG_SCALE_345);		
	t_bias012 = data->GetVector(XPTG_BIAS_012);		
	t_bias345 = data->GetVector(XPTG_BIAS_345);	

	minmax_rgb.Init();
	minmax1.Init();
	minmax2.Init();
	minmax3.Init();

	/*
	//TEST
	if(ptm_calculator.SetData(sb_modus,name,iscalc,&t_rgb,&t_a012,&t_a345,&t_normal))
	{
		ptm_calculator.Start(TRUE);
	}
	*/
	iscalc = ptm_calculator.GetData(t_scale012,t_scale345,t_bias012,t_bias345);
	data->SetVector(XPTG_SCALE_012,t_scale012);
	data->SetVector(XPTG_SCALE_345,t_scale345);		
	data->SetVector(XPTG_BIAS_012,t_bias012);		
	data->SetVector(XPTG_BIAS_345,t_bias345);

	/*
	// CREATE PTM >>>>>>>>>>>>>>
	if(!iscalc){
		print("------ PTM Create ------");
		OTimer tim;
		String line;

		AutoAlloc<BaseFile>	file; if (!file) return LOAD_NOMEM;
		if (!file->Open(name,GE_READ,FILE_NODIALOG,GE_MOTOROLA)) return LOAD_NOMEM;

		LONG n = 13; //TEST ???????????
		ReadPhrase(file,&line);	n  = line.StringToReal();
		if(n<6) {  print("Not enough images to create PTM! ",n);  return LOAD_NOMEM; }

		LONG	c;
		//String	res_name = "Result",comp_name;
		String	*nameArr = bNew String[n+1]; if(!nameArr) return LOAD_NOMEM;
		Vector	*lulvArr = (Vector*)GeAlloc(n*sizeof(Vector)); if(!lulvArr) return LOAD_NOMEM;
		Vector	*rgbArr = (Vector*)GeAlloc(n*sizeof(Vector)); if(!rgbArr) return LOAD_NOMEM;

		LONG *indArr = (LONG *)GeAlloc(n*sizeof(LONG )); if(!indArr) return LOAD_NOMEM;

		// Read "*.lp" File
		Vector lulv;
		for(c=0; c<n; c++){
			//Image Name
			ReadPhrase(file,&line);
			nameArr[c] = line;
			//print(line);
			//Lu
			ReadPhrase(file,&line);
			lulv.x = line.StringToReal();	
			//Lv
			ReadPhrase(file,&line);
			lulv.y = line.StringToReal();
			//Lz
			ReadPhrase(file,&line);
			lulv.z = line.StringToReal();
			lulvArr[ c] =  lulv;
		}

		Filename mmtf;
		BaseBitmap **bmps = (BaseBitmap**)GeAlloc(n*sizeof(BaseBitmap*)); if(!bmps) return LOAD_NOMEM;
		for(c=0; c<n; c++){
			bmps[c] = BaseBitmap::Alloc();
			if(bmps[c]){
				//From Array
				mmtf = name;
				mmtf.SetFile(nameArr[c]);
				if (!bmps[c]->Init(mmtf)){ print("Open Error ",c); }
			}
		}
		//Create Result Name
		Filename res_name = name;
		bDelete(nameArr); //Free

		// Init Normal Calculation
		LONG norm_n = n;
		//norm_n = 3; //TEST ONLY

		MatrixT<LReal>  Vn(norm_n,norm_n);
		VectorT<LReal>  Sn(norm_n);
		// Init Normal Matrix
		MatrixT<LReal>  NM(3,norm_n);
		MatrixT<LReal>  iNM(norm_n,3); //Inverse Matrix of U 
		//Init Normal Matrix
		for(c=0; c<norm_n; c++){
			lulv = lulvArr[c];

			NM(0,c) = lulv.x;
			NM(1,c) = lulv.y;
			NM(2,c) = lulv.z;
		}
		//Invert Normal Matrix
		Bool res1 = Svd2(NM,Sn,Vn);
		FixSvd(Sn,NM,Vn);
		Pinv(iNM,NM,Sn,Vn); //P Inverse


		// Temp Matrizen
		MatrixT<LReal>  V(n,n);
		VectorT<LReal>  S(n);
		// Init Polynomenal Matrix
		MatrixT<LReal>  LM(6,n);
		MatrixT<LReal>  iLM(n,6); //Inverse Matrix of U 
		//Init LuLv Matrix
		for(c=0; c<n; c++){
			lulv = lulvArr[c];
			Real lu = lulv.x;
			Real lv = lulv.y;
			Real lw = lulv.z;
			//Real lw = Sqrt(1.0-lu*lu-lv*lv); //TEST ???

			LM(0,c) = lu*lu;
			LM(1,c) = lv*lv;
			LM(2,c) = lu*lv;
			LM(3,c) = lu;
			LM(4,c) = lv;
			LM(5,c) = 1.0; //PTM_1.2
			//LM(5,c) = lw; //PTM_RE //TEST ?????
		}
		//Invert Polynomenal Matrix
		Bool res2 = Svd2(LM,S,V);
		FixSvd(S,LM,V);
		Pinv(iLM,LM,S,V); //P Inverse

		//DO
		UWORD r,g,b;
		VectorT<LReal>  COEFF(6);
		VectorT<LReal>  NORM(3);
		VectorT<LReal>  LUM(n);
		VectorT<LReal>  LUMn(norm_n);

		LONG xres = 0; 
		if(bmps[0]) xres = bmps[0]->GetBw();
		LONG yres = 0; 
		if(bmps[0]) yres = bmps[0]->GetBh();

		t_rgb.Init(xres,yres);
		t_a012.Init(xres,yres);
		t_a345.Init(xres,yres);
		t_normal.Init(xres,yres);

		//Real aveLum;
		Real	lum;
		Vector	rgb,nrgb;
		Vector	hsv;
		Vector	tt012;
		Vector	tt345;
		Vector	norm;

		MinMax  lummm;
		lummm.Init();

		tim.PrintS("Init ");
		tim.Start();

		//TEMP
		TMap2d<Vector>	rgb_temp;
		rgb_temp.Init(xres,yres);

		// Calculate Average Color
		for(LONG y=0; y<yres; y++){
			for(LONG x=0; x<xres; x++){
				nrgb = 0.0;
				for(c=0; c<n; c++) 
				{
					bmps[c]->GetPixel(x,y,&r,&g,&b);
					rgb.x = Real(r)/256.0;
					rgb.y = Real(g)/256.0;
					rgb.z = Real(b)/256.0;

					rgbArr[c] = rgb;
					nrgb += rgb;
				}
				nrgb = nrgb / Real(n);

				rgb_temp.SetPixel(x,y,nrgb);
			}
		}

		tim.PrintS("Colors ");
		tim.Start();

		Vector rgb_average = rgb_temp.CalcAverage();
		Vector rgb_max = rgb_temp.CalcMax();
		Vector rgb_min = rgb_temp.CalcMin();

		Real cr_max = Max(Max(rgb_max.x,rgb_max.y),rgb_max.z);
		Vector rgb_scale = 1.0 / cr_max;
		
		rgb_temp.Scale(rgb_scale); //???

		print("rgb_average ",rgb_average);
		print("rgb_max ",rgb_max);
		print("rgb_min ",rgb_min);
		print("rgb_scale ",rgb_scale);

		Vector n_rgb_max = rgb_temp.CalcMax();
		Vector n_rgb_min = rgb_temp.CalcMin();
		print("n_rgb_max ",n_rgb_max);
		print("n_rgb_min ",n_rgb_min);


		// Calculate (DO FASTRER !!)
		for(LONG y=0; y<yres; y++)
		{
			for(LONG x=0; x<xres; x++)
			{
				nrgb = rgb_temp.GetPixel(x,y);//nrgb = Cut(nrgb,0.0,1.0); //???????????
				// Luminance
				for(c=0; c<n; c++){
					//Get Akt Color
					bmps[c]->GetPixel(x,y,&r,&g,&b);
					rgb.x = Real(r)/256.0;
					rgb.y = Real(g)/256.0;
					rgb.z = Real(b)/256.0;

					//Methode 1 ->
					rgb = rgb / nrgb; //OK 1
					lum = VectorGray(rgb);	//OK 1

					lummm.AddPoint(rgb);			
					LUM[c] = lum;

					if(c<norm_n){
						if(lum<0.2) lum = 0.2;
						//else if(lum>0.8) lum = VectorGray(nrgb);
						LUMn[c] = lum;//Normal
					}
				}
				Mult(iLM,LUM,COEFF);

				tt012.x = COEFF[0];
				tt012.y = COEFF[1];
				tt012.z = COEFF[2];
				tt345.x = COEFF[3];
				tt345.y = COEFF[4];
				tt345.z = COEFF[5];

				t_rgb.SetPixel(x,y,nrgb);
				t_a012.SetPixel(x,y,tt012);
				t_a345.SetPixel(x,y,tt345);

				//Normal
				Mult(iNM,LUMn,NORM);
				norm.x = NORM[0];
				norm.y = NORM[1];
				norm.z = NORM[2];
				norm = !norm;
				t_normal.SetPixel(x,y,norm);
			}
		}// Calculate

		tim.PrintS("Calc ");
		//DO
		//print("Lum Max ",lummm.GetMax());
		//print("Lum Min ",lummm.GetMin());
		//print("RGB Ave ",t_rgb.CalcAverage());
		//print("012 Ave ",t_a012.CalcAverage());
		//print("345 Ave ",t_a345.CalcAverage());
		//print("RGB min ",t_rgb.CalcMin());
		//print("RGB max ",t_rgb.CalcMax());
		//print("A012 min ",t_a012.CalcMin());
		//print("A012 max ",t_a012.CalcMax());
		//print("A345 min ",t_a345.CalcMin());
		//print("A345 max ",t_a345.CalcMax());

		//SAVE
		if(sb_modus==1){
			Vector min012 = t_a012.CalcMin();
			Vector min345 = t_a345.CalcMin();
			Vector max012 = t_a012.CalcMax();
			Vector max345 = t_a345.CalcMax();

			if(min345.z>0.0) min345.z = 0.0; //???

			//Auto Scale
			t_scale012 = max012 - min012;
			t_scale345 = max345 - min345;

			//Auto Bias
			t_bias012 = -(min012 / t_scale012);
			t_bias345 = -(min345 / t_scale345);

			data->SetVector(XPTG_SCALE_012,t_scale012);
			data->SetVector(XPTG_SCALE_345,t_scale345);		
			data->SetVector(XPTG_BIAS_012,t_bias012);		
			data->SetVector(XPTG_BIAS_345,t_bias345);	

			print("Scale012 ",t_scale012);
			print("Scale345 ",t_scale345);
			print("Bias012 ",t_bias012);
			print("Bias345 ",t_bias345);
		}else if(sb_modus==2){
			//Default Scale
			t_scale012 = Vector(2.0,2.0,2.0);
			t_scale345 = Vector(2.0,2.0,2.0);
			//Default Bias
			t_bias012 = Vector(0.65,0.65,0.55);
			t_bias345 = Vector(0.60,0.60,0.0);

			data->SetVector(XPTG_SCALE_012,t_scale012);
			data->SetVector(XPTG_SCALE_345,t_scale345);		
			data->SetVector(XPTG_BIAS_012,t_bias012);		
			data->SetVector(XPTG_BIAS_345,t_bias345);
		}
		
		//name.SetFile(String("GenTest1"));
		//name.SetFile(String(res_name));
		res_name.SetSuffix("ptm");
		WriteLRGB(res_name,t_rgb,t_a012,t_a345,t_scale012,t_scale345,t_bias012,t_bias345);
		
//ERROR:
		//Free
		for(LONG c=0; c<n; c++){
			BaseBitmap::Free(bmps[c]);
		}
		GeFree(bmps);
		GeFree(rgbArr);
		GeFree(lulvArr);
		GeFree(indArr);

		iscalc = TRUE;
	}
	// <<<<<<<<<<<<<< CREATE PTM <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	*/

	return LOAD_OK;
}
//#################################################################
Vector PTMgenShaderData::Output(PluginShader *sh, ChannelData *cd)
{
	if(!iscalc) return 0.5;

	Vector  uv = Repeat(cd->p,1.0);
	//TEST
	Vector res;
	if(part==0){
		if(cd->vd)
		{
			VolumeData *vd = cd->vd;

			Vector	pos = vd->p;
			Vector	normal = vd->n;	
			Vector	tangente;
			Vector	binormal;

			//GetDUDV(TexData *tex, const Vector &p, const Vector &phongn, const Vector &orign, LONG global_id, Bool forceuvw, Vector *ddu, Vector *ddv)
			vd->GetDUDV(vd->tex,pos,normal,normal,0,FALSE,&tangente,&binormal); //Remo: 05.05.2007
			
			//vd->GetDUDV(vd->tex,pos,normal,&tangente,&binormal);
			binormal = -binormal; //???

			Vector rgb      = t_rgb.GetBicubicCol(uv);
			Vector coeff012 = t_a012.GetBicubicCol(uv);
			Vector coeff345 = t_a345.GetBicubicCol(uv);

			Vector  diff_col = 0.0;
			Vector  lu2_lv2_lulv;

			LONG	lightCount = vd->GetLightCount();
			// Lights
			for(LONG c=0; c<lightCount; c++){
				RayLight *vLight = vd->GetLight(c); //Light Info

				// DEFUSE >>>>>>>>>>>>>>>>>>>>>>
				Vector lightDir = !(vLight->m.off - pos);// Normalize light direction
				// Transform light direction to Normal Space
				Vector lightTBN = Vector(lightDir*tangente,lightDir*binormal,lightDir*normal);

				// z-extrapolation
				Real zz = lightTBN.z;
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
				Real diff = Dot(lu2_lv2_lulv, coeff012) + Dot(lightTBN, coeff345);
				if(diff<0.0) diff=0.0;
				diff_col += diff * vLight->color;
			}
			return diff_col ^ rgb;
		}else{
			res = t_rgb.GetBicubicCol(uv);
			minmax_rgb.AddPoint(res);
			return res;
		}

	}else if(part==1){
		//Light Only
		if(cd->vd)
		{
			VolumeData *vd = cd->vd;

			Vector	pos = vd->p;
			Vector	normal = vd->n;	
			Vector	tangente;
			Vector	binormal;

			vd->GetDUDV(vd->tex,pos,normal,normal,0,FALSE,&tangente,&binormal); //Remo: 05.05.2007
			//vd->GetDUDV(vd->tex,pos,normal,&tangente,&binormal);
				binormal = -binormal; //???

			//Vector rgb      = t_rgb.GetBicubicCol(uv);
			Vector coeff012 = t_a012.GetBicubicCol(uv);
			Vector coeff345 = t_a345.GetBicubicCol(uv);

			Vector  diff_col = 0.0;
			Vector  lu2_lv2_lulv;

			LONG	lightCount = vd->GetLightCount();
			// Lights
			for(LONG c=0; c<lightCount; c++){
				RayLight *vLight = vd->GetLight(c); //Light Info

				// DEFUSE >>>>>>>>>>>>>>>>>>>>>>
				Vector lightDir = !(vLight->m.off - pos);// Normalize light direction
				// Transform light direction to Normal Space
				Vector lightTBN = Vector(lightDir*tangente,lightDir*binormal,lightDir*normal);

				// z-extrapolation
				Real zz = lightTBN.z;
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
				Real diff = Dot(lu2_lv2_lulv, coeff012) + Dot(lightTBN, coeff345);
				if(diff<0.0) diff=0.0;
				diff_col += diff * vLight->color;
			}
			return diff_col;
		}else{
			Vector coeff012 = t_a012.GetBicubicCol(uv);
			Vector coeff345 = t_a345.GetBicubicCol(uv);
			
			Vector  lightTBN = Vector(0.1,0.1,0.8);
			Vector  lu2_lv2_lulv;

			// Prepare higher-order terms
			lu2_lv2_lulv.x = lightTBN.x * lightTBN.x; //lu^2
			lu2_lv2_lulv.y = lightTBN.y * lightTBN.y; //lv^2
			lu2_lv2_lulv.z = lightTBN.x * lightTBN.y; //lu*lv

				// Evaluate polynomial
			Real diff = Dot(lu2_lv2_lulv, coeff012) + Dot(lightTBN, coeff345);
			if(diff<0.0) diff=0.0;

			return diff;
		}
	}else if(part==2){
		//RGB
		res = t_rgb.GetBicubicCol(uv);
		minmax_rgb.AddPoint(res);
		return res;
	}else if(part==3){
		// A012
		res = t_a012.GetBicubicCol(uv);
		minmax1.AddPoint(res);
		res = (res / t_scale012)+t_bias012;

		//res = Cut(res,0.0,1.0); //CUT
		return res;
	}else if(part==4){ 
		// A345
		res = t_a345.GetBicubicCol(uv);
		minmax2.AddPoint(res);
		res = (res / t_scale345)+t_bias345;

		//res = Cut(res,0.0,1.0); //CUT
		return res;

	}else if(part==5){
		//Normal
		Vector norm = t_normal.GetBicubicCol(uv);
		minmax3.AddPoint(norm);

		return (norm+1.0)*0.5;
	}else if(part==6){
		//Normal
		Vector coeff012 = t_a012.GetBicubicCol(uv);//*0.5+0.5;
		Vector coeff345 = t_a345.GetBicubicCol(uv);//*0.5+0.5;

		Vector norm = CalcNormal(coeff012,coeff345);
		minmax3.AddPoint(norm);

		return (norm+1.0)*0.5;
	}


	return 0.0;
}
//#################################################################
void PTMgenShaderData::FreeRender(PluginShader *sh)
{
	/*
	if(part==1){
		print("Rgb Max ",minmax_rgb.GetMax());
		print("Rgb Min ",minmax_rgb.GetMin());
	}else if(part==2){
		print("012 Max ",minmax1.GetMax());
		print("012 Min ",minmax1.GetMin());
	}else if(part==3){
		print("345 Max ",minmax2.GetMax());
		print("345 Min ",minmax2.GetMin());
	}else if(part==4 || part==5){
		print("Norm Max ",minmax3.GetMax());
		print("Norm Min ",minmax3.GetMin());
	}
	*/
}
//#################################################################
Bool RegisterPTMgenShader(void)
{
	String name=GeLoadString(IDS_PTMGEN); if (!name.Content()) return TRUE;
	return RegisterShaderPlugin(DI_PTMGEN_SHADER_ID,name,0,PTMgenShaderData::Alloc,"XPTMgen",DISKLEVEL);
}
