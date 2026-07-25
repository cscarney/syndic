#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later
"""Make a Qt .app bundle self-contained and reproducible

This is a rather hackish way to work around the limitations of macdeployqt in
bundling KF6 libraries, QML modules and plugins. We scan the binaries already
in the bundle for external dependencies using otool, copy them into the bundle
and relativize their paths with install_name_tool. 

We additionally prune libs that nothing depends on to make sure that we get a
consistent retuls regardless of what libs are present in the build environment

This script is designed to be project-independent; avoid hard-coding stuff here
"""

import argparse
import os
import shutil
import stat
import subprocess
import sys
from pathlib import Path

SYSTEM_PREFIXES = ("/usr/lib/", "/System/Library/", "/Library/Apple/")
RELATIVE_PREFIXES = ("@executable_path", "@loader_path", "@rpath")
FRAMEWORKS_REL = "Contents/Frameworks"


def run(cmd):
    return subprocess.run(cmd, check=True, capture_output=True, text=True).stdout


def is_macho(path: Path) -> bool:
    if path.is_symlink() or not path.is_file():
        return False
    try:
        with path.open("rb") as fh:
            magic = fh.read(4)
    except OSError:
        return False
    # Little-endian 64/32-bit Mach-O, plus universal ("fat") binaries.
    return magic in (
        b"\xcf\xfa\xed\xfe",
        b"\xce\xfa\xed\xfe",
        b"\xca\xfe\xba\xbe",
        b"\xbe\xba\xfe\xca",
    )


def macho_files(root: Path):
    for dirpath, _dirnames, filenames in os.walk(root):
        for name in filenames:
            path = Path(dirpath) / name
            if is_macho(path):
                yield path


def dependencies(path: Path):
    out = run(["otool", "-L", str(path)])
    lines = out.splitlines()[1:]
    deps = []
    for line in lines:
        line = line.strip()
        if not line or "(compatibility version" not in line:
            continue
        deps.append(line.rsplit(" (compatibility version", 1)[0].strip())
    # drop the library's own ID
    own_id = install_id(path)
    if deps and own_id and deps[0] == own_id:
        deps = deps[1:]
    return deps


def install_id(path: Path):
    try:
        out = run(["otool", "-D", str(path)]).splitlines()
    except subprocess.CalledProcessError:
        return None
    return out[1].strip() if len(out) > 1 else None


def rpaths(path: Path):
    out = run(["otool", "-l", str(path)])
    found, lines = [], out.splitlines()
    for i, line in enumerate(lines):
        if line.strip() == "cmd LC_RPATH":
            for follow in lines[i : i + 4]:
                if "path " in follow:
                    found.append(follow.split("path ", 1)[1].split(" (offset")[0])
                    break
    return found


def make_writable(path: Path):
    path.chmod(path.stat().st_mode | stat.S_IWUSR)


def framework_parts(dep: str):
    if ".framework/" not in dep:
        return None
    head, tail = dep.split(".framework/", 1)
    return Path(head + ".framework"), tail


