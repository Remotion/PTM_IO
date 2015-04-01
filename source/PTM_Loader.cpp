/////////////////////////////////////////////////////////////
// PTM_Loader.cpp                                          //
/////////////////////////////////////////////////////////////
// (c) 2004 Remotion (Igor Schulz), all rights reserved    //
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "PresetTools.h"

#include "re_print.h"

#include "PTM_Types.h"

#include "XPTMshader.h"
#define DI_PTMSHADER_SHADER_ID 1015747

#include "Mmaterial.h"

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_PTM_LOADER 1015746


//-----------------------------------------------------------------------------
class PTMLoaderData : public SceneLoaderData
{
	public:
		virtual Bool Identify(PluginSceneLoader *node, const Filename &name, UCHAR *probe, LONG size);
		virtual LONG Load(PluginSceneLoader *node, const Filename &name, BaseDocument *doc, LONG filterflags, String *error, BaseThread *bt);

		static NodeData *Alloc(void) { return gNew PTMLoaderData; }
};
//-----------------------------------------------------------------------------
Bool PTMLoaderData::Identify(PluginSceneLoader *node, const Filename &name, UCHAR *probe, LONG size)
{
	if( (probe[0]=='P') && (probe[1]=='T') && (probe[2]=='M') ) return TRUE; 
	else return FALSE;
}
//-----------------------------------------------------------------------------
LONG PTMLoaderData::Load(PluginSceneLoader *node, const Filename &name, BaseDocument *doc, LONG filterflags, String *error, BaseThread *bt)
{
	BaseMaterial *mat = BaseMaterial::Alloc(Mmaterial); if(!mat) return FILEERROR_MEMORY;
	doc->InsertMaterial(mat);

	PluginShader *sha = PluginShader::Alloc(DI_PTMSHADER_SHADER_ID); if(!sha) return FILEERROR_MEMORY;
	BaseContainer *sh_data = sha->GetDataInstance();
	sh_data->SetFilename(XPTM_FILE_NAME,name);

	mat->InsertShader(sha);

	BaseContainer *mat_data = mat->GetDataInstance();
	mat_data->SetLink(MATERIAL_COLOR_SHADER,sha); //TODO: ??????
	//mat_data->SetLink(MATERIAL_LUMINANCE_SHADER,sha);
	
	mat_data->SetBool(MATERIAL_USE_COLOR,FALSE);
	mat_data->SetBool(MATERIAL_USE_SPECULAR,FALSE);
	mat_data->SetBool(MATERIAL_USE_LUMINANCE,TRUE);
	//mat_b_data->SetBool(MATERIAL_USE_DISPLACEMENT,TRUE);
	//if(mat_data->GetBool(MATERIAL_USE_LUMINANCE)) print("Use Lum");

	//mat->Update(TRUE,FALSE);
	//doc->Message(MSG_UPDATE);
	mat->Message(MSG_UPDATE);
	//mat->MultiMessage(MULTIMSG_BROADCAST,MSG_UPDATE,NULL);

	return FILEERROR_NONE;
}

//#############################################################################
Bool RegisterPTM_IO(void)
{
	// decide by name if the plugin shall be registered - just for user convenience
	String name=GeLoadString(IDS_PTM); if (!name.Content()) return TRUE;
	
	if (!RegisterSceneLoaderPlugin(ID_PTM_LOADER,name,0,PTMLoaderData::Alloc,0,NULL)) return FALSE;
	return TRUE;
}

