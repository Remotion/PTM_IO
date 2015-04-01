/////////////////////////////////////////////////////////////
// Tag Plugin  PTMTag
/////////////////////////////////////////////////////////////
// Date: 06.11.2004 Time: 19:06:04
/////////////////////////////////////////////////////////////
// (c) 2004 REMOTION, all rights reserved 
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "re_print.h"
#include "c4d_symbols.h"
#include "TPTMTag.h"

#include "PresetTools.h"
#include "Container\re_TMap2d.h"

#include "MathTools03.h"
#include "re_exVector.h"
#include "Optimize.h"

#include "Container\VectorT.h"
#include "Container\MatrixT.h"

#include "..\PTM_Types.h"
#include "..\PTM_IO.h"

#define DI_PTMTAG_TAG_ID 1015750

//#################################################################
class PTMTagData : public TagData
{
		INSTANCEOF(PTMTagData,TagData)
	private:
		Bool is_writen;
	public:
		////inherited from NodeData////
		virtual Bool Init(GeListNode *node);
		virtual void Free(GeListNode *node);
		//virtual Bool Read(GeListNode *node, HyperFile *hf, LONG level);
		//virtual Bool Write(GeListNode *node, HyperFile *hf);
		//virtual Bool Message(GeListNode *node, LONG type, void *data);
		//virtual Bool CopyTo(NodeData *dest, GeListNode *snode, GeListNode *dnode, LONG flags, AliasTrans *trn);
		//virtual Bool GetDDescription(GeListNode *node, Description *description,LONG &flags);
		//virtual Bool GetDParameter(GeListNode *node, const DescID &id,GeData &t_data,LONG &flags);
		virtual Bool GetDEnabling(GeListNode *node, const DescID &id,GeData &t_data,LONG flags,const BaseContainer *itemdesc);
		//virtual Bool SetDParameter(GeListNode *node, const DescID &id,const GeData &t_data,LONG &flags);
		//static NodeData *Alloc(void) { return gNew MyNodeData; }
		////inherited from NodeData////

		virtual Bool Draw(PluginTag *tag, BaseObject *op, BaseDraw *bd, BaseDrawHelp *bh);
		virtual LONG Execute(PluginTag *tag, BaseDocument *doc, BaseObject *op, BaseThread *bt, LONG priority, LONG flags);
		//virtual Bool AddToExecution(PluginTag *tag, PriorityList *list);
		//
		static NodeData *Alloc(void) { return gNew PTMTagData; }
};
//########################## Init ###############################
Bool PTMTagData::Init(GeListNode *node)
{
	// initialize settings
	BaseTag	  *op	= (BaseTag*)node;
	BaseContainer *data = op->GetDataInstance();

	data->SetString(PTAG_FILE_NAME,"PTM_Images");
	data->SetString(PTAG_FILE_EXT,".tga");
	//PTMTAGData::Init(data);
	is_writen = FALSE;
	//...
	return TRUE;
}
//########################## Free ###############################
void PTMTagData::Free(GeListNode *node)
{
	//...
}
//########################## GetDEnabling ###############################
Bool PTMTagData::GetDEnabling(GeListNode *node, const DescID &id,GeData &t_data,LONG flags,const BaseContainer *itemdesc)
{
	BaseContainer &data = *((BaseTag*)node)->GetDataInstance();
	switch (id[0].id)
	{
		case 0: break;
	}
	return SUPER::GetDEnabling(node, id, t_data, flags, itemdesc);
}

//#################################################################
Bool PTMTagData::Draw(PluginTag *tag, BaseObject *op, BaseDraw *bd, BaseDrawHelp *bh)
{
	

	return TRUE;
}

//#################################################################
LONG PTMTagData::Execute(PluginTag *tag, BaseDocument *doc, BaseObject *op, BaseThread *bt, LONG priority, LONG flags)
{
	BaseContainer *data = tag->GetDataInstance();

	BaseObject *light = op->GetDown(); if(!light) return EXECUTION_RESULT_OK;
	
	PolygonObject *poly = NULL;
	PolygonObject *top = NULL;
	if(!op->IsInstanceOf(Opolygon)) poly = (PolygonObject*)op->GetCache();
	else poly = (PolygonObject*)op;
	if(poly) top = (PolygonObject*)poly->GetDeformCache();
	if(top) poly = top;
	if(poly)
	{
		LONG	lcnt = poly->GetPointCount(); if(lcnt < 8) return EXECUTION_RESULT_OK;
		Vector	*padr = poly->GetPoint(); 

		//LONG	lcnt = data->GetLong(PTAG_LIGHTS_COUNT);
		String	mainname = data->GetString(PTAG_FILE_NAME);
		String  ext = data->GetString(PTAG_FILE_EXT);
		Vector	lulv;
		Vector	pos = 0.0;

		//------------Frame-----------
		BaseTime	aktTime = doc->GetTime();
		LONG		frame = aktTime.GetFrame(doc->GetFps());

		BaseDraw	*bd = doc->GetRenderBaseDraw(); 
		//Matrix 
		Matrix vmg = bd->GetMg(); //?????????
		Matrix ivmg = bd->GetMi(); //?????????
		ivmg.off = 0.0; //?????????
		ivmg.v2 = -ivmg.v2; //?????????
			
		pos = padr[frame % lcnt];
		//Set Light Pos
		light->SetPos(pos);

		//Write "*.lp" File:
		//if(!is_writen)
		if(frame==0) //TEST ONLY
		{
			Filename name = doc->GetDocumentPath();
			name += doc->GetDocumentName();// + String("_pt");
			name.SetSuffix("lp");
			//print(name.GetString());//???

			String str;

			AutoAlloc<BaseFile>	file; if (!file) return EXECUTION_RESULT_MEMORYERROR;
			if (!file->Open(name,GE_WRITE,FILE_NODIALOG,GE_MOTOROLA)) return EXECUTION_RESULT_MEMORYERROR;

			//Image, Light Count
			str = LongToString(lcnt);
			WriteLine(file, str);

			for(LONG c=0; c<lcnt; c++){
				pos = padr[c];
				lulv = !(pos*ivmg);
				//lulv = !(bd->WS(pos));

				//Image Name
				str = mainname+RealToString(c,3,0)+ext;

				//Lu
				str += "\t" + RealToString(lulv.x,-1,6);
				//Lv
				str += "\t" + RealToString(lulv.y,-1,6);
				//Lw
				str += "\t" + RealToString(lulv.z,-1,6);

				WriteLine(file, str);
			}

			is_writen = TRUE;
		}
	}

	return EXECUTION_RESULT_OK;
}
//#################################################################
Bool RegisterPTMTag(void)
{
	String name=GeLoadString(IDS_PTMTAG); if (!name.Content()) return TRUE;
	return RegisterTagPlugin(DI_PTMTAG_TAG_ID,name,TAG_VISIBLE | TAG_EXPRESSION,PTMTagData::Alloc,"TPTMTag","PTMTag.tif",0);
}
