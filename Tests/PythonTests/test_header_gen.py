import importlib.util
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
GENERATOR = ROOT / "Scripts" / "header_gen.py"
SPEC = importlib.util.spec_from_file_location("header_gen", GENERATOR)
HEADER_GEN = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(HEADER_GEN)


def test_header_generator_creates_stable_aggregate(tmp_path):
    include_dir = tmp_path / "baba-is-auto"
    nested = include_dir / "Nested"
    nested.mkdir(parents=True)
    (nested / "Example.hpp").write_text("// example\n", encoding="utf-8")

    HEADER_GEN.generate(include_dir)

    header = include_dir / "baba-is-auto.hpp"
    generated = header.read_text(encoding="utf-8")
    modified = header.stat().st_mtime_ns
    time.sleep(0.01)
    HEADER_GEN.generate(include_dir)

    assert header.read_text(encoding="utf-8") == generated
    assert header.stat().st_mtime_ns == modified
    assert "#include <baba-is-auto/Nested/Example.hpp>" in generated
    assert "#include <baba-is-auto/baba-is-auto.hpp>" not in generated
