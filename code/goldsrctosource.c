

#include <stdarg.h>
#include <stddef.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_TRIANGLE
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#define STB_SPRINTF_NOUNALIGNED
#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

#define GC_MATHS_IMPLEMENTATION
#include "gc_maths.h"

#ifdef DEBUG_GRAPHICS
#ifdef GC_DEBUG
#define SOKOL_DEBUG
#endif // GC_DEBUG
#define SOKOL_IMPL
#define SOKOL_GLCORE
#define SOKOL_NO_ENTRY
#include "sokol/sokol_log.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
//typedef v2 ImVec2;
//typedef v4 ImVec4;
#include "imgui/cimgui.h"
#include "sokol/sokol_imgui.h"
#endif // DEBUG_GRAPHICS

#include "gc_common.h"
#include "memory.h"
#include "platform.h"
#include "str.h"

#define STRINGMAP_VALUE_TYPE i32
#define STRINGMAP_NAME Int
#include "gc_stringmap_generator.h"

#include "memory.c"
#include "printing.c"
#define dmx_serialise
#define dmx_name_override(name)
#define dmx_function(function)
#define dmx_serialise_array(type, name) i32 TOKENPASTE(name, Count); type *names

#include "str.c"
#include "goldsrctosource.h"
#include "vtf.h"
#include "utils.c"

#include "sourcebsp.c"
#include "goldsrcbsp.c"
#include "wad3.c"
#include "zip.c"

#ifdef DEBUG_GRAPHICS
#include "debug_render.c"
#else
#include "debug_render_stub.c"
#endif

#include "entities.c"
#include "conversion.c"
#include "vmf.c"
#include "bsp.c"

#include "pcg/pcg_basic.c"
#include "dmx.c"
#include "vmap.c"



static_global const char *g_cmdArgTypeStrings[CMDARGTYPE_COUNT] = {
	"None",
	"String",
	"Integer",
};

static_function void FatalError(const char *error)
{
	MyPrintf("FATAL ERROR: $\n", error);
	ASSERT(0);
	exit(EXIT_FAILURE);
}

static_function void Error(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	PrintString("Error: ");
	VPrint(format, args);
	
	va_end(args);
	ASSERT(0);
}

static_function void Warning(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	PrintString("Warning: ");
	VPrint(format, args);
	
	va_end(args);
}


static_function i32 GsrcContentsToSrcContents(i32 gsrcContents)
{
	i32 result = 0;
	switch (gsrcContents)
	{
		case GSRC_CONTENTS_SKY:
		case GSRC_CONTENTS_SOLID: {result |= SRC_CONTENTS_SOLID;} break;
		case GSRC_CONTENTS_LAVA:
		case GSRC_CONTENTS_WATER: {result |= SRC_CONTENTS_WATER;} break;
		case GSRC_CONTENTS_SLIME: {result |= SRC_CONTENTS_SLIME;} break;
		case GSRC_CONTENTS_CLIP: {result |= SRC_CONTENTS_PLAYERCLIP;} break;
		case GSRC_CONTENTS_CURRENT_0: {result |= SRC_CONTENTS_WATER | SRC_CONTENTS_CURRENT_0;} break;
		case GSRC_CONTENTS_CURRENT_90: {result |= SRC_CONTENTS_WATER | SRC_CONTENTS_CURRENT_90;} break;
		case GSRC_CONTENTS_CURRENT_180: {result |= SRC_CONTENTS_WATER | SRC_CONTENTS_CURRENT_180;} break;
		case GSRC_CONTENTS_CURRENT_270: {result |= SRC_CONTENTS_WATER | SRC_CONTENTS_CURRENT_270;} break;
		case GSRC_CONTENTS_CURRENT_UP: {result |= SRC_CONTENTS_WATER | SRC_CONTENTS_CURRENT_UP;} break;
		case GSRC_CONTENTS_CURRENT_DOWN: {result |= SRC_CONTENTS_WATER | SRC_CONTENTS_CURRENT_DOWN;} break;
		case GSRC_CONTENTS_TRANSLUCENT: {result |= SRC_CONTENTS_SOLID | SRC_CONTENTS_WINDOW;} break;
		default: {result |= SRC_CONTENTS_EMPTY;} break;
	}
	return result;
}

