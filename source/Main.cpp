/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// Starts the plugin registration

#include "c4d.h"
#include "re_print.h"

// forward declarations
Bool RegisterPTM_IO(void);
Bool RegisterPTMshaderShader(void);
Bool RegisterPTMgenShader(void);
Bool RegisterPTMTag(void);
Bool RegisterPTMShader3iShader(void);

//#############################################################################
Bool PluginStart(void)
{
	// menu plugins
	if (!RegisterPTM_IO()) { print("!RegisterPTM_IO"); }
	if (!RegisterPTMshaderShader()) { print("!RegisterPTMshaderShader"); }
	if (!RegisterPTMgenShader()) { print("!RegisterPTMgenShader"); }
	if (!RegisterPTMTag()) { print("!RegisterPTMTag"); }
	if (!RegisterPTMShader3iShader()) { print("!RegisterPTMShader3iShader"); }

	GePrint("  Remotion  PTM_IO 0.1 loaded.");

	return TRUE;
}
//#############################################################################
void PluginEnd(void)
{
}
//#############################################################################
Bool PluginMessage(LONG id, void *data)
{
	//use the following lines to set a plugin priority
	//
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!resource.Init()) return FALSE; // don't start plugin without resource
			return TRUE;

		case C4DMSG_PRIORITY: 
			return TRUE;
	}

	return FALSE;
}
