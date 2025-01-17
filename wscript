# -*- mode: python -*-

APPNAME = "Antares"
VERSION = "0.5.1"

WARNINGS = ["-Wall", "-Werror", "-Wno-sign-compare"]

def common(ctx):
    ctx.load("compiler_c compiler_cxx")
    ctx.load("core externals", tooldir="ext/waf-sfiera")
    ctx.load("antares_test", tooldir="tools")
    ctx.external("libpng libsfz libzipxx rezin")

def dist(dst):
    dst.algo = "zip"
    dst.excl = (
        " test **/.* **/*.zip **/*.pyc **/build ext/*/ext"
        " ext/libsfz/waf ext/libzipxx/waf ext/rezin/waf"
    )

def options(opt):
    common(opt)

def configure(cnf):
    common(cnf)

    cnf.env.append_value("FRAMEWORK_antares/system/audio-toolbox", "AudioToolbox")
    cnf.env.append_value("FRAMEWORK_antares/system/cocoa", "Cocoa")
    cnf.env.append_value("FRAMEWORK_antares/system/carbon", "Carbon")
    cnf.env.append_value("FRAMEWORK_antares/system/core-foundation", "CoreFoundation")
    cnf.env.append_value("FRAMEWORK_antares/system/openal", "OpenAL")
    cnf.env.append_value("FRAMEWORK_antares/system/opengl", "OpenGL")

