
internal void DebugGfxAddTexture(u8 *data, s32 width, s32 height, b32 rgb888 = false) {}

internal void DebugGfxAddFace(Verts *poly, v3 normal, s32 textureIndex, v4 s, v4 t) {}

internal void DebugGfxAddBrushSide(Verts *poly, v3 normal, s32 textureIndex = -1, v4 s = {}, v4 t = {}) {}

internal void DebugGfxAddBrush(s32 sideCount) {}

internal void DebugGfxAddMesh(Verts *poly, v3 normal, s32 textureIndex, v4 s, v4 t) {}

