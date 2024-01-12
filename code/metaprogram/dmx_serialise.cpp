
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

enum DmxAttrType : u8
{
	DMX_ATTR_COUNT = 16,
	DMX_ATTR_UNKNOWN = 0,
	
	DMX_ATTR_ELEMENT    = 1,
	DMX_ATTR_INT32      = 2,
	DMX_ATTR_F32        = 3,
	DMX_ATTR_BOOL       = 4,
	DMX_ATTR_STRING     = 5,
	DMX_ATTR_BINARYBLOB = 6,
	DMX_ATTR_TIMESPAN   = 7,
	DMX_ATTR_RGBA8      = 8,
	DMX_ATTR_VECTOR2D   = 9,
	DMX_ATTR_VECTOR3D   = 10,
	DMX_ATTR_VECTOR4D   = 11,
	DMX_ATTR_QANGLE     = 12,
	DMX_ATTR_QUATERNION = 13,
	DMX_ATTR_MATRIX4X4  = 14,
	DMX_ATTR_UINT64     = 15,
	DMX_ATTR_BYTE       = 16,
	
	DMX_ATTR_ELEMENT_ARRAY    = 33,
	DMX_ATTR_INT32_ARRAY      = 34,
	DMX_ATTR_F32_ARRAY        = 35,
	DMX_ATTR_BOOL_ARRAY       = 36,
	DMX_ATTR_STRING_ARRAY     = 37,
	DMX_ATTR_BINARYBLOB_ARRAY = 38,
	DMX_ATTR_TIMESPAN_ARRAY   = 39,
	DMX_ATTR_RGBA8_ARRAY      = 40,
	DMX_ATTR_VECTOR2D_ARRAY   = 41,
	DMX_ATTR_VECTOR3D_ARRAY   = 42,
	DMX_ATTR_VECTOR4D_ARRAY   = 43,
	DMX_ATTR_QANGLE_ARRAY     = 44,
	DMX_ATTR_QUATERNION_ARRAY = 45,
	DMX_ATTR_MATRIX4X4_ARRAY  = 46,
	DMX_ATTR_UINT64_ARRAY     = 47,
	DMX_ATTR_BYTE_ARRAY       = 48,
};

global char *g_attrToString[] = {
	"Unknown",
	"ElementId",
	"Int",
	"F32",
	"Bool",
	"String",
	"Binary",
	"Timespan",
	"Rgba8",
	"V2",
	"V3",
	"V4",
	"QAngle",
	"Quaternion",
	"Mat4",
	"U64",
	"U8",
};

enum TokenType
{
	TOKEN_UNKNOWN,
	TOKEN_OPEN_BRACKET,  // (
	TOKEN_CLOSE_BRACKET,  // )
	TOKEN_OPEN_BRACE,  // {
	TOKEN_CLOSE_BRACE, // }
	TOKEN_ASTERISK,  // *
	TOKEN_SEMICOLON,  // ;
	TOKEN_COMMA,  // ;
	TOKEN_IDENTIFIER,
	TOKEN_EOS, // end of stream
};

#define STR(strLiteral) (str{sizeof(strLiteral) - 1, strLiteral})

struct str
{
	s32 length;
	char *data;
};

struct Token
{
	TokenType type;
	str string;
};

struct Tokeniser
{
	char *at;
};

struct DmxProperty
{
	bool array;
	bool pointer;
	str type;
	str name;
	str nameOverride;
	str serialiseFuncOverride;
	// TODO: default value
};

struct DmxStruct
{
	str typeName;
	s32 propertyCount;
	DmxProperty props[256];
};

struct State
{
	s32 structCount;
	DmxStruct structs[128];
};

global char *g_tokenNames[] = {
	"UNKNOWN",
	"(",
	")",
	"{",
	"}",
	"*",
	";",
	",",
	"IDENTIFIER",
	"END OF STREAM",
};

internal str ReadEntireFileString(char *path)
{
	str result = {};
	
	FILE *file = fopen(path, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		result.length = ftell(file) + 1;
		fseek(file, 0, SEEK_SET);
		
		result.data = (char *)malloc(result.length);
		fread(result.data, result.length - 1, 1, file);
		result.data[result.length - 1] = 0;
		fclose(file);
	}
	
	return result;
}

internal bool StrEquals(str a, str b, b32 caseSensitive = true)
{
	bool result = true;
	if (a.length == b.length)
	{
		char *aEnd = a.data + a.length;
		char *bEnd = b.data + b.length;
		if (caseSensitive)
		{
			result = memcmp(a.data, b.data, a.length) == 0;
		}
		else
		{
			for (char *c1 = a.data, *c2 = b.data;
				 c1 < aEnd && c2 < bEnd;
				 c1++, c2++)
			{
				char lower1 = *c1 <= 'z' ? *c1 & 0xdf : *c1;
				char lower2 = *c2 <= 'z' ? *c2 & 0xdf : *c1;
				if (lower1 != lower2)
				{
					result = false;
					break;
				}
			}
		}
	}
	else
	{
		result = false;
	}
	return result;
}

