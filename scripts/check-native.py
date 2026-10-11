"""Exercise real hidden GL windows. Run under xvfb-run on headless Linux."""
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    executable = Path(sys.argv[1]).resolve()
    cases = [
        ("demo", None, 0, "PC=21 R0=30 R1=246 steps=6"),
        ("xor", bytes([1, 0, 3, 1, 1, 5, 5, 0, 1, 255, 0, 0]), 0, "R0=6"),
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
                program = root / f"{name}.bin"
                program.write_bytes(bytecode)
                command.append(str(program))
            result = subprocess.run(command, capture_output=True, text=True, timeout=30)
            assert result.returncode == expected_code, (name, result.stdout, result.stderr)
            assert expected_text in result.stdout, (name, result.stdout, result.stderr)
            assert Path(f"{capture}.final.png").is_file(), name
            print(f"PASS {name}: {result.stdout.strip()}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
