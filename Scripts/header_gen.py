#!/usr/bin/env python

import filecmp
import shutil
from pathlib import Path


script_dir = Path(__file__).resolve().parent


def generate(include_dir):
    include_dir = Path(include_dir)
    header = include_dir / "baba-is-auto.hpp"
    file_names = sorted(
        path.relative_to(include_dir).as_posix()
        for path in include_dir.rglob("*.hpp")
        if path != header
    )

    header_tmp = header.with_name(header.name + ".tmp")
    with header_tmp.open("w") as header_file:
        header_file.write("""// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.\n
""")
        header_file.write("#ifndef BABA_IS_AUTO_HPP\n")
        header_file.write("#define BABA_IS_AUTO_HPP\n\n")
        for filename in file_names:
            line = "#include <baba-is-auto/%s>\n" % filename
            header_file.write(line)
        header_file.write("\n#endif  // BABA_IS_AUTO_HPP\n")

    if not header.exists() or not filecmp.cmp(header, header_tmp, shallow=False):
        shutil.move(header_tmp, header)
    else:
        header_tmp.unlink()


def main():
    generate(script_dir.parent / "Includes" / "baba-is-auto")


if __name__ == "__main__":
    main()