def build(bld):
    common(bld)

    bld(
        rule="${SRC} %s %s ${TGT}" % (VERSION, bld.options.sdk),
        source="scripts/generate-info-plist.py",
        target="antares/Info.plist",
    )

    bld.program(
        target="antares/Antares",
        mac_app=True,
        mac_plist="antares/Info.plist",
        mac_resources=[
            "resources/Antares.icns",
            "resources/ExtractData.nib",
            "resources/MainMenu.nib",
        ],
        source=[
            "src/cocoa/AntaresController.m",
            "src/cocoa/AntaresExtractDataController.m",
            "src/cocoa/main.m",
            "src/cocoa/video-driver.cpp",
            "src/cocoa/core-foundation.cpp",
            "src/cocoa/http.cpp",
            "src/cocoa/prefs-driver.cpp",
            "src/cocoa/c/AntaresController.cpp",
            "src/cocoa/c/CocoaVideoDriver.m",
            "src/cocoa/c/DataExtractor.cpp",
            "src/cocoa/c/scenario-list.cpp",
        ],
        cflags=WARNINGS,
        cxxflags=WARNINGS,
        arch="i386 ppc",
        use=[
            "antares/libantares",
            "antares/system/cocoa",
            "antares/system/carbon",
        ],
    )

    bld.program(
        target="antares/offscreen",
        source=[
            "src/bin/offscreen.cpp",
            "src/video/offscreen-driver.cpp",
        ],
        cxxflags=WARNINGS,
        use=[
            "antares/system/opengl",
            "antares/opengl",
            "antares/libantares",
        ],
    )

    bld.program(
        target="antares/replay",
        source=[
            "src/bin/replay.cpp",
            "src/video/offscreen-driver.cpp",
        ],
        cxxflags=WARNINGS,
        use=[
            "antares/system/opengl",
            "antares/opengl",
            "antares/libantares",
        ],
    )

    bld.program(
        target="antares/build-pix",
        source="src/bin/build-pix.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares",
    )

    bld.platform(
        target="antares/build-pix",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.program(
        target="antares/ls-scenarios",
        source="src/bin/ls-scenarios.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares",
    )

    bld.program(
        target="antares/object-data",
        source="src/bin/object-data.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares",
    )

    bld.platform(
        target="antares/object-data",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.program(
        target="antares/extract-data",
        source=[
            "src/bin/extract-data.cpp",
        ],
        cxxflags=WARNINGS,
        use=[
            "antares/libantares",
        ],
    )

    bld.program(
        target="antares/hash-data",
        source="src/bin/hash-data.cpp",
        cxxflags=WARNINGS,
        use="libsfz/libsfz",
    )

    bld.platform(
        target="antares/extract-data",
        source=[
            "src/cocoa/core-foundation.cpp",
            "src/cocoa/http.cpp",
        ],
        platform="darwin",
        arch="i386 ppc",
        use="antares/system/core-foundation",
    )

    bld(
        target="antares/libantares",
        use=[
            "antares/libantares-config",
            "antares/libantares-data",
            "antares/libantares-drawing",
            "antares/libantares-game",
            "antares/libantares-math",
            "antares/libantares-sound",
            "antares/libantares-ui",
            "antares/libantares-video",
        ],
    )

    bld.stlib(
        target="antares/libantares-config",
        source=[
            "src/config/keys.cpp",
            "src/config/ledger.cpp",
            "src/config/preferences.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.platform(
        target="antares/libantares-config",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.stlib(
        target="antares/libantares-data",
        source=[
            "src/data/extractor.cpp",
            "src/data/interface.cpp",
            "src/data/picture.cpp",
            "src/data/races.cpp",
            "src/data/replay.cpp",
            "src/data/replay-list.cpp",
            "src/data/resource.cpp",
            "src/data/scenario.cpp",
            "src/data/scenario-list.cpp",
            "src/data/space-object.cpp",
            "src/data/string-list.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "libpng/libpng",
            "libsfz/libsfz",
            "rezin/librezin",
            "zipxx/libzipxx",
        ],
    )

    bld.platform(
        target="antares/libantares-data",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.stlib(
        target="antares/libantares-drawing",
        source=[
            "src/drawing/briefing.cpp",
            "src/drawing/build-pix.cpp",
            "src/drawing/color.cpp",
            "src/drawing/interface.cpp",
            "src/drawing/interface-text.cpp",
            "src/drawing/libpng-pix-map.cpp",
            "src/drawing/offscreen-gworld.cpp",
            "src/drawing/pix-map.cpp",
            "src/drawing/pix-table.cpp",
            "src/drawing/retro-text.cpp",
            "src/drawing/shapes.cpp",
            "src/drawing/sprite-handling.cpp",
            "src/drawing/text.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "libpng/libpng",
            "libsfz/libsfz",
        ],
    )

    bld.platform(
        target="antares/libantares-drawing",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.stlib(
        target="antares/libantares-game",
        source=[
            "src/game/admiral.cpp",
            "src/game/beam.cpp",
            "src/game/cheat.cpp",
            "src/game/cursor.cpp",
            "src/game/globals.cpp",
            "src/game/input-source.cpp",
            "src/game/instruments.cpp",
            "src/game/labels.cpp",
            "src/game/main.cpp",
            "src/game/messages.cpp",
            "src/game/minicomputer.cpp",
            "src/game/motion.cpp",
            "src/game/non-player-ship.cpp",
            "src/game/player-ship.cpp",
            "src/game/scenario-maker.cpp",
            "src/game/space-object.cpp",
            "src/game/starfield.cpp",
            "src/game/time.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.platform(
        target="antares/libantares-game",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.stlib(
        target="antares/libantares-math",
        source=[
            "src/math/fixed.cpp",
            "src/math/geometry.cpp",
            "src/math/random.cpp",
            "src/math/rotation.cpp",
            "src/math/special.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.platform(
        target="antares/libantares-math",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.stlib(
        target="antares/libantares-sound",
        source=[
            "src/sound/driver.cpp",
            "src/sound/fx.cpp",
            "src/sound/music.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.platform(
        target="antares/libantares-sound",
        platform="darwin",
        source="src/sound/openal-driver.cpp",
        arch="i386 ppc",
        use=[
            "antares/system/audio-toolbox",
            "antares/system/openal",
        ],
    )

    bld.stlib(
        target="antares/libantares-ui",
        source=[
            "src/ui/card.cpp",
            "src/ui/event.cpp",
            "src/ui/event-tracker.cpp",
            "src/ui/flows/master.cpp",
            "src/ui/flows/replay-game.cpp",
            "src/ui/flows/solo-game.cpp",
            "src/ui/interface-handling.cpp",
            "src/ui/interface-screen.cpp",
            "src/ui/screens/briefing.cpp",
            "src/ui/screens/debriefing.cpp",
            "src/ui/screens/help.cpp",
            "src/ui/screens/main.cpp",
            "src/ui/screens/options.cpp",
            "src/ui/screens/play-again.cpp",
            "src/ui/screens/scroll-text.cpp",
            "src/ui/screens/select-level.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.platform(
        target="antares/libantares-ui",
        platform="darwin",
        arch="i386 ppc",
    )

    bld.stlib(
        target="antares/libantares-video",
        source=[
            "src/video/driver.cpp",
            "src/video/transitions.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "libpng/libpng",
            "libsfz/libsfz",
        ],
    )

    bld.platform(
        target="antares/libantares-video",
        platform="darwin",
        source=[
            "src/cocoa/core-opengl.cpp",
            "src/video/opengl-driver.cpp",
        ],
        arch="i386 ppc",
        use="antares/system/opengl",
    )

    bld.antares_test(
        target="antares/build-pix",
        rule="antares/build-pix",
        expected="test/build-pix",
    )

    bld.antares_test(
        target="antares/object-data",
        rule="antares/object-data",
        expected="test/object-data",
    )

    def regtest(name):
        bld.antares_test(
            target="antares/%s" % name,
            rule="antares/offscreen %s" % name,
            expected="test/%s" % name,
        )

    regtest("main-screen")
    regtest("mission-briefing")
    regtest("options")
    regtest("pause")

    def replay_test(name):
        bld.antares_test(
            target="antares/replay/%s" % name,
            rule="antares/replay",
            srcs="test/%s.NLRP" % name,
            expected="test/%s" % name,
        )

    replay_test("and-it-feels-so-good")
    replay_test("blood-toil-tears-sweat")
    replay_test("hand-over-fist")
    replay_test("make-way")
    replay_test("out-of-the-frying-pan")
    replay_test("space-race")
    replay_test("the-left-hand")
    replay_test("the-mothership-connection")
    replay_test("the-stars-have-ears")
    replay_test("while-the-iron-is-hot")
    replay_test("yo-ho-ho")
    replay_test("you-should-have-seen-the-one-that-got-away")
