const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const examples = [_][]const u8{
        "and_then",
        "arithmetic",
        "complex_data",
        "error_codes",
        "file_operation",
        "or_else",
        "transform",
    };

    const run_all = b.step("run-all", "Run all examples");

    inline for (examples) |name| {
        const exe = b.addExecutable(.{
            .name = name,
            .target = target,
            .optimize = optimize,
        });
        exe.linkLibCpp();
        exe.addIncludePath(b.path("."));
        exe.addCSourceFile(.{
            .file = b.path(b.fmt("examples/{s}.cpp", .{name})),
            .flags = &[_][]const u8{
                "-std=c++20",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-Wpedantic",
            },
        });

        b.installArtifact(exe);

        const run_cmd = b.addRunArtifact(exe);
        if (b.args) |args| run_cmd.addArgs(args);

        const run_step = b.step(b.fmt("run-{s}", .{name}), b.fmt("Run {s} example", .{name}));
        run_step.dependOn(&run_cmd.step);
        run_all.dependOn(run_step);
    }
}