internal void Error(char *format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	printf("ERROR: %s\n", buffer);
	ASSERT(0);
	exit(1);
}

internal bool IsWhiteSpace(char c)
{
	bool result = c == ' ' || c == '\t'
		|| c == '\v' || c == '\n'
		|| c == '\r' || c == '\f';
	return result;
}

internal bool IsNewLine(char c)
{
	bool result = (c == '\r' || c == '\n');
	return result;
}

internal void EatWhiteSpace(Tokeniser *tokeniser)
{
	while (IsWhiteSpace(tokeniser->at[0]))
	{
		tokeniser->at++;
	}
}

internal void EatComments(Tokeniser *tokeniser)
{
	if (tokeniser->at[0] == '/' && tokeniser->at[1] == '/')
	{
		while (tokeniser->at[0] && !IsNewLine(*(++tokeniser->at)));
		EatWhiteSpace(tokeniser);
	}
	else if (tokeniser->at[0] == '/' && tokeniser->at[1] == '*')
	{
		tokeniser->at += 2;
		while (tokeniser->at[0] && (tokeniser->at[0] != '*' || tokeniser->at[1] != '/'))
		{
			tokeniser->at++;
		}
		if (tokeniser->at[0])
		{
			tokeniser->at += 2;
		}
		EatWhiteSpace(tokeniser);
	}
}

internal bool IsIdentifierChar(char c)
{
	b32 result = ((c >= 'a' && c <= 'z')
				  || (c >= 'A' && c <= 'Z')
				  || (c >= '0' && c <= '9') // identifiers can start with/be numbers
				  || c == '_');
	return result;
}

internal Token GetToken(Tokeniser *tokeniser)
{
	Token result = {};
	EatWhiteSpace(tokeniser);
	EatComments(tokeniser);
	result.string.data = tokeniser->at;
	result.string.length = 1;
	switch (*tokeniser->at)
	{
		case '(': {result.type = TOKEN_OPEN_BRACKET;} break;
		case ')': {result.type = TOKEN_CLOSE_BRACKET;} break;
		case '{': {result.type = TOKEN_OPEN_BRACE;} break;
		case '}': {result.type = TOKEN_CLOSE_BRACE;} break;
		case '*': {result.type = TOKEN_ASTERISK;} break;
		case ';': {result.type = TOKEN_SEMICOLON;} break;
		case ',': {result.type = TOKEN_COMMA;} break;
		case 0: {result.type = TOKEN_EOS;} break;
		default:
		{
			if (IsIdentifierChar(tokeniser->at[0]))
			{
				result.type = TOKEN_IDENTIFIER;
				while (IsIdentifierChar(*(++tokeniser->at)))
				{
					result.string.length++;
				}
				tokeniser->at--;
			}
			else
			{
				result.type = TOKEN_UNKNOWN;
			}
		};
	}
	tokeniser->at++;
	
	return result;
}

internal void ErrorWrongTokenType(TokenType expected, TokenType actual)
{
	Error("Expected token \"%s\", but got \"%s\"", g_tokenNames[expected], g_tokenNames[actual]);
}

internal Token RequireIdentifier(Tokeniser *tokeniser, str name)
{
	Token result = GetToken(tokeniser);
	if (result.type != TOKEN_IDENTIFIER)
	{
		Error("Expected identifier, but got \"%s\"", g_tokenNames[result.type]);
	}
	else if (!StrEquals(result.string, name))
	{
		Error("Expected identifier \"%.*s\", but got \"%.*s\"", name.length, name.data, result.string.length, result.string.data);
	}
	return result;
}

internal Token RequireToken(Tokeniser *tokeniser, TokenType type)
{
	Token token = GetToken(tokeniser);
	if (token.type != type)
	{
		ErrorWrongTokenType(type, token.type);
	}
	return token;
}

