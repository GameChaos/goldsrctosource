
const std = @import("std");

pub fn build(b: *std.Build) void
{
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimise = b.standardOptimizeOption(.{});
    
	const imgui = b.addStaticLibrary(.{
        .name = "imgui",
        .target = target,
        .optimize = .ReleaseFast,
        .link_libc = true,
    });
    imgui.linkLibCpp();
    
    imgui.addCSourceFiles(.{
        .root = b.path("code/imgui"),
        .files = &.{
            "imgui.cpp", "imgui_draw.cpp",
            "imgui_tables.cpp", "imgui_widgets.cpp", "imgui_demo.cpp"
        },
    });
    
    imgui.addIncludePath(b.path("code/imgui/"));
    //imgui.installHeader(b.path("code/imgui.h"), "imgui.h");
	
	if (b.default_step.addDirectoryWatchInput(.{.cwd_relative = "code/*"}) catch @panic("failed to watch dir"))
	{
		
	}
    
	// TODO: sokol-shdc!!!!!!
    const exe = b.addExecutable(.{
        .name = "goldsrctosource",
        .target = target,
        .optimize = optimise,
        .link_libc = true,
    });
	
	if (target.result.os.tag == .windows)
	{
		exe.addCSourceFile(.{
			.file = b.path("code/win32_goldsrctosource.cpp"),
		});
    }
	else if (target.result.os.tag == .linux)
	{
		exe.addCSourceFile(.{
			.file = b.path("code/linux_goldsrctosource.cpp"),
		});
	}
    b.installArtifact(exe);
}