class Relocator:
    def __init__(self, bundle: Path, search_paths, verbose=False):
        self.bundle = bundle
        self.frameworks = bundle / FRAMEWORKS_REL
        self.search_paths = [Path(p) for p in search_paths]
        self.verbose = verbose
        self.frameworks.mkdir(parents=True, exist_ok=True)
        self.copied = []

    def log(self, msg):
        if self.verbose:
            print(f"  {msg}")

    def rel_to_executable(self, target: Path) -> str:
        """@executable_path-relative reference to a file inside the bundle."""
        rel = os.path.relpath(target, self.bundle / "Contents" / "MacOS")
        return f"@executable_path/{rel}"

    def expand(self, text: str, origin: Path) -> str:
        return text.replace("@loader_path", str(origin.parent)).replace(
            "@executable_path", str(self.bundle / "Contents" / "MacOS")
        )

    def resolve_relative(self, dep: str, origin: Path):
        """Resolve an @rpath/@loader_path/@executable_path dependency to a real file."""
        if dep.startswith("@rpath/"):
            suffix = dep[len("@rpath/") :]
            search = [Path(self.expand(rp, origin)) for rp in rpaths(origin)]
            search.append(self.frameworks)
            search += self.search_paths
            for directory in search:
                candidate = directory / suffix
                if candidate.exists():
                    return candidate
            return None

        candidate = Path(self.expand(dep, origin))
        return candidate if candidate.exists() else None

    def bundle_destination(self, dep: str, source: Path):
        """Where to put a dependency in the bundle"""
        parts = framework_parts(dep)
        if parts:
            fw_dir, tail = parts
            return self.frameworks / fw_dir.name / tail
        return self.frameworks / source.name

    def ingest(self, source: Path, dest: Path):
        """Copy a library into the bundle"""
        if dest.exists():
            return dest
        dest.parent.mkdir(parents=True, exist_ok=True)
        if ".framework/" in str(dest):
            # Copy the whole framework then drop headers
            fw_name = next(
                p for p in dest.parts if p.endswith(".framework")
            )
            src_fw = Path(str(source).split(f"/{fw_name}/")[0]) / fw_name
            dst_fw = self.frameworks / fw_name
            if not dst_fw.exists():
                shutil.copytree(src_fw, dst_fw, symlinks=True)
                for junk in ("Headers", "Versions/A/Headers", "Versions/Current/Headers"):
                    target = dst_fw / junk
                    if target.is_symlink():
                        target.unlink()
                    elif target.is_dir():
                        shutil.rmtree(target)
        else:
            shutil.copy2(source, dest)
        make_writable(dest)
        self.copied.append(dest)
        self.log(f"copied {source} -> {dest.relative_to(self.bundle)}")
        run(["install_name_tool", "-id", self.rel_to_executable(dest), str(dest)])
        return dest

    def process(self, path: Path) -> bool:
        """Rewrite one binary's load commands"""
        changed = False
        make_writable(path)

        # Normalize the install id of anything already living in the bundle.
        if path.suffix == ".dylib" or ".framework/" in str(path):
            current = install_id(path)
            wanted = self.rel_to_executable(path)
            if current and current != wanted:
                run(["install_name_tool", "-id", wanted, str(path)])
                changed = True

        for dep in dependencies(path):
            if dep.startswith(SYSTEM_PREFIXES) or dep in ("/usr/lib/libSystem.B.dylib",):
                continue

            if dep.startswith(RELATIVE_PREFIXES):
                source = self.resolve_relative(dep, path)
                if source is not None and self.bundle in source.parents:
                    # Already points at something inside the bundle
                    if dep.startswith("@rpath/"):
                        # @rpath depends on LC_RPATH that does not survive deployment
                        run(["install_name_tool", "-change", dep,
                             self.rel_to_executable(source), str(path)])
                        changed = True
                    continue
                if source is None:
                    print(f"warning: cannot resolve {dep} (from {path})", file=sys.stderr)
                    continue
                # if we get here it resolves to a file outside the bundle
                target = self.ingest(source, self.bundle_destination(dep, source))
                run(["install_name_tool", "-change", dep,
                     self.rel_to_executable(target), str(path)])
                changed = True
                continue

            source = Path(dep)
            if not source.exists():
                # Fall back to matching by basename in the search paths
                found = None
                for sp in self.search_paths:
                    candidate = sp / source.name
                    if candidate.exists():
                        found = candidate
                        break
                if found is None:
                    print(f"warning: cannot resolve {dep} (from {path})", file=sys.stderr)
                    continue
                source = found

            dest = self.bundle_destination(dep, source)
            if dest.exists() and self.bundle in dest.parents:
                target = dest
            else:
                target = self.ingest(source, dest)
            run(["install_name_tool", "-change", dep, self.rel_to_executable(target), str(path)])
            changed = True

        return changed

    def run_to_fixpoint(self, max_passes=12):
        for pass_no in range(1, max_passes + 1):
            self.copied = []
            files = list(macho_files(self.bundle))
            print(f"pass {pass_no}: {len(files)} Mach-O files")
            for path in files:
                self.process(path)
            if not self.copied:
                return
            print(f"  pulled in {len(self.copied)} new libraries; re-scanning")
        raise SystemExit("error: dependency resolution did not converge")

    def reachable_libraries(self):
        roots = []
        for sub in ("Contents/MacOS", "Contents/PlugIns", "Contents/Resources/qml"):
            root_dir = self.bundle / sub
            if root_dir.is_dir():
                roots.extend(macho_files(root_dir))

        seen, queue = set(), list(roots)
        while queue:
            current = queue.pop()
            real = current.resolve()
            if real in seen:
                continue
            seen.add(real)
            for dep in dependencies(current):
                if dep.startswith(SYSTEM_PREFIXES) or not dep.startswith(RELATIVE_PREFIXES):
                    continue
                resolved = self.resolve_relative(dep, current)
                if resolved is not None and resolved.resolve() not in seen:
                    queue.append(resolved)
        return seen

    def collect_garbage(self):
        """Delete libraries that nothing in the bundle loads"""
        reachable = self.reachable_libraries()
        freed = 0

        # Frameworks are keyed on the framework's binary
        for fw in sorted(self.frameworks.glob("*.framework")):
            binaries = [p for p in macho_files(fw)]
            if binaries and not any(b.resolve() in reachable for b in binaries):
                freed += sum(f.stat().st_size for f in fw.rglob("*") if f.is_file())
                shutil.rmtree(fw)
                self.log(f"pruned {fw.name}")

        # Plain dylibs, plus the symlinks that pointed at them.
        for lib in sorted(self.frameworks.glob("*.dylib")):
            if lib.is_symlink():
                continue
            if lib.resolve() not in reachable:
                freed += lib.stat().st_size
                lib.unlink()
                self.log(f"pruned {lib.name}")
        for link in sorted(self.frameworks.glob("*.dylib")):
            if link.is_symlink() and not link.resolve().exists():
                link.unlink()

        print(f"garbage collection freed {freed / 1024 / 1024:.1f} MB")

    def verify(self):
        """Fail if anything in the bundle still points outside of it"""
        problems = []
        for path in macho_files(self.bundle):
            for dep in dependencies(path):
                if dep.startswith(SYSTEM_PREFIXES):
                    continue
                if dep.startswith(RELATIVE_PREFIXES):
                    resolved = self.resolve_relative(dep, path)
                    if resolved is None:
                        problems.append(f"{path}: dangling {dep}")
                    elif self.bundle not in resolved.resolve().parents:
                        problems.append(f"{path}: {dep} escapes the bundle ({resolved})")
                    continue
                problems.append(f"{path}: external {dep}")
        if problems:
            print("bundle is NOT self-contained:", file=sys.stderr)
            for problem in problems:
                print(f"  {problem}", file=sys.stderr)
            raise SystemExit(1)
        print("verified: all references are bundle-relative or system libraries")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="path to the .app bundle")
    parser.add_argument(
        "--search",
        action="append",
        default=[],
        help="directory to search for libraries with broken install names",
    )
    parser.add_argument("--verify-only", action="store_true")
    parser.add_argument(
        "--gc",
        action="store_true",
        help="delete libraries that nothing in the bundle loads",
    )
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    bundle = args.bundle.resolve()
    if not (bundle / "Contents").is_dir():
        raise SystemExit(f"error: {bundle} is not an app bundle")

    relocator = Relocator(bundle, args.search, verbose=args.verbose)
    if not args.verify_only:
        relocator.run_to_fixpoint()
        if args.gc:
            relocator.collect_garbage()
    relocator.verify()


if __name__ == "__main__":
    main()