static_function bool ParseCmdArgs(CmdArgs *cmdArgs, i32 argCount, char *arguments[])
{
	bool result = true;
	for (i32 i = 1; i < argCount; i++)
	{
		bool found = false;
		for (i32 j = 0; j < ARRAYCOUNT(cmdArgs->args); j++)
		{
			if (i >= argCount)
			{
				break;
			}
			
			if (StringEquals(arguments[i], cmdArgs->args[j].argName, false))
			{
				found = true;
				if (cmdArgs->args[j].isInCmdLine)
				{
					Error("%s is used twice in the command line!\n", arguments[i]);
					result = false;
					break;
				}
				
				cmdArgs->args[j].isInCmdLine = true;
				if (cmdArgs->args[j].type != CMDARG_NONE)
				{
					if (i + 1 < argCount)
					{
						if (arguments[i + 1][0] != '-')
						{
							if (cmdArgs->args[j].type == CMDARG_STRING)
							{
								i64 argLen = StringLength(arguments[i + 1]);
								if (argLen >= sizeof(cmdArgs->args[j].stringValue))
								{
									Error("String is too long for argument %s! Maximum length is %lli characters.\n\n",
										  arguments[i], (i64)sizeof(MEMBER(CmdArg, stringValue)) - 1);
									result = false;
									break;
								}
								
								Mem_Copy(arguments[i + 1], cmdArgs->args[j].stringValue, argLen);
								cmdArgs->args[j].stringValue[argLen] = '\0';
							}
							else if (cmdArgs->args[j].type == CMDARG_INTEGER)
							{
								if (!StringToS32(arguments[i + 1], &cmdArgs->args[j].intValue))
								{
									Error("Couldn't convert command \"%s\"'s argument \"%s\" to an integer!\n\n",
										  arguments[i], arguments[i + 1]);
									result = false;
									break;
								}
							}
							else
							{
								Error("Dumbass programmer configured the command line options incorrectly. Shame on them!\n\n");
							}
							
							i++;
						}
						else
						{
							// oh no sad sad :(
							Error("Argument missing for command %s\n\n", arguments[i]);
							result = false;
							break;
						}
					}
					else
					{
						// oh no sad sad :(
						Error("Argument missing for command %s\n\n", arguments[i]);
						result = false;
						break;
					}
				}
				continue;
			}
			
			if (found)
			{
				break;
			}
		}
		
		if (!found)
		{
			Error("Invalid command \"%s\"\n\n", arguments[i]);
			result = false;
			break;
		}
		
		if (!result)
		{
			break;
		}
	}
	
	return result;
}

static_function void PrintCmdLineHelp(CmdArgs *cmdArgs)
{
	PrintString("Available commands:\n");
	for (i32 i = 0; i < ARRAYCOUNT(cmdArgs->args); i++)
	{
		if (cmdArgs->args[i].type == CMDARG_NONE)
		{
			MyPrintf("$: $ \n",
				  cmdArgs->args[i].argName,
				  cmdArgs->args[i].description);
		}
		else
		{
			MyPrintf("$ [$] : $ \n",
				  cmdArgs->args[i].argName,
				  g_cmdArgTypeStrings[cmdArgs->args[i].type],
				  cmdArgs->args[i].description);
		}
	}
	PrintString("\n");
}

