
#include "common.h"

#include <stdarg.h>

#include "platform.h"
#define HANDMADE_MATH_IMPLEMENTATION
#define HMM_PREFIX
#include "handmade_math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_TRIANGLE
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#include "str.cpp"
#include "goldsrctosource.h"
#include "vtf.h"
#include "utils.cpp"

#include "sourcebsp.cpp"
#include "goldsrcbsp.cpp"
#include "wad3.cpp"
#include "zip.cpp"

#ifdef DEBUG_GRAPHICS
#include "debug_render.cpp"
#else
#include "debug_render_stub.cpp"
#endif

#include "entities.cpp"
#include "conversion.cpp"
#include "vmf.cpp"
#include "bsp.cpp"

#include "dmx.cpp"

internal void FatalError(char *error)
{
	Print("FATAL ERROR: %s\n", error);
	ASSERT(0);
	exit(EXIT_FAILURE);
}

internal void Error(char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	PrintString("Error: ");
	Vprint(format, args);
	
	va_end(args);
	ASSERT(0);
}

internal void Warning(char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	PrintString("Warning: ");
	Vprint(format, args);
	
	va_end(args);
}

inline void *BufferPushDataAndSetLumpSize(FileWritingBuffer *buffer, SrcHeader *header, s32 lumpIndex, void *data, s32 bytes)
{
	void *result = buffer->memory + buffer->usedBytes;
	header->lump[lumpIndex].offset = (s32)buffer->usedBytes;
	header->lump[lumpIndex].length = bytes;
	if (!BufferPushData(buffer, data, bytes))
	{
		ASSERT(0);
		result = NULL;
	}
	
	return result;
}

internal s32 GsrcContentsToSrcContents(s32 gsrcContents)
{
	s32 result = 0;
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

internal b32 ParseCmdArgs(CmdArgs *cmdArgs, s32 argCount, char *arguments[])
{
	b32 result = true;
	for (s32 i = 1; i < argCount; i++)
	{
		b32 found = false;
		for (s32 j = 0; j < ARRAYCOUNT(cmdArgs->args); j++)
		{
			if (i >= argCount)
			{
				break;
			}
			
			if (StringEquals(arguments[i], cmdArgs->args[j].argName))
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
								s64 argLen = StringLength(arguments[i + 1]);
								if (argLen >= sizeof(cmdArgs->args[j].stringValue))
								{
									Error("String is too long for argument %s! Maximum length is %lli characters.\n\n",
										  arguments[i], (s64)sizeof(MEMBER(CmdArg, stringValue)) - 1);
									result = false;
									break;
								}
								
								Mem_Copy(arguments[i + 1], cmdArgs->args[j].stringValue, argLen, sizeof(cmdArgs->args[j].stringValue));
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

internal void PrintCmdLineHelp(CmdArgs *cmdArgs)
{
	PrintString("Available commands:\n");
	for (s32 i = 0; i < ARRAYCOUNT(cmdArgs->args); i++)
	{
		if (cmdArgs->args[i].type == CMDARG_NONE)
		{
			Print("%s: %s \n",
				  cmdArgs->args[i].argName,
				  cmdArgs->args[i].description);
		}
		else
		{
			Print("%s [%s] : %s \n",
				  cmdArgs->args[i].argName,
				  g_cmdArgTypeStrings[cmdArgs->args[i].type],
				  cmdArgs->args[i].description);
		}
	}
	PrintString("\n");
}

void BSPMain(s32 argCount, char *arguments[])
{
	Arena tempArena = ArenaCreate(GIGABYTES(2));
	Arena arena = ArenaCreate(GIGABYTES(4));
	
	DmxTest(&arena);
	
	// NOTE(GameChaos): TODO: negative values for floats/integers on the cmd line don't work right
	// now cos it thinks it's another parameter because of the - character.
	CmdArgs cmdArgs = {};
	cmdArgs.help = {"-help", "Help!!!", CMDARG_NONE};
	cmdArgs.input = {"-input", "Input GoldSrc v31 bsp file to be converted.", CMDARG_STRING};
	cmdArgs.outputbsp = {"-outputbsp", "Output path of the converted v21 Source bsp file (CS:GO).", CMDARG_STRING};
	cmdArgs.outputvmf = {"-outputvmf", "Output path of the converted vmf file.", CMDARG_STRING};
	cmdArgs.enginePath = {"-enginepath", "Path of the Half-Life/ folder. Example: \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Half-Life\"", CMDARG_STRING};
	cmdArgs.mod = {"-mod", "Name of the mod folder. Example: cstrike", CMDARG_STRING};
	// TODO: enable the usage of this when converting a bsp as well.
	cmdArgs.assetPath = {"-assetpath", "Path to export materials and assets to when converting a VMF. Example: \"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo\\\". Converted materials will be put into \"" CONVERTED_MATERIAL_PATH "\" in this path.", CMDARG_STRING};
	
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
	
	char *assetPath = NULL;
	if (cmdArgs.assetPath.isInCmdLine)
	{
		assetPath = cmdArgs.assetPath.stringValue;
	}
	
	char valvePath[512];
	Format(valvePath, sizeof(valvePath), "%s", cmdArgs.enginePath.stringValue);
	AppendToPath(valvePath, sizeof(valvePath), "valve");
	
	char modPath[512];
	Format(modPath, sizeof(modPath), "%s", cmdArgs.enginePath.stringValue);
	AppendToPath(modPath, sizeof(modPath), cmdArgs.mod.stringValue);
	
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
		s32 faceCount = 0;
		for (s32 faceInd = 0; faceInd < mapData.faceCount; faceInd++)
		{
			GsrcFace face = mapData.lumpFaces[faceInd];
			GsrcTexinfo texinfo = mapData.lumpTexinfo[face.texInfoIndex];
			local_persist Verts poly;
			poly.vertCount = 0;
			for (u32 surfEdge = face.firstEdge;
				 surfEdge < face.firstEdge + face.edges;
				 surfEdge++)
			{
				s32 edge = mapData.lumpSurfEdges[surfEdge];
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
				v4 s = Vec4(texinfo.vecs[0][0], texinfo.vecs[0][1], texinfo.vecs[0][2], texinfo.vecs[0][3]);
				v4 t = Vec4(texinfo.vecs[1][0], texinfo.vecs[1][1], texinfo.vecs[1][2], texinfo.vecs[1][3]);
				GsrcPlane plane = mapData.lumpPlanes[face.plane];
				DebugGfxAddFace(&poly, plane.normal, texinfo.miptex, s, t);
				faceCount++;
			}
		}
#endif
		// convert to source
		SrcMapData srcMapData = {};
		if (cmdArgs.outputbsp.isInCmdLine)
		{
			if (BspFromGoldsource(&arena, &tempArena, &mapData, &srcMapData, CMDARG_GET_STRING(cmdArgs.outputbsp),
								  modPath, valvePath))
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
			if (VmfFromGoldsource(&arena, &tempArena, &mapData, CMDARG_GET_STRING(cmdArgs.outputvmf),
								  modPath, valvePath, assetPath))
			{
			}
		}
	}
	else
	{
		ASSERT(0);
	}
	ArenaFree(&arena);
	ArenaFree(&tempArena);
}