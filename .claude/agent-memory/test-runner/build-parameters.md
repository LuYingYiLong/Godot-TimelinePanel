---
name: build-parameters
description: SCons 构建命令和常用参数
metadata:
  type: project
---

## 构建命令

```powershell
# Debug 构建
cd D:\GodotProjects\Godot-TimelinePanel && scons target=template_debug -j8

# Release 构建
cd D:\GodotProjects\Godot-TimelinePanel && scons target=template_release -j8

# 清理构建
cd D:\GodotProjects\Godot-TimelinePanel && scons -c target=template_debug
```

## 输出位置

- Debug: `addons/timeline_panel/bin/timeline_panel.windows.template_debug.x86_64.dll`
- Release: `addons/timeline_panel/bin/timeline_panel.windows.template_release.x86_64.dll`

## 构建系统

- **构建工具**：SCons（SConstruct 在项目根）
- **编译器**：MSVC 14.44（通过 scons 自动检测）
- **godot-cpp**：子模块，分支 `4.5`，需预编译 `.lib` 文件
- **C++ 标准**：C++20
- **平台**：Windows x86_64

## 编译验证

```powershell
# 第1步：编译
cd D:\GodotProjects\Godot-TimelinePanel && scons target=template_debug -j8

# 第2步：验证 DLL 已生成
Test-Path "addons\timeline_panel\bin\timeline_panel.windows.template_debug.x86_64.dll"

# 第3步：冒烟测试 — Godot 无头加载不崩溃
& "D:\Godot_v4.7-stable_win64.exe\Godot_v4.7-stable_win64_console.exe" --headless --disable-crash-handler --path "D:\GodotProjects\example" --quit
```

**Why:** Godot-TimelinePanel 是 GDExtension，没有独立的 C++ 测试框架。C++ 层的正确性通过编译通过 + Godot 加载不崩溃 + GDScript 测试调用 C++ 方法验证。

**How to apply:** 每次修改 C++ 代码后运行上述三步验证流程。
