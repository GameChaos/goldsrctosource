
internal void EatWhiteSpace_(EntlumpTokeniser *tokeniser)
{
	while (IsWhiteSpace(tokeniser->at[0]))
	{
		tokeniser->at++;
	}
}

internal str EntlumpGetString_(EntlumpTokeniser *tokeniser, s32 length)
{
	str result = {};
	result.length = length;
	result.data = tokeniser->at;
	
	return result;
}

internal EntlumpToken EntlumpGetToken(EntlumpTokeniser *tokeniser)
{
	EntlumpToken result = {};
	
	EatWhiteSpace_(tokeniser);
	
	switch (*tokeniser->at)
	{
		case '{':
		{
			result.type = ENTLUMPTOKEN_OPEN_BRACE;
			result.string = EntlumpGetString_(tokeniser, 1);
			tokeniser->at++;
		} break;
		
		case '}':
		{
			result.type = ENTLUMPTOKEN_CLOSE_BRACE;
			result.string = EntlumpGetString_(tokeniser, 1);
			tokeniser->at++;
		} break;
		
		case '\0':
		{
			result.type = ENTLUMPTOKEN_EOS;
		} break;
		
		case '\"':
		{
			result.type = ENTLUMPTOKEN_IDENTIFIER;
			s32 identifierLen = 0;
			tokeniser->at++;
			result.string.data = tokeniser->at;
			for (s32 i = 0; i < S32_MAX; i++)
			{
				if (*tokeniser->at == '\"')
				{
					break;
				}
				if (*tokeniser->at == '\0')
				{
					result.type = ENTLUMPTOKEN_EOS;
					break;
				}
				tokeniser->at++;
			}
			
			result.string.length = tokeniser->at - result.string.data;
			// NOTE(GameChaos): skip last quotation mark
			tokeniser->at++;
		} break;
		
		default:
		{
			result.type = ENTLUMPTOKEN_UNKNOWN;
			result.string = EntlumpGetString_(tokeniser, 1);
		} break;
	}
	
	return result;
}

// TODO: DON'T ASSUME NULL TERMINATED!!!!!!!!!!!!!!!!
internal b32 EntlumpParseEntity_(EntProperties *ent, EntlumpTokeniser *tokeniser)
{
	b32 result = false;
	*ent = {};
	EntlumpToken token;
	token = EntlumpGetToken(tokeniser);
	if (token.type == ENTLUMPTOKEN_EOS)
	{
		return result;
	}
	
	for (;;)
	{
		token = EntlumpGetToken(tokeniser);
		switch (token.type)
		{
			case ENTLUMPTOKEN_OPEN_BRACE:
			{
				continue;
			} break;
			
			case ENTLUMPTOKEN_IDENTIFIER:
			{
				EntlumpToken valueToken;
				valueToken = EntlumpGetToken(tokeniser);
				if (valueToken.type == ENTLUMPTOKEN_IDENTIFIER)
				{
					if (StrEquals(token.string, STR("classname"), false))
					{
						if (!ent->classname.data)
						{
							ent->classname = valueToken.string;
						}
					}
					else
					{
						ent->properties[ent->propertyCount].key = token.string;
						ent->properties[ent->propertyCount].value = valueToken.string;
						ent->propertyCount++;
					}
				}
				else
				{
					Error("Entity lump: Unexpected token \"%.*s\".",
						  valueToken.string.length, valueToken.string.data);
					goto end;
				}
			} break;
			
			case ENTLUMPTOKEN_CLOSE_BRACE:
			{
				result = true;
				goto end;
			} break;
			
			case ENTLUMPTOKEN_EOS:
			{
				Error("Entity lump: Unexpected end of entity lump! Entity lump parsing failed.");
				goto end;
			} break;
			
			default:
			{
				Error("Entity lump: Invalid token \"%.*s\". Entity lump parsing failed.",
					  token.string.length,token.string.data);
				goto end;
			} break;
		}
	}
	end:
	return result;
}

internal EntList GsrcParseEntities(Arena *arena, str entLump)
{
	EntProperties ent = {};
	EntList result = {};
	EntlumpTokeniser tokeniser = {entLump.data};
	result.ents = (EntProperties *)ArenaAlloc(arena, sizeof(*result.ents) * MAX_ENTITIES);
	while (EntlumpParseEntity_(&ent, &tokeniser) && result.entCount <= MAX_ENTITIES)
	{
		result.ents[result.entCount++] = ent;
#if 0
		// print entity properties for debugging
		PrintString("{\n");
		for (s32 i = 0; i < ent.propertyCount; i++)
		{
			Print("\t\"%.*s\" \"%.*s\"\n", ent.properties[i].key.length, ent.properties[i].key.data,
				  ent.properties[i].value.length, ent.properties[i].value.data);
		}
		PrintString("}\n");
#endif
	}
	return result;
}

internal void EntPushProp(EntProperties *out, str key, str value)
{
	out->properties[out->propertyCount++] = {key, value};
}

internal ModelInfo EntConvertCommonBrush(Arena *arena, EntProperties *gsrcEnt, EntProperties *out)
{
	ModelInfo result = {};
	result.model = -1;
	result.rendermode = -1;
	for (s32 prop = 0; prop < gsrcEnt->propertyCount; prop++)
	{
		if (StrEquals(gsrcEnt->properties[prop].key, STR("angles"), false))
		{
			EntPushProp(out, STR("movedir"), gsrcEnt->properties[prop].value);
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("wait"), false))
		{
			EntPushProp(out, STR("wait"), gsrcEnt->properties[prop].value);
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("model"), false))
		{
			EntPushProp(out, STR("model"), gsrcEnt->properties[prop].value);
			result.model = -1;
			if (StringToS32(gsrcEnt->properties[prop].value.data + 1, &result.model) == STRINGTONUM_SUCCESS
				&& result.model >= 0 && result.model < SRC_MAX_MAP_MODELS)
			{
				out->model = result.model;
			}
			else
			{
				ASSERT(0);
				result.model = -1;
			}
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("speed"), false))
		{
			EntPushProp(out, STR("speed"), gsrcEnt->properties[prop].value);
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("lip"), false))
		{
			EntPushProp(out, STR("lip"), gsrcEnt->properties[prop].value);
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("rendermode"), false))
		{
			EntPushProp(out, STR("rendermode"), gsrcEnt->properties[prop].value);
			result.rendermode = -1;
			StringToS32(gsrcEnt->properties[prop].value.data, &result.rendermode);
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("renderamt"), false))
		{
			EntPushProp(out, STR("renderamt"), gsrcEnt->properties[prop].value);
		}
		// TODO: rendercolor is a little bit differenet in source?
#if 0
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("rendercolor"), false))
		{
			EntPushProp(out, STR("rendercolor"), gsrcEnt->properties[prop].value);
		}
#endif
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("health"), false))
		{
			EntPushProp(out, STR("health"), gsrcEnt->properties[prop].value);
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("targetname"), false))
		{
			EntPushProp(out, STR("targetname"), gsrcEnt->properties[prop].value);
		}
		else if (StrEquals(gsrcEnt->properties[prop].key, STR("target"), false))
		{
			EntPushProp(out, STR("target"), gsrcEnt->properties[prop].value);
		}
		EntPushProp(out, STR("origin"), STR("0 0 0"));
	}
	return result;
}

internal void EntConvertOneToOne(EntProperties *gsrcEnt, EntProperties *out)
{
	for (s32 prop = 0; prop < gsrcEnt->propertyCount; prop++)
	{
		EntPushProp(out, gsrcEnt->properties[prop].key, gsrcEnt->properties[prop].value);
	}
}
