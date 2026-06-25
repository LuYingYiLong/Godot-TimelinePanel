---
name: toolchain-environment
description: 开发环境工具链路径和版本信息
metadata:
  type: reference
---

## 工具链

| 工具 | 路径 | 版本 |
|------|------|------|
| Godot 编辑器 | `D:\Godot_v4.7-stable_win64.exe\Godot_v4.7-stable_win64.exe` | 4.7 stable |
| Godot 控制台 | `D:\Godot_v4.7-stable_win64.exe\Godot_v4.7-stable_win64_console.exe` | 4.7 stable |
| MSVC | `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\` | 14.44 |
| dumpbin | `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe` | VS 2026 |
| PowerShell | `C:\Program Files\PowerShell\7\pwsh.exe` | 7+ |
| SCons | PATH（由 pip 安装） | — |

## 关键路径

- GDExtension 项目根：`D:\GodotProjects\Godot-TimelinePanel`
- 测试 Godot 项目：`D:\GodotProjects\example`
- Godot 用户数据日志：`C:\Users\绿影翼龙\AppData\Roaming\Godot\app_userdata\Lightech\logs\godot.log`
- WER crash dump：`%LOCALAPPDATA%\CrashDumps\`

## 缺失工具

- **cdb.exe**：未安装 Windows Debugging Tools — 用 VS 图形调试器或 dumpbin 代替
- **WinDbg**：未安装
- **procdump**：未安装
