#!/usr/bin/env python3
"""Strict black-box tester for the Codexion project.

Run from the project root:
    python3 codexion_strict_tester.py
    python3 codexion_strict_tester.py --program ./codexion --repeat 20

This validates all properties observable from the official stdout format.
FIFO/EDF request-order arbitration cannot be proven from official stdout because
request/enqueue events are not part of that format; inspect/instrument those
events separately before submission.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path


LINE_RE = re.compile(
    r"^(?P<time>\d+) (?P<coder>\d+) "
    r"(?P<event>has taken a dongle|is compiling|is debugging|"
    r"is refactoring|burned out)$"
)

GREEN = "\033[0;32m"
RED = "\033[0;31m"
YELLOW = "\033[1;33m"
BLUE = "\033[0;34m"
RESET = "\033[0m"


@dataclass(frozen=True)
class Config:
    coders: int
    burnout: int
    compile_ms: int
    debug_ms: int
    refactor_ms: int
    required: int
    cooldown: int
    scheduler: str

    def argv(self) -> list[str]:
        return [
            str(self.coders),
            str(self.burnout),
            str(self.compile_ms),
            str(self.debug_ms),
            str(self.refactor_ms),
            str(self.required),
            str(self.cooldown),
            self.scheduler,
        ]


@dataclass(frozen=True)
class Event:
    time: int
    coder: int
    text: str
    line_number: int


@dataclass(frozen=True)
class Case:
    name: str
    config: Config
    expected: str
    repeats: int


class Failure(Exception):
    pass


def color(label: str, value: str) -> str:
    return f"{value}{label}{RESET}"


def run_process(command: list[str], timeout: float) -> subprocess.CompletedProcess[str]:
    try:
        return subprocess.run(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        raise Failure(f"timeout after {timeout:.0f}s") from exc


def parse_events(output: str, config: Config) -> list[Event]:
    if "\x00" in output:
        raise Failure("stdout contains a NUL byte")
    events: list[Event] = []
    previous_time = -1
    for line_number, raw in enumerate(output.splitlines(), 1):
        match = LINE_RE.fullmatch(raw)
        if not match:
            raise Failure(f"invalid or merged output at line {line_number}: {raw!r}")
        event = Event(
            int(match.group("time")),
            int(match.group("coder")),
            match.group("event"),
            line_number,
        )
        if not 1 <= event.coder <= config.coders:
            raise Failure(f"invalid coder id at line {line_number}: {event.coder}")
        if event.time < previous_time:
            raise Failure(f"timestamp moved backwards at line {line_number}")
        previous_time = event.time
        events.append(event)
    if not events:
        raise Failure("program produced no stdout")
    return events


def coder_dongles(coder: int, total: int) -> tuple[int, int]:
    right = coder - 1
    left = (coder - 2) % total
    return left, right


def validate_two_dongles(events: list[Event], config: Config) -> None:
    taken = [0] * (config.coders + 1)
    for event in events:
        if event.text == "has taken a dongle":
            taken[event.coder] += 1
            if taken[event.coder] > 2:
                raise Failure(
                    f"coder {event.coder} took more than two dongles "
                    f"before compiling (line {event.line_number})"
                )
        elif event.text == "is compiling":
            if taken[event.coder] != 2:
                raise Failure(
                    f"coder {event.coder} compiled after {taken[event.coder]} "
                    f"take messages (line {event.line_number})"
                )
            taken[event.coder] = 0


def validate_dongle_timeline(events: list[Event], config: Config) -> None:
    uses: list[list[tuple[int, int]]] = [[] for _ in range(config.coders)]
    for event in events:
        if event.text != "is compiling":
            continue
        for dongle in coder_dongles(event.coder, config.coders):
            uses[dongle].append((event.time, event.coder))
    tolerance_ms = 3
    required_gap = config.compile_ms + config.cooldown
    for dongle, timeline in enumerate(uses):
        timeline.sort()
        for previous, current in zip(timeline, timeline[1:]):
            previous_start, previous_coder = previous
            current_start, current_coder = current
            observed_gap = current_start - previous_start
            if observed_gap + tolerance_ms < required_gap:
                raise Failure(
                    f"dongle {dongle + 1} reused too early: coder "
                    f"{previous_coder} compiled at {previous_start}ms, coder "
                    f"{current_coder} at {current_start}ms; gap={observed_gap}ms, "
                    f"required about {required_gap}ms"
                )


def validate_state_order(events: list[Event], config: Config) -> None:
    phase = ["initial"] * (config.coders + 1)
    compile_count = [0] * (config.coders + 1)
    for event in events:
        coder = event.coder
        if event.text == "is compiling":
            if phase[coder] not in ("initial", "refactoring"):
                raise Failure(
                    f"coder {coder} compiled after phase {phase[coder]} "
                    f"at line {event.line_number}"
                )
            phase[coder] = "compiling"
            compile_count[coder] += 1
            if compile_count[coder] > config.required:
                raise Failure(f"coder {coder} compiled more than required")
        elif event.text == "is debugging":
            if phase[coder] != "compiling":
                raise Failure(
                    f"coder {coder} debugged after phase {phase[coder]} "
                    f"at line {event.line_number}"
                )
            phase[coder] = "debugging"
        elif event.text == "is refactoring":
            if phase[coder] != "debugging":
                raise Failure(
                    f"coder {coder} refactored after phase {phase[coder]} "
                    f"at line {event.line_number}"
                )
            phase[coder] = "refactoring"


def validate_burnout(events: list[Event], config: Config, expected: str) -> None:
    burnouts = [event for event in events if event.text == "burned out"]
    if len(burnouts) > 1:
        raise Failure("more than one burnout message")
    if burnouts and burnouts[0] != events[-1]:
        raise Failure("burnout message is not the final output line")
    if expected == "success" and burnouts:
        raise Failure(f"unexpected burnout of coder {burnouts[0].coder}")
    if expected == "burnout" and not burnouts:
        raise Failure("expected a burnout, but none was printed")
    if not burnouts:
        return
    burnout = burnouts[0]
    starts = [
        event.time
        for event in events
        if event.coder == burnout.coder and event.text == "is compiling"
    ]
    last_start = starts[-1] if starts else 0
    deadline = last_start + config.burnout
    delay = burnout.time - deadline
    if delay < -2:
        raise Failure(
            f"coder {burnout.coder} burned out {abs(delay)}ms before deadline"
        )
    if delay > 10:
        raise Failure(
            f"burnout printed {delay}ms after deadline; maximum allowed is 10ms"
        )


def validate_success(events: list[Event], config: Config, expected: str) -> None:
    if expected != "success":
        return
    counts = [0] * (config.coders + 1)
    for event in events:
        if event.text == "is compiling":
            counts[event.coder] += 1
    for coder in range(1, config.coders + 1):
        if counts[coder] != config.required:
            raise Failure(
                f"coder {coder} compiled {counts[coder]} times; "
                f"expected exactly {config.required}"
            )


def validate_run(output: str, config: Config, expected: str) -> None:
    events = parse_events(output, config)
    validate_two_dongles(events, config)
    validate_dongle_timeline(events, config)
    validate_state_order(events, config)
    validate_burnout(events, config, expected)
    validate_success(events, config, expected)


def invalid_argument_tests(program: str) -> list[str]:
    tests = [
        [],
        ["0", "100", "10", "10", "10", "1", "0", "fifo"],
        ["-1", "100", "10", "10", "10", "1", "0", "fifo"],
        ["5", "0", "10", "10", "10", "1", "0", "fifo"],
        ["5", "100", "10", "10", "10", "0", "0", "fifo"],
        ["5", "100", "10", "10", "10", "1", "-1", "fifo"],
        ["five", "100", "10", "10", "10", "1", "0", "fifo"],
        ["5", "100", "10", "10", "10", "1", "0", "FIFO"],
        ["5", "100", "10", "10", "10", "1", "0", "bad"],
        ["5", "100", "10", "10", "10", "1", "0"],
        ["5", "100", "10", "10", "10", "1", "0", "edf", "extra"],
        ["2147483648", "100", "10", "10", "10", "1", "0", "edf"],
    ]
    failures: list[str] = []
    for index, arguments in enumerate(tests, 1):
        try:
            result = run_process([program, *arguments], 2)
            if result.returncode == 0:
                failures.append(f"invalid-input #{index} returned exit status 0")
        except Failure as exc:
            failures.append(f"invalid-input #{index}: {exc}")
    return failures


def configured_cases(repeat: int) -> list[Case]:
    return [
        Case("single-coder burnout", Config(1, 300, 50, 50, 50, 2, 0, "fifo"), "burnout", max(2, repeat // 2)),
        Case("forced burnout", Config(5, 250, 200, 100, 100, 3, 0, "edf"), "burnout", max(2, repeat // 2)),
        Case("FIFO baseline", Config(5, 4000, 100, 100, 100, 4, 0, "fifo"), "success", repeat),
        Case("EDF baseline", Config(5, 2000, 100, 100, 100, 5, 0, "edf"), "success", repeat),
        Case("EDF cooldown 400", Config(5, 3500, 200, 100, 100, 4, 400, "edf"), "success", repeat),
        Case("EDF cooldown 800", Config(5, 4500, 200, 100, 100, 4, 800, "edf"), "success", repeat),
        Case("EDF even contention", Config(10, 3500, 100, 100, 100, 4, 300, "edf"), "success", repeat),
        Case("EDF large", Config(50, 6000, 100, 100, 100, 3, 200, "edf"), "success", max(3, repeat // 2)),
    ]


def run_cases(program: str, log_dir: Path, repeat: int) -> list[str]:
    failures: list[str] = []
    for case in configured_cases(repeat):
        print(color("[CASE]", BLUE), case.name, " ".join(case.config.argv()))
        case_failed = False
        for iteration in range(1, case.repeats + 1):
            started = time.monotonic()
            try:
                result = run_process([program, *case.config.argv()], 30)
                elapsed = time.monotonic() - started
                log_path = log_dir / f"{case.name.replace(' ', '_')}_{iteration}.log"
                log_path.write_text(result.stdout, encoding="utf-8")
                if result.stderr:
                    (log_dir / f"{case.name.replace(' ', '_')}_{iteration}.stderr.log").write_text(
                        result.stderr, encoding="utf-8"
                    )
                if result.returncode != 0:
                    raise Failure(f"exit status {result.returncode}")
                validate_run(result.stdout, case.config, case.expected)
                print(color("[PASS]", GREEN), f"run {iteration}/{case.repeats} ({elapsed:.2f}s)")
            except Failure as exc:
                case_failed = True
                message = f"{case.name}, run {iteration}: {exc}"
                failures.append(message)
                print(color("[FAIL]", RED), message)
        if not case_failed:
            print(color("[PASS]", GREEN), case.name)
    return failures


def run_valgrind(program: str, log_dir: Path) -> list[str]:
    if shutil.which("valgrind") is None:
        print(color("[SKIP]", YELLOW), "Valgrind/Helgrind are not installed")
        return []
    config = Config(5, 2000, 100, 100, 100, 3, 0, "edf")
    failures: list[str] = []
    commands = {
        "valgrind": [
            "valgrind", "--leak-check=full", "--show-leak-kinds=all",
            "--errors-for-leak-kinds=all", "--error-exitcode=97",
            program, *config.argv(),
        ],
        "helgrind": [
            "valgrind", "--tool=helgrind", "--error-exitcode=98",
            program, *config.argv(),
        ],
    }
    for name, command in commands.items():
        try:
            result = run_process(command, 90)
            (log_dir / f"{name}.log").write_text(
                result.stdout + result.stderr, encoding="utf-8"
            )
            if result.returncode != 0 or "ERROR SUMMARY: 0 errors" not in result.stderr:
                raise Failure(f"{name} reported errors (see {log_dir / (name + '.log')})")
            print(color("[PASS]", GREEN), name)
        except Failure as exc:
            failures.append(str(exc))
            print(color("[FAIL]", RED), exc)
    return failures


def main() -> int:
    parser = argparse.ArgumentParser(description="Strict Codexion black-box tester")
    parser.add_argument("--program", default="./codexion")
    parser.add_argument("--repeat", type=int, default=10)
    parser.add_argument("--skip-valgrind", action="store_true")
    parser.add_argument("--logs", default="strict_test_logs")
    arguments = parser.parse_args()

    if arguments.repeat < 1:
        parser.error("--repeat must be positive")
    program_path = Path(arguments.program)
    if not program_path.exists() or not os.access(program_path, os.X_OK):
        print(color("[FAIL]", RED), f"{arguments.program} is missing or not executable")
        print("Run make first, then execute this tester from the project root.")
        return 2

    program = str(program_path.resolve())
    log_dir = Path(arguments.logs)
    log_dir.mkdir(parents=True, exist_ok=True)
    failures: list[str] = []

    print(color("[SECTION]", BLUE), "Invalid arguments")
    invalid_failures = invalid_argument_tests(program)
    failures.extend(invalid_failures)
    if invalid_failures:
        for failure in invalid_failures:
            print(color("[FAIL]", RED), failure)
    else:
        print(color("[PASS]", GREEN), "all invalid arguments were rejected")

    print("\n" + color("[SECTION]", BLUE), "Concurrency and timing")
    failures.extend(run_cases(program, log_dir, arguments.repeat))

    if not arguments.skip_valgrind:
        print("\n" + color("[SECTION]", BLUE), "Memory and thread tools")
        failures.extend(run_valgrind(program, log_dir))

    print("\n" + "=" * 60)
    if failures:
        print(color(f"[FAILED] {len(failures)} failure(s)", RED))
        for failure in failures:
            print(" -", failure)
        print(f"Logs: {log_dir}")
        return 1

    print(color("[PASS] all observable strict tests passed", GREEN))
    print(
        color("[IMPORTANT]", YELLOW),
        "Official stdout cannot prove FIFO/EDF request ordering. "
        "Audit or temporarily trace REQUEST and GRANT events before submission.",
    )
    print(f"Logs: {log_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
