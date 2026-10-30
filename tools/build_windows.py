#!/usr/bin/env python3
"""Cross-build the native simulator and a verified Windows x64 portable ZIP.

Requires CMake, Ninja, MinGW-w64 and curl on macOS/Linux. Resources must already
be generated; this command never regenerates art or builds/flashes firmware.
"""
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]
SDL_VERSION = "2.32.10"
SDL_SHA256 = "83a5d74012311edc3c0d40ea6faecbe57ad692aa033fa5dc273cc937e3938ff2"
BUILD = ROOT / "build/windows-x64"
DEPS = ROOT / "build/windows-deps"
STAGE = ROOT / "build/windows-package/PokemonPlus"
SYSTEM_DLLS = {
    "kernel32.dll", "msvcrt.dll", "user32.dll", "gdi32.dll", "winmm.dll",
    "imm32.dll", "ole32.dll", "oleaut32.dll", "shell32.dll", "setupapi.dll",
    "version.dll", "advapi32.dll", "ntdll.dll", "comdlg32.dll", "comctl32.dll",
    "ws2_32.dll", "ucrtbase.dll", "bcrypt.dll", "dwmapi.dll", "hid.dll",
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(args, log=None):
    if log is None:
        return subprocess.check_output([str(a) for a in args], text=True)
    with log.open("w") as stream:
        result = subprocess.run([str(a) for a in args], stdout=stream, stderr=subprocess.STDOUT)
    if result.returncode:
        raise RuntimeError(f"{args[0]} failed; see {log}\n{log.read_text()[-6000:]}")


def pe_check(path, gui=False):
    data = path.read_bytes()
    if data[:2] != b"MZ":
        raise ValueError(f"Not a Windows executable: {path}")
    offset = struct.unpack_from("<I", data, 0x3c)[0]
    if data[offset:offset + 4] != b"PE\0\0" or struct.unpack_from("<H", data, offset + 4)[0] != 0x8664:
        raise ValueError(f"Not Windows x64 PE: {path}")
    if gui and struct.unpack_from("<H", data, offset + 24 + 68)[0] != 2:
        raise ValueError("Application must use the Windows GUI subsystem")


def main():
    for command in ("cmake", "ninja", "curl", "tar", "x86_64-w64-mingw32-gcc", "x86_64-w64-mingw32-g++", "x86_64-w64-mingw32-objdump", "x86_64-w64-mingw32-strip"):
        if not shutil.which(command):
            raise SystemExit(f"Missing {command}. On macOS: brew install cmake ninja mingw-w64")
    DEPS.mkdir(parents=True, exist_ok=True)
    archive = DEPS / f"SDL2-devel-{SDL_VERSION}-mingw.tar.gz"
    if not archive.exists() or digest(archive) != SDL_SHA256:
        run(["curl", "-fLsS", "--connect-timeout", "15", "--max-time", "300", "--retry", "2",
             f"https://www.libsdl.org/release/{archive.name}", "-o", archive])
    if digest(archive) != SDL_SHA256:
        raise ValueError("SDL2 download SHA256 mismatch")
    run(["tar", "-xzf", archive, "-C", DEPS])
    sdl = DEPS / f"SDL2-{SDL_VERSION}"
    prefix = sdl / "x86_64-w64-mingw32"
    BUILD.mkdir(parents=True, exist_ok=True)
    print("Building Windows x64 simulator…", flush=True)
    run(["cmake", "-S", ROOT / "preview", "-B", BUILD, "-G", "Ninja",
         f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'preview/mingw-w64.cmake'}",
         f"-DSDL2_DIR={prefix / 'lib/cmake/SDL2'}", "-DCMAKE_BUILD_TYPE=Release",
         "-DCMAKE_C_FLAGS_RELEASE=-O2", "-DBUILD_SHARED_LIBS=OFF"], BUILD / "configure.log")
    # Assertions are intentional simulator guards; do not define NDEBUG.
    run(["cmake", "--build", BUILD, "--target", "pokemon_plus", "-j", "8"], BUILD / "build.log")
    STAGE.mkdir(parents=True, exist_ok=True)
    packaged = []

    def copy(source, relative):
        dest = STAGE / relative
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, dest)
        packaged.append(relative)
        return dest

    exe = copy(BUILD / "PokemonPlus.exe", "PokemonPlus.exe")
    run(["x86_64-w64-mingw32-strip", exe])
    pe_check(exe, gui=True)
    copy(prefix / "bin/SDL2.dll", "SDL2.dll")
    imports = {}
    pending = ["PokemonPlus.exe", "SDL2.dll"]
    while pending:
        name = pending.pop()
        pe_check(STAGE / name)
        dump = run(["x86_64-w64-mingw32-objdump", "-p", STAGE / name])
        imports[name] = re.findall(r"DLL Name:\s*(\S+)", dump)
        for dependency in imports[name]:
            if dependency.lower() in SYSTEM_DLLS or dependency.lower().startswith(("api-ms-win-", "ext-ms-win-")):
                continue
            if dependency in packaged:
                continue
            candidate = Path(run(["x86_64-w64-mingw32-gcc", f"-print-file-name={dependency}"]).strip())
            if not candidate.is_file():
                raise ValueError(f"Missing non-system DLL: {dependency}")
            copy(candidate, dependency)
            pending.append(dependency)

    manifest = json.loads((ROOT / "sdcard/pokemon-plus/manifest.json").read_text())
    assets = [row for row in manifest["files"] if row["path"].endswith(".bin")]
    actual = {p.relative_to(ROOT / "sdcard/pokemon-plus").as_posix() for p in (ROOT / "sdcard/pokemon-plus").rglob("*.bin")}
    if actual != {row["path"] for row in assets}:
        raise ValueError("Resource files differ from the generated manifest")
    for row in assets:
        source = ROOT / "sdcard/pokemon-plus" / row["path"]
        if source.stat().st_size != row["bytes"] or digest(source) != row["sha256"]:
            raise ValueError(f"Resource mismatch: {row['path']}")
        copy(source, "resources/" + row["path"])
    copy(sdl / "LICENSE.txt", "licenses/SDL2.txt")
    copy(ROOT / "managed_components/lvgl__lvgl/LICENCE.txt", "licenses/LVGL.txt")
    version = re.search(r'set\(PROJECT_VER "([^"]+)"\)', (ROOT / "CMakeLists.txt").read_text())[1]
    readme = STAGE / "使用说明.txt"
    readme.write_text(
        f"Pokemon Plus {version} · Windows x64 模拟器\n\n"
        "解压整个 ZIP，双击 PokemonPlus.exe。请保留 resources 文件夹和 SDL2.dll。\n"
        "鼠标点击操作，按住宠物一秒查看状态。\n"
        "游戏画面始终圆形；拖动窗口边缘缩放，或用下方 - / + 调整圆屏大小，点百分比还原至100%。\n"
        "圆屏下方四个按钮：+100步、摇一摇、+1小时、回庭院。\n"
        "K：增加100步；S：模拟摇晃；空格：推进一小时；Esc：返回庭院。\n"
        "当前沿用原生预览：游戏进度仅在本次运行内保留，关闭后重置；不连接或修改设备。\n",
        encoding="utf-8-sig")
    packaged.append(readme.name)
    checksums = {name: digest(STAGE / name) for name in sorted(packaged)}
    (STAGE / "SHA256SUMS.txt").write_text("".join(f"{sha}  {name}\n" for name, sha in checksums.items()))
    packaged.append("SHA256SUMS.txt")
    output = ROOT / "dist/pokemon-plus-windows-x64.zip"
    output.parent.mkdir(exist_ok=True)
    temp = output.with_suffix(".tmp.zip")
    with zipfile.ZipFile(temp, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as zipped:
        for name in sorted(packaged):
            zipped.write(STAGE / name, "PokemonPlus/" + name)
    with zipfile.ZipFile(temp) as zipped:
        if zipped.testzip():
            raise ValueError("ZIP CRC check failed")
        for name, sha in checksums.items():
            if hashlib.sha256(zipped.read("PokemonPlus/" + name)).hexdigest() != sha:
                raise ValueError(f"ZIP hash mismatch: {name}")
    temp.replace(output)
    audit = {"version": version, "architecture": "Windows x64 GUI", "resources": len(assets),
             "resourceBytes": sum(row["bytes"] for row in assets), "imports": imports,
             "zipSha256": digest(output), "zipBytes": output.stat().st_size,
             "peCheck": True, "manifestHashes": True, "zipCrcAndHashes": True,
             "windowsRuntimeTested": False, "sdlVersion": SDL_VERSION, "sdlDownloadSha256": SDL_SHA256}
    (BUILD / "package-checks.json").write_text(json.dumps(audit, ensure_ascii=False, indent=2) + "\n")
    print(json.dumps(audit, ensure_ascii=False, indent=2))
    print(output)


if __name__ == "__main__":
    main()