static_function void BSPMain(i32 argCount, char *arguments[])
{
	Arena tempArena = ArenaCreate(GIGABYTES(2));
	Arena arena = ArenaCreate(GIGABYTES(4));
	
	// NOTE(GameChaos): TODO: negative values for floats/integers on the cmd line don't work right
	// now cos it thinks it's another parameter because of the - character.
	CmdArgs cmdArgs = {
		.help = {"-help", "Help!!!", CMDARG_NONE},
		.input = {"-input", "Input GoldSrc v31 bsp file to be converted.", CMDARG_STRING},
		.outputbsp = {"-outputbsp", "Output path of the converted v21 Source bsp file (CS:GO).", CMDARG_STRING},
		.outputvmf = {"-outputvmf", "Output path of the converted vmf file.", CMDARG_STRING},
		.outputvmap = {"-outputvmap", "Output path of the converted Source 2 vmap file.", CMDARG_STRING},
		.enginePath = {"-enginepath", "Path of the Half-Life/ folder. Example: \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Half-Life\"", CMDARG_STRING},
		.mod = {"-mod", "Name of the mod folder. Example: cstrike", CMDARG_STRING},
		// TODO: enable the usage of this when converting a bsp as well.
		.assetPath = {"-assetpath", "Path to export materials and assets to when converting a VMF. Example: \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo\\\". Converted materials will be put into \"" CONVERTED_MATERIAL_PATH "\" in this path.", CMDARG_STRING},
	};
	
	if (!ParseCmdArgs(&cmdArgs, argCount, arguments))
	{
		Error("Command line parsing failed!\n\n");
		PrintCmdLineHelp(&cmdArgs);
		return;
	}
	
	// TODO: check if all the paths are valid
	if (!cmdArgs.input.isInCmdLine)
	{
		Error("Please provide an input bsp file with -input.\n\n");
		PrintCmdLineHelp(&cmdArgs);
		return;
	}
	
	if (!cmdArgs.outputbsp.isInCmdLine && !cmdArgs.outputvmf.isInCmdLine)
	{
		Error("Please provide an output path with -outputbsp or -outputvmf.\n\n");
		PrintCmdLineHelp(&cmdArgs);
		return;
	}
	
	if (!cmdArgs.enginePath.isInCmdLine)
	{
		Error("Please provide a path for the Half-Life folder with -enginepath.\n\n");
		PrintCmdLineHelp(&cmdArgs);
		return;
	}
	
	if (!cmdArgs.mod.isInCmdLine)
	{
		Error("Please provide a name for the mod folder with -mod.\n\n");
		PrintCmdLineHelp(&cmdArgs);
		return;
	}
	
	GamePaths *paths = ArenaAlloc(&arena, sizeof(*paths));
	if (cmdArgs.assetPath.isInCmdLine)
	{
		Format(paths->assets, sizeof(paths->assets), "%s", cmdArgs.assetPath.stringValue);
	}
	
	Format(paths->valve, sizeof(paths->valve), "%s", cmdArgs.enginePath.stringValue);
	AppendToPath(paths->valve, sizeof(paths->valve), "valve");
	
	Format(paths->mod, sizeof(paths->mod), "%s", cmdArgs.enginePath.stringValue);
	AppendToPath(paths->mod, sizeof(paths->mod), cmdArgs.mod.stringValue);
	
	ReadFileResult fileData = ReadEntireFile(&arena, CMDARG_GET_STRING(cmdArgs.input));
	// NOTE(GameChaos): kz_goldenbean_200401.bsp CRASHES THE PROGRAM TODO: debug!
	ASSERT(fileData.contents);
	ASSERT(fileData.size);
	
	GsrcMapData mapData = {};
	mapData.fileDataSize = fileData.size;
	mapData.fileData = fileData.contents;
	
	
	if (GsrcImportBsp(&tempArena, &mapData))
	{
		
#ifdef DEBUG_GRAPHICS
		// NOTE(GameChaos): debug leaf faces
		i32 faceCount = 0;
		for (i32 faceInd = 0; faceInd < mapData.faceCount; faceInd++)
		{
			GsrcFace face = mapData.lumpFaces[faceInd];
			GsrcTexinfo texinfo = mapData.lumpTexinfo[face.texInfoIndex];
			// NOTE(GameChaos): static_persist because the struct is way too
			//  big to allocate on the stack
			static_persist Verts poly;
			poly.vertCount = 0;
			for (u32 surfEdge = face.firstEdge;
				 surfEdge < face.firstEdge + face.edges;
				 surfEdge++)
			{
				i32 edge = mapData.lumpSurfEdges[surfEdge];
				v3 vert;
				if (edge >= 0)
				{
					vert = mapData.lumpVertices[mapData.lumpEdges[edge].vertex[0]];
				}
				else
				{
					vert = mapData.lumpVertices[mapData.lumpEdges[-edge].vertex[1]];
				}
				
				poly.verts[poly.vertCount++] = vert;
			}
			
			//if (faceCount < 2)
			{
				v4 s = (v4){texinfo.vecs[0][0], texinfo.vecs[0][1], texinfo.vecs[0][2], texinfo.vecs[0][3]};
				v4 t = (v4){texinfo.vecs[1][0], texinfo.vecs[1][1], texinfo.vecs[1][2], texinfo.vecs[1][3]};
				GsrcPlane plane = mapData.lumpPlanes[face.plane];
				DebugGfxAddFace(&poly, plane.normal, texinfo.miptex, s, t);
				faceCount++;
			}
		}
#endif
		if (cmdArgs.outputvmap.isInCmdLine)
		{
			VmapFromGoldsrc(&arena, &tempArena, &mapData, CMDARG_GET_STRING(cmdArgs.outputvmap), paths);
		}
		
		// convert to source
		SrcMapData srcMapData = {};
		if (cmdArgs.outputbsp.isInCmdLine)
		{
			if (BspFromGoldsource(&arena, &tempArena, &mapData, &srcMapData, CMDARG_GET_STRING(cmdArgs.outputbsp),
								  paths->mod, paths->valve))
			{		
				PrintString("\n");
				for (int i = 0; i < SRC_HEADER_LUMPS; i++)
				{
					// pad string nicely
					char firstPart[128];
					Format(firstPart, sizeof firstPart, "%-35s offset:", g_srcLumpNames[i]);
					
					char secondPart[128];
					Format(secondPart, sizeof secondPart, "length: %i", srcMapData.header->lump[i].length);
					
					Print("%s %-11i %-32s\n",
						  firstPart,
						  srcMapData.header->lump[i].offset,
						  secondPart);
				}
			}
		}
		if (cmdArgs.outputvmf.isInCmdLine)
		{
			VmfFromGoldsource(&arena, &tempArena, &mapData, CMDARG_GET_STRING(cmdArgs.outputvmf),
							  paths->mod, paths->valve, paths->assets);
		}
	}
	else
	{
		ASSERT(0);
	}
	
	ArenaFree(&arena);
	ArenaFree(&tempArena);
}