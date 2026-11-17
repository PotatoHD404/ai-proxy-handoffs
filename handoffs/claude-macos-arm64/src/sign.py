from pathlib import Path
import subprocess
import sys

app = Path(sys.argv[1]).resolve()
mach_magic = {
    bytes.fromhex(magic)
    for magic in ("cffaedfe", "feedfacf", "cefaedfe", "feedface", "cafebabe", "cafebabf")
}

def sign(path):
    subprocess.run(
        ["/usr/bin/codesign", "--force", "--sign", "-", "--timestamp=none", str(path)],
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
    )

def is_mach(path):
    with path.open("rb") as stream:
        return stream.read(4) in mach_magic

for path in app.rglob("*"):
    if not path.is_symlink() and path.is_file() and is_mach(path) and "Contents/_CodeSignature" not in str(path):
        try:
            sign(path)
        except subprocess.CalledProcessError as error:
            print(error.stderr.decode(), file=sys.stderr)
            raise

bundles = [
    path for path in app.rglob("*")
    if not path.is_symlink() and path.is_dir() and path.suffix in (".app", ".xpc", ".bundle", ".framework")
]
for path in sorted(bundles, key=lambda item: len(item.parts), reverse=True):
    sign(path)
sign(app)
