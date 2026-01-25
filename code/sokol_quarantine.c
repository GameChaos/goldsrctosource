
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
#include "imgui/cimgui.h"
#include "sokol/sokol_imgui.h"