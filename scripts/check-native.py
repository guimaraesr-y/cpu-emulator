"""Exercise real hidden GL windows. Run under xvfb-run on headless Linux."""
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    executable = Path(sys.argv[1]).resolve()
    cases = [
        ("immediate_halt", bytes([255, 0, 0]), 0, "PC=3 R0=0 R1=0 steps=1"),
        ("two_instructions", bytes([1, 0, 42, 255, 0, 0]), 0, "PC=6 R0=42 R1=0 steps=2"),
        ("demo", None, 0, "PC=21 R0=30 R1=246 steps=6"),
        ("xor", bytes([1, 0, 3, 1, 1, 5, 5, 0, 1, 255, 0, 0]), 0, "R0=6"),
        ("asm_labels", b"; source map\r\njmp end\r\nmov R0, 99\r\nend: mov R0, 42\r\nhlt\r\n", 0, "PC=12 R0=42"),
        ("asm_error", b"mov R9, 1\nhlt\n", 1, "empty, oversized or incomplete bytecode"),
        ("register_fault", bytes([2, 0, 8]), 1, "invalid register index"),
        ("missing_halt", bytes([1, 0, 7]), 1, "PC outside loaded instructions"),
        ("truncated", bytes([1, 0]), 1, "empty, oversized or incomplete bytecode"),
        ("oversized", bytes(1025), 1, "empty, oversized or incomplete bytecode"),
    ]
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        for name, bytecode, expected_code, expected_text in cases:
            capture = root / f"{name}.png"
            command = [str(executable), "--smoke-test", str(capture)]
            if bytecode is not None:
                program = root / f"{name}.{'asm' if name.startswith('asm_') else 'bin'}"
                program.write_bytes(bytecode)
                command.append(str(program))
            result = subprocess.run(command, capture_output=True, text=True, timeout=30)
            assert result.returncode == expected_code, (name, result.stdout, result.stderr)
            assert expected_text in result.stdout, (name, result.stdout, result.stderr)
            assert capture.is_file(), (name, "requested capture missing")
            assert Path(f"{capture}.final.png").is_file(), name
            if name == "demo":
                assert capture.read_bytes() != Path(f"{capture}.final.png").read_bytes(), "demo needs a distinct execution-phase capture"
            if name in ("immediate_halt", "two_instructions"):
                assert capture.read_bytes() == Path(f"{capture}.final.png").read_bytes(), "short programs capture their final state"
            print(f"PASS {name}: {result.stdout.strip()}")
        # A successful CPU halt must not hide a failed image export.
        missing_capture = root / "missing_directory" / "capture.png"
        result = subprocess.run(
            [str(executable), "--smoke-test", str(missing_capture), str(root / "immediate_halt.bin")],
            capture_output=True, text=True, timeout=30)
        assert result.returncode == 1 and "status=halted" in result.stdout, (result.stdout, result.stderr)
        assert not missing_capture.exists()
        print("PASS image_export_failure: halted CPU still reports failed capture")
    return 0


if __name__ == "__main__":
    sys.exit(main())
