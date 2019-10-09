// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		AutoArrangerPiPL.r
//
//	Description:
//		An automation plug in that uses a hidden filter plug in to
//		do something very boring and set all the pixels to 0.
//
//-------------------------------------------------------------------------------
#include "PIDefines.h"

#define plugInName "AutoArranger"
#define plugInCopyrightYear	"2019"
#define plugInDescription \
	"Arrange multiple same size documents into one document"

#ifdef __PIMac__
	#include "PIGeneral.r"
	#include "PIUtilities.r"
#elif defined(__PIWin__)
	#define Rez
	#include "PIGeneral.h"
	#include "PIUtilities.r"
#endif

#include "PITerminology.h"
#include "PIActions.h"

resource 'PiPL' ( 16000, "AutoArranger", purgeable)
	{
		{
		Kind { Actions },
		Name { plugInName"..." },
		Category { "AdobeSDK" },
		Version { (latestActionsPlugInVersion << 16) | latestActionsPlugInSubVersion },
		
		Component { ComponentNumber, plugInName },

		#ifdef __PIMac__
			CodeMacIntel64 { "AutoPluginMain" },
		#else
			#if defined(_WIN64)
				CodeWin64X86 { "AutoPluginMain" },
			#else
				CodeWin32X86 { "AutoPluginMain" },
			#endif
		#endif

		EnableInfo { "true" },

		Persistent{},
		
		Messages
		{
			startupRequired,
			doesNotPurgeCache,
			shutdownRequired,
			acceptProperty
		},

		HasTerminology
			{ 
			'AArr', 
			'AuAr', 
			16000, 
			"92SelfUse"
			},
		}
	};

//-------------------------------------------------------------------------------
//	Dictionary (scripting) resource
//-------------------------------------------------------------------------------

resource 'aete' (16000, "AutoArranger dictionary", purgeable)
	{
	1, 0, english, roman,					/* aete version and language specifiers */
		{
		"AutoArranger",							/* vendor suite name */
		"AutoArranger plug-ins",					/* optional description */
		'AArr',								/* suite ID */
		1,									/* suite code, must be 1 */
		1,									/* suite level, must be 1 */
			{								/* structure for automation */
			"AutoArranger",						/* name */
			"No comment",					/* optional description */
			'AArr',							/* class ID, must be unique or Suite ID */
			'AuAr',							/* event ID, must be unique */

			NO_REPLY,						/* never a reply */
			IMAGE_DIRECT_PARAMETER,			/* direct parameter, used by Photoshop */
				{							// filter or selection class here:
				}
			},
			{},	/* non-filter/automation plug-in class here */
			{}, /* comparison ops (not supported) */
			{ // Enumerations go here:
			}	/* end of any enumerations */
		}
	};
// end AutoArrangerPiPL.r
