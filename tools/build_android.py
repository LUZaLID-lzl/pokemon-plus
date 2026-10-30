#!/usr/bin/env python3
"""Build and sign an offline Android 14+ APK using the installed SDK/NDK/JDK."""
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import time
import zipfile

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build/android"
DEPS = ROOT / "build/android-deps"
SDL_VERSION = "2.32.10"
SDL_SHA256 = "5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165"
SDK = Path(os.environ.get("ANDROID_SDK_ROOT", Path.home()/"Library/Android/sdk"))
JDK = Path(os.environ.get("JAVA_HOME", "/Applications/Android Studio.app/Contents/jbr/Contents/Home"))
ENV = dict(os.environ, JAVA_HOME=str(JDK))
TOOLS = SDK / "build-tools/36.0.0"
PLATFORM = SDK / "platforms/android-36/android.jar"
NDK = Path(os.environ["ANDROID_NDK_HOME"]) if "ANDROID_NDK_HOME" in os.environ else next(
    (p for p in sorted((SDK/"ndk").glob("*"), key=lambda p: tuple(map(int,p.name.split("."))), reverse=True)
     if (p/"build/cmake/android.toolchain.cmake").is_file()), SDK/"ndk/missing")
ABIS = ("arm64-v8a", "x86_64")


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(args, log=None):
    if log is None:
        try:
            return subprocess.check_output([str(a) for a in args], env=ENV, text=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as failure:
            raise RuntimeError(f"{args[0]} failed:\n{failure.output}") from failure
    with log.open("w") as stream:
        result = subprocess.run([str(a) for a in args], env=ENV, stdout=stream, stderr=subprocess.STDOUT)
    if result.returncode:
        raise RuntimeError(f"Command failed; see {log}\n{log.read_text()[-6000:]}")


def main():
    for path in (TOOLS/"aapt2", TOOLS/"d8", TOOLS/"zipalign", TOOLS/"apksigner", PLATFORM, NDK/"build/cmake/android.toolchain.cmake", JDK/"bin/javac"):
        if not path.exists():
            raise SystemExit(f"Required Android build component is missing: {path}")
    BUILD.mkdir(parents=True, exist_ok=True)
    DEPS.mkdir(parents=True, exist_ok=True)
    source_archive = DEPS / f"SDL2-{SDL_VERSION}.tar.gz"
    if not source_archive.exists() or sha(source_archive) != SDL_SHA256:
        run(["curl", "-fLsS", "--connect-timeout", "15", "--max-time", "300", "--retry", "2",
             f"https://www.libsdl.org/release/SDL2-{SDL_VERSION}.tar.gz", "-o", source_archive])
    if sha(source_archive) != SDL_SHA256:
        raise ValueError("SDL2 source download hash mismatch")
    source = DEPS / f"SDL2-{SDL_VERSION}"
    if not (source/"CMakeLists.txt").exists():
        run(["tar", "-xzf", source_archive, "-C", DEPS])
    version = re.search(r'set\(PROJECT_VER "([^"]+)"\)', (ROOT/"CMakeLists.txt").read_text())[1]
    parts = list(map(int, version.split(".")))
    version_code = (parts[0]*10000+parts[1]*100+parts[2])*100+1
    native_api=min(34,json.loads((NDK/"meta/platforms.json").read_text())["max"])
    for abi in ABIS:
        print(f"Building Android {abi}…", flush=True)
        native = BUILD/abi
        native.mkdir(exist_ok=True)
        run(["cmake", "-S", ROOT/"android", "-B", native, "-G", "Ninja",
             f"-DCMAKE_TOOLCHAIN_FILE={NDK/'build/cmake/android.toolchain.cmake'}",
             f"-DANDROID_ABI={abi}", f"-DANDROID_PLATFORM=android-{native_api}", "-DANDROID_STL=c++_static",
             "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_C_FLAGS_RELEASE=-O2",
             f"-DPP_SDL_SOURCE_DIR={source}", f"-DPP_APP_VERSION={version}"], native/"configure.log")
        run(["cmake", "--build", native, "--target", "main", "-j8"], native/"build.log")
    android_manifest = (ROOT/"android/AndroidManifest.xml.in").read_text().replace("@VERSION_CODE@", str(version_code)).replace("@VERSION@",version)
    (BUILD/"AndroidManifest.xml").write_text(android_manifest)
    run([TOOLS/"aapt2", "compile", "--dir", ROOT/"android/res", "-o", BUILD/"compiled-resources.zip"])
    run([TOOLS/"aapt2", "link", "-I", PLATFORM, "--manifest", BUILD/"AndroidManifest.xml",
         "-o", BUILD/"base.apk", BUILD/"compiled-resources.zip"])
    classes = BUILD/"classes"
    classes.mkdir(exist_ok=True)
    sources = sorted((source/"android-project/app/src/main/java").rglob("*.java")) + sorted((ROOT/"android/src").rglob("*.java"))
    run([JDK/"bin/javac", "-source", "8", "-target", "8", "-Xlint:-options", "-bootclasspath", str(PLATFORM)+os.pathsep+str(TOOLS/"core-lambda-stubs.jar"),
         "-d", classes, *sources], BUILD/"javac.log")
    dex = BUILD/"dex"
    dex.mkdir(exist_ok=True)
    run([TOOLS/"d8", "--min-api", "34", "--lib", PLATFORM, "--output", dex, *sorted(classes.rglob("*.class"))], BUILD/"dex.log")
    manifest = json.loads((ROOT/"sdcard/pokemon-plus/manifest.json").read_text())
    assets = [row for row in manifest["files"] if row["path"].endswith(".bin")]
    resource_zip = BUILD/"native-resources.zip"
    with zipfile.ZipFile(resource_zip, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as zipped:
        for row in assets:
            path=ROOT/"sdcard/pokemon-plus"/row["path"]
            if sha(path)!=row["sha256"] or path.stat().st_size!=row["bytes"]:
                raise ValueError(f"Resource hash mismatch: {path}")
            zipped.write(path,row["path"])
    unsigned = BUILD/"unsigned.apk"
    shutil.copy2(BUILD/"base.apk",unsigned)
    elf_details = {}
    llvm = next(p/"bin" for p in (NDK/"toolchains/llvm/prebuilt").iterdir() if (p/"bin/llvm-strip").is_file())
    with zipfile.ZipFile(unsigned,"a",zipfile.ZIP_DEFLATED) as apk:
        for file in sorted(dex.glob("*.dex")):
            apk.write(file,file.name)
        apk.write(resource_zip,"assets/resources.zip",compress_type=zipfile.ZIP_STORED)
        apk.writestr("assets/resources.sha256",sha(resource_zip))
        apk.write(source/"LICENSE.txt","assets/licenses/SDL2.txt")
        apk.write(ROOT/"managed_components/lvgl__lvgl/LICENCE.txt","assets/licenses/LVGL.txt")
        for abi in ABIS:
            for lib in [BUILD/abi/"libmain.so", BUILD/abi/"sdl/libSDL2.so"]:
                run([llvm/"llvm-strip", "--strip-unneeded", lib])
                detail=run([llvm/"llvm-readelf", "-lW", lib])
                loads=[line for line in detail.splitlines() if line.strip().startswith("LOAD ")]
                if not loads or any(int(line.split()[-1],16)<16384 for line in loads):
                    raise ValueError(f"ELF is not aligned for 16KB pages: {lib}")
                elf_details[f"{abi}/{lib.name}"]={"sha256":sha(lib), "pageAlignment":16384}
                apk.write(lib,f"lib/{abi}/{lib.name}",compress_type=zipfile.ZIP_STORED)
    aligned = BUILD/"aligned.apk"
    run([TOOLS/"zipalign", "-P", "16", "-f", "4", unsigned, aligned])
    keys = ROOT/"android/.signing"
    keys.mkdir(exist_ok=True)
    key = keys/"pokemon-plus.jks"
    old_key=ROOT/"build/android-signing/pokemon-plus.jks"
    if not key.exists() and old_key.exists():
        shutil.copy2(old_key,key)
        key.chmod(0o600)
    if not key.exists():
        run([JDK/"bin/keytool", "-genkeypair", "-keystore", key, "-storepass", "android", "-keypass", "android",
             "-alias", "pokemonplus", "-keyalg", "RSA", "-keysize", "2048", "-validity", "10000",
             "-dname", "CN=Pokemon Plus Simulator"], BUILD/"key-generation.log")
        key.chmod(0o600)
    output=ROOT/"dist/pokemon-plus-android14.apk"
    run([TOOLS/"apksigner", "sign", "--ks", key, "--ks-key-alias", "pokemonplus", "--ks-pass", "pass:android",
         "--key-pass", "pass:android", "--min-sdk-version", "34", "--v1-signing-enabled", "false", "--out", output, aligned])
    signature=run([TOOLS/"apksigner", "verify", "--verbose", "--print-certs", output])
    (BUILD/"signature-checks.txt").write_text(signature)
    run([TOOLS/"zipalign", "-c", "-P", "16", "4", output])
    badging=run([TOOLS/"aapt2", "dump", "badging", output])
    (BUILD/"apk-badging.txt").write_text(badging)
    with zipfile.ZipFile(output) as apk:
        if apk.testzip():raise ValueError("APK ZIP CRC failure")
        if hashlib.sha256(apk.read("assets/resources.zip")).hexdigest()!=sha(resource_zip):
            raise ValueError("APK resource archive hash mismatch")
    audit={"version":version,"versionCode":version_code,"minSdk":34,"targetSdk":36,"abis":ABIS,
           "resources":len(assets),"resourceZipSha256":sha(resource_zip),"apkSha256":sha(output),
           "apkBytes":output.stat().st_size,"signatureVerified":True,"zipAlignment16KB":True,"libraries":elf_details}
    (BUILD/"package-checks.json").write_text(json.dumps(audit,indent=2)+"\n")
    print(json.dumps(audit,indent=2))
    print(output)


if __name__=="__main__":
    main()