internal DmxStruct ParseStruct(Tokeniser *tokeniser)
{
	RequireIdentifier(tokeniser, STR("struct"));
	Token token = RequireToken(tokeniser, TOKEN_IDENTIFIER);
	
	DmxStruct result = {};
	result.typeName = token.string;
	
	RequireToken(tokeniser, TOKEN_OPEN_BRACE);
	token = RequireToken(tokeniser, TOKEN_IDENTIFIER);
	while (token.type == TOKEN_IDENTIFIER)
	{
		DmxProperty prop = {};
		if (StrEquals(token.string, STR("dmx_serialise_array")))
		{
			prop.array = true;
			RequireToken(tokeniser, TOKEN_OPEN_BRACKET);
			prop.type = RequireToken(tokeniser, TOKEN_IDENTIFIER).string; // type
			token = GetToken(tokeniser);
			if (token.type == TOKEN_ASTERISK)
			{
				prop.pointer = true;
				RequireToken(tokeniser, TOKEN_COMMA);
			}
			else if (token.type != TOKEN_COMMA)
			{
				ErrorWrongTokenType(TOKEN_COMMA, token.type);
			}
			prop.name = RequireToken(tokeniser, TOKEN_IDENTIFIER).string;
			
			RequireToken(tokeniser, TOKEN_CLOSE_BRACKET);
		}
		else
		{
			prop.type = token.string; // type
			token = GetToken(tokeniser);
			if (token.type == TOKEN_ASTERISK)
			{
				prop.name = RequireToken(tokeniser, TOKEN_IDENTIFIER).string;
				prop.pointer = true;
			}
			else if (token.type == TOKEN_IDENTIFIER)
			{
				prop.name = token.string;
			}
			else
			{
				ErrorWrongTokenType(TOKEN_IDENTIFIER, token.type);
			}
			// TODO: serialisation function override
			if (StrEquals(token.string, STR("dmx_function")))
			{
				RequireToken(tokeniser, TOKEN_OPEN_BRACKET);
				prop.serialiseFuncOverride = RequireToken(tokeniser, TOKEN_IDENTIFIER).string;
				RequireToken(tokeniser, TOKEN_CLOSE_BRACKET);
				prop.name = RequireToken(tokeniser, TOKEN_IDENTIFIER).string;
			}
			else if (StrEquals(token.string, STR("dmx_name_override")))
			{
				RequireToken(tokeniser, TOKEN_OPEN_BRACKET);
				prop.nameOverride = RequireToken(tokeniser, TOKEN_IDENTIFIER).string;
				RequireToken(tokeniser, TOKEN_CLOSE_BRACKET);
				prop.name = RequireToken(tokeniser, TOKEN_IDENTIFIER).string; // variable name
			}
		}
		
		if (result.propertyCount < ARRAYCOUNT(result.props))
		{
			result.props[result.propertyCount++] = prop;
		}
		else
		{
			Error("Too many attributes for struct %.*s!", (s32)result.typeName.length, result.typeName.data);
		}
		
		RequireToken(tokeniser, TOKEN_SEMICOLON);
		
		// token for next loop
		token = GetToken(tokeniser);
	}
	return result;
}

#if 0

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCStoredCamera, CStoredCamera, DMX_ATTR_ELEMENT_ARRAY);

