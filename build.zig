
const std = @import("std");
const assert = std.debug.assert;

pub fn build(b: *std.Build) !void
{
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimise = b.standardOptimizeOption(.{});
    
	_ = try b.default_step.addDirectoryWatchInput(.{.cwd_relative = "code/*"});
    
    const exe = b.addExecutable(.{
        .name = "goldsrctosource",
        .target = target,
        .optimize = optimise,
        .link_libc = true,
	});
	
	if (optimise == .Debug)
	{
		exe.defineCMacro("GC_DEBUG", "");
		const debugGraphics = true;
		if (debugGraphics)
		{
			const imgui = b.addStaticLibrary(.{
				.name = "imgui",
				.target = target,
				.optimize = .ReleaseFast,
				.link_libc = true,
			});
			
			imgui.addCSourceFiles(.{
				.root = b.path("code/imgui"),
				.files = &.{
					"imgui.cpp", "imgui_draw.cpp",
					"imgui_tables.cpp", "imgui_widgets.cpp", "imgui_demo.cpp"
				},
			});
			
			imgui.addIncludePath(b.path("code/imgui/"));
			imgui.linkLibCpp();
			
			const dep_sokol_shdc = b.dependency("sokol_shdc", .{});
			const sokolShdc = b.addSystemCommand(&.{"bin/linux/sokol-shdc", "--help"});
			sokolShdc.setCwd(dep_sokol_shdc.path(""));
			b.default_step.dependOn(&sokolShdc.step);
			
			exe.linkLibrary(imgui);
			// TODO: sokol-shdc!!!!!!
			exe.defineCMacro("DEBUG_GRAPHICS", "");
			if (target.result.os.tag == .windows)
			{
				exe.linkSystemLibrary("gdi32");
			}
			else if (target.result.os.tag == .linux)
			{
				exe.linkSystemLibrary("x11");
				exe.linkSystemLibrary("xi");
				exe.linkSystemLibrary("xcursor");
				exe.linkSystemLibrary("gl");
			}
		}
		if (target.result.os.tag == .linux)
		{
			exe.linkSystemLibrary("ubsan");
		}
	}
	
	exe.linkLibC();
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
			.flags = &.{"-fno-sanitize-trap=undefined"}
		});
	}
    b.installArtifact(exe);
}
