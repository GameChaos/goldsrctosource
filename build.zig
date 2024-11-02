
const std = @import("std");
const builtin = @import("builtin");
const assert = std.debug.assert;

fn AddShaderBuildCommand(b: *std.Build, dep_sokol_shdc: *std.Build.Dependency, inputPath: []const u8, outputPath: []const u8) *std.Build.Step.Run
{
	const shdcPath = switch (builtin.os.tag) {
		.windows => "bin/win32/sokol-shdc.exe",
		.linux => "bin/linux/sokol-shdc",
		.macos => switch (builtin.cpu.arch) {
			.aarch64 =>"bin/osx_arm64/sokol-shdc",
			.x86_64 =>"bin/osx/sokol-shdc",
			else => @compileError("Sokol-shdc doesn't support this osx cpu's arhitecture."),
		},
		else => @compileError("Sokol-shdc is not supported on this machine. Shaders can't be compiled."),
	};
	const sokolShdc = b.addSystemCommand(&.{shdcPath, "--input"});
	sokolShdc.setCwd(dep_sokol_shdc.path(""));
	sokolShdc.addFileArg(b.path(inputPath));
	sokolShdc.addArg("--output");
	sokolShdc.addFileArg(b.path(outputPath));
	sokolShdc.addArgs(&.{"--slang", "glsl410:hlsl4:metal_macos", "-b"});
	
	return sokolShdc;
}

pub fn build(b: *std.Build) !void
{
    const target = b.standardTargetOptions(.{});
	
	if (target.result.os.tag != .linux and target.result.os.tag != .windows)
	{
		std.debug.print("{} is not supported as a build target.\n", .{target.result.os.tag});
		std.process.exit(1);
	}
	
	if (target.result.cpu.arch != .x86_64)
	{
		std.debug.print("Only x86_64 is supported as a build target.\n", .{});
		std.process.exit(1);
	}
	
	// TODO: ABI check?

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
					"imgui_tables.cpp", "imgui_widgets.cpp", "imgui_demo.cpp",
					"cimgui.cpp"
				},
			});
			
			imgui.addIncludePath(b.path("code/imgui/"));
			imgui.addIncludePath(b.path("code/"));
			imgui.linkLibCpp();
			
			
			const dep_sokol_shdc = b.dependency("sokol_shdc", .{});
			
			const worldShader = AddShaderBuildCommand(b, dep_sokol_shdc,
													  "code/shaders/world.glsl",
													  "code/shaders_compiled/world.h");
			const wireShader = AddShaderBuildCommand(b, dep_sokol_shdc,
													  "code/shaders/wire.glsl",
													  "code/shaders_compiled/wire.h");
			b.default_step.dependOn(&worldShader.step);
			b.default_step.dependOn(&wireShader.step);
			
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
				exe.linkSystemLibrary("rt");
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
			.file = b.path("code/win32_goldsrctosource.c"),
			.flags = &.{"-std=c23"},
		});
    }
	else if (target.result.os.tag == .linux)
	{
		exe.addCSourceFile(.{
			.file = b.path("code/linux_goldsrctosource.c"),
			.flags = &.{"-fno-sanitize-trap=undefined", "-std=c23"}
		});
	}
    b.installArtifact(exe);
}