// example of generated serialisation
internal void DmxSerialiseCStoredCameras(Dmx *dmx, DmxElement *parent, str name, CStoredCameras value, Arena *arena)
{
	DmxSerialiseCStoredCamera(dmx, parent, STR("activecamera"), value.activecamera, arena);
	//DmxElement *elem = DmxAddElement(dmx, parent, name, STR("CStoredCameras"), arena);
	//DmxAddAttributeInt(dmx, elem, STR("activecamera"), {});
	
	DmxSerialiseArrayCStoredCamera(
	DmxAddAttribute(dmx, elem, STR("cameras"), DMX_ATTR_ELEMENT);
	
}
#endif

internal DmxAttrType StrToDmxAttrType(str string, bool pointer)
{
	DmxAttrType result = DMX_ATTR_UNKNOWN;
	if (pointer && StrEquals(string, STR("char"))) {result = DMX_ATTR_STRING;}
	else if (StrEquals(string, STR("DmxElementId"))) {result = DMX_ATTR_ELEMENT;}
	else if (StrEquals(string, STR("s32")) || StrEquals(string, STR("int"))) {result = DMX_ATTR_INT32;}
	else if (StrEquals(string, STR("f32")) || StrEquals(string, STR("float"))) {result = DMX_ATTR_F32;}
	else if (StrEquals(string, STR("bool"))) {result = DMX_ATTR_BOOL;}
	else if (StrEquals(string, STR("DmxBinaryBlob"))) {result = DMX_ATTR_BINARYBLOB;}
	//else if (StrEquals(string, STR("Timespan"))) {result = DMX_ATTR_TIMESPAN;} // timespan not supported right now (isn't used)
	//else if (StrEquals(string, STR("rgba8"))) {result = DMX_ATTR_RGBA8;} // rgba8 not supported right now (isn't used)
	else if (StrEquals(string, STR("v2"))) {result = DMX_ATTR_VECTOR2D;}
	else if (StrEquals(string, STR("v3"))) {result = DMX_ATTR_VECTOR3D;}
	else if (StrEquals(string, STR("v4"))) {result = DMX_ATTR_VECTOR4D;}
	else if (StrEquals(string, STR("QAngle"))) {result = DMX_ATTR_QANGLE;}
	else if (StrEquals(string, STR("quaternion"))) {result = DMX_ATTR_QUATERNION;}
	else if (StrEquals(string, STR("mat4"))) {result = DMX_ATTR_MATRIX4X4;}
	else if (StrEquals(string, STR("u64"))) {result = DMX_ATTR_UINT64;}
	else if (StrEquals(string, STR("u8"))) {result = DMX_ATTR_BYTE;}
	return result;
}

internal void GenerateSerialisation(FILE *out, DmxStruct struc)
{
	fprintf(out, "DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArray%.*s, %.*s, DMX_ATTR_ELEMENT_ARRAY);\n\n",
			struc.typeName.length, struc.typeName.data,
			struc.typeName.length, struc.typeName.data);
	fprintf(out, "internal void DmxSerialise%.*s(Dmx *dmx, DmxElement *parent, str name, %.*s value, Arena *arena)\n{\n",
			struc.typeName.length, struc.typeName.data,
			struc.typeName.length, struc.typeName.data);
	ASSERT(struc.propertyCount < ARRAYCOUNT(struc.props));
	for (s32 i = 0; i < struc.propertyCount; i++)
	{
		str nameStoredInDmx = struc.props[i].name;
		if (struc.props[i].nameOverride.data)
		{
			nameStoredInDmx = struc.props[i].nameOverride;
		}
		DmxAttrType attrType = StrToDmxAttrType(struc.props[i].type, struc.props[i].pointer);
		if (attrType != DMX_ATTR_UNKNOWN)
		{
			// TODO: default value?
			fprintf(out, "\t{\n\t\tDmxElement *elem = DmxAddAttribute%s(dmx, parent, STR(\"%.*s\"), value.%.*s);\n\t}\n",
					g_attrToString[attrType],
					nameStoredInDmx.length, nameStoredInDmx.data,
					struc.props[i].name.length, struc.props[i].name.data
					);
		}
	}
	fprintf(out, "}\n\n");
}

int main()
{
	char *inputFiles[] = {
		"../code/vmap.h",
	};
	
	char *outputFiles[] = {
		"../code/vmap_dmx_serialise.cpp",
	};
	
	for (s32 fileInd = 0; fileInd < ARRAYCOUNT(inputFiles); fileInd++)
	{
		str file = ReadEntireFileString(inputFiles[fileInd]);
		ASSERT(file.data && file.length);
		if (!file.length)
		{
			Error("Couldn't read file %s", inputFiles[fileInd]);
			continue;
		}
		
		FILE *out = fopen(outputFiles[fileInd], "wb");
		if (!out)
		{
			Error("Couldn't create output file %s", outputFiles[fileInd]);
		}
		
		fprintf(out, "%s", "\n// AUTO GENERATED, DO NOT EDIT!\n\n");
		
		Tokeniser tokeniser = {};
		tokeniser.at = file.data;
		for (;;)
		{
			Token token = GetToken(&tokeniser);
			if (token.type == TOKEN_EOS)
			{
				break;
			}
			if (token.type != TOKEN_IDENTIFIER || !StrEquals(token.string, STR("dmx_serialise")))
			{
				continue;
			}
			DmxStruct struc = ParseStruct(&tokeniser);
			GenerateSerialisation(out, struc);
			
#if 0
			printf("%.*s\n{\n", (s32)struc.typeName.length, struc.typeName.data);
			for (s32 i = 0; i < struc.propertyCount; i++)
			{
				DmxProperty prop = struc.props[i];
				printf("\tARR: %i TYPE: %-16.*s %sNAME: %-16.*s",
					   prop.array,
					   (s32)prop.type.length,
					   prop.type.data,
					   prop.pointer ? "* " : "",
					   (s32)prop.name.length,
					   prop.name.data);
				if (prop.nameOverride.length)
				{
					printf(" NAME_OVERRIDE: %.*s", (s32)prop.nameOverride.length, prop.nameOverride.data);
				}
				if (prop.serialiseFuncOverride.length)
				{
					printf(" SERIALISATION_FUNC_OVERRIDE: %.*s", (s32)prop.serialiseFuncOverride.length, prop.serialiseFuncOverride.data);
				}
				printf("\n");
			}
			printf("}\n");
#endif
		}
		fclose(out);
		free(file.data);
	}
	
	return 0;
}