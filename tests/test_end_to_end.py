"""End-to-end checks for the ISA v1 assembler and CPU executable."""

import re
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSEMBLER = ROOT / "bin" / "assembler"
EMULATOR = ROOT / "bin" / "emulator"


class EndToEndTests(unittest.TestCase):
    def assemble(self, text, success=True):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "input.asm"
            output = Path(directory) / "output.bin"
            source.write_text(text, encoding="utf-8")
            result = subprocess.run(
                [str(ASSEMBLER), str(source), str(output)],
                capture_output=True,
                text=True,
                check=False,
            )
            if success:
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertTrue(output.exists())
                return output.read_bytes()
            self.assertNotEqual(result.returncode, 0, text)
            self.assertFalse(output.exists(), "invalid ASM must not produce bytecode")
            self.assertIn(":", result.stderr)
            return result.stderr

    def run_cpu(self, bytecode, success=True):
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / "input.bin"
            binary.write_bytes(bytecode)
            result = subprocess.run(
                [str(EMULATOR), str(binary)],
                capture_output=True,
                text=True,
                check=False,
            )
        if success:
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("HLT after", result.stdout)
            return {int(reg): int(value) for reg, value in
                    re.findall(r"R([0-7]): (\d+)", result.stdout)}
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertTrue("Execution error" in result.stderr or "Invalid program" in result.stderr, result.stderr)
        return result.stderr

    def test_program1_and_jump_over_instruction(self):
        source = (ROOT / "example_programs" / "program1.asm").read_text(encoding="utf-8")
        bytecode = self.assemble(source)
        self.assertEqual(len(bytecode), 21)
        self.assertEqual(bytecode[12:15], bytes([0x04, 18, 0]))
        self.assertEqual(self.run_cpu(bytecode), dict(enumerate([30, 246, 0, 0, 0, 0, 0, 0])))

    def test_xor_example(self):
        source = (ROOT / "example_programs" / "xor_program.asm").read_text(encoding="utf-8")
        self.assertEqual(self.run_cpu(self.assemble(source))[0], 6)

    def test_fixed_encoding_for_every_opcode(self):
        bytecode = self.assemble("""\
            mov R0, 10
            mov R1, 20
            add R0, R1
            sub R1, R0
            xor R0, R1
            jmp finish
            mov R2, 99
            finish: hlt
        """)
        self.assertEqual(bytecode, bytes([
            0x01, 0, 10, 0x01, 1, 20, 0x02, 0, 1, 0x03, 1, 0,
            0x05, 0, 1, 0x04, 21, 0, 0x01, 2, 99, 0xFF, 0, 0,
        ]))
        self.assertEqual(self.run_cpu(bytecode)[2], 0)

    def test_jump_supports_addresses_above_255(self):
        bytecode = self.assemble("jmp far\n" + "hlt\n" * 90 +
                                 "far: mov R0, 42\nhlt\n")
        self.assertEqual(bytecode[:3], bytes([0x04, 0x11, 0x01]))
        self.assertEqual(self.run_cpu(bytecode)[0], 42)

    def test_negative_literal_and_unsigned_wraparound(self):
        registers = self.run_cpu(self.assemble("""\
            MOV R0, -1
            mov R1, 3
            add R0, R1
            sub R1, R0
            xor R0, R1
            hlt
        """))
        self.assertEqual(registers[0], 3)
        self.assertEqual(registers[1], 1)

    def test_comments_hex_literals_and_numeric_jump(self):
        registers = self.run_cpu(self.assemble("""\
            # start
            jmp 6
            mov R0, 99
            skip: mov R0, 0xFE ; result is 254
            hlt
        """))
        self.assertEqual(registers[0], 254)

    def test_rejects_invalid_assembly_without_partial_output(self):
        invalid = [
            "xyz R0, 1\nhlt\n",
            "mov R8, 1\nhlt\n",
            "mov R00, 1\nhlt\n",
            "mov R0, R1\nhlt\n",
            "mov R0, 256\nhlt\n",
            "mov R0, -129\nhlt\n",
            "mov R0, 3oops\nhlt\n",
            "add R0, R9\nhlt\n",
            "sub R0 R1\nhlt\n",
            "xor R0, R1, R2\nhlt\n",
            "jmp\nhlt\n",
            "jmp nowhere\nhlt\n",
            "jmp -1\nhlt\n",
            "jmp 1\nhlt\n",
            "jmp 1023\nhlt\n",
            "jmp 99\nhlt\n",
            "hlt garbage\n",
            "twice: hlt\ntwice: hlt\n",
            "bad-label: hlt\n",
            "onlylabel:\n",
            "; only comments\n",
            "hlt\n" * 342,
            " " * 511 + "hlt\n",
        ]
        for source in invalid:
            with self.subTest(source=source[:80]):
                self.assemble(source, success=False)

    def test_rejects_invalid_or_truncated_bytecode(self):
        invalid = [
            b"",
            bytes([0x01, 0]),
            bytes([0xFE, 0, 0]),
            bytes([0x01, 8, 5]),
            bytes([0x02, 0, 8]),
            bytes([0x04, 1, 0, 0xFF, 0, 0]),
            bytes([0x04, 6, 0, 0xFF, 0, 0]),
            bytes([0x04, 0, 4]),
            bytes([0xFF, 0, 1]),
            bytes([0x01, 0, 3]),  # missing HLT, even with zero-filled memory
            bytes([0xFF, 0, 0]) + b"\x00" * 1022,
        ]
        for bytecode in invalid:
            with self.subTest(bytecode=bytecode[:15]):
                self.run_cpu(bytecode, success=False)

    def test_non_halting_program_has_step_limit(self):
        error = self.run_cpu(bytes([0x04, 0, 0]), success=False)
        self.assertIn("step limit", error)


if __name__ == "__main__":
    unittest.main()
