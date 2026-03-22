#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/"])

# Collect C++ sources
sources = Glob("src/*.cpp")

if env["platform"] == "android":
    sources += [env.SharedObject(f) for f in giflib_sources]
else:
    sources += [env.Object(f) for f in giflib_sources]

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "addons/timeline_panel/bin/libtimeline_panel.{}.{}.framework/libtimeline_panel.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "addons/timeline_panel/bin/libtimeline_panel.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "addons/timeline_panel/bin/libtimeline_panel.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )

elif env["platform"] == "android":
    library = env.SharedLibrary(
        "addons/timeline_panel/bin/libtimeline_panel{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

else:
    library = env.SharedLibrary(
        "addons/timeline_panel/bin/timeline_panel{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
