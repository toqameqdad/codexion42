#!/usr/bin/env bash

# Extra Codexion tests: build, invalid input, functional edge cases,
# output invariants, and Valgrind leak checking.
#
# Usage:
#   chmod +x edge_tests.sh
#   ./edge_tests.sh
#   TIMEOUT=15 VALGRIND_TIMEOUT=90 ./edge_tests.sh

set -u

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$ROOT_DIR" || exit 1

TIMEOUT="${TIMEOUT:-8}"
VALGRIND_TIMEOUT="${VALGRIND_TIMEOUT:-90}"
TMP_DIR="${TMPDIR:-/tmp}/codexion-edge-tests-$$"
mkdir -p "$TMP_DIR"

PASS=0
FAIL=0
SKIP=0

cleanup()
{
	rm -rf "$TMP_DIR"
}
trap cleanup EXIT INT TERM

pass()
{
	printf '[PASS] %s\n' "$1"
	PASS=$((PASS + 1))
}

fail()
{
	printf '[FAIL] %s\n' "$1"
	FAIL=$((FAIL + 1))
}

skip()
{
	printf '[SKIP] %s\n' "$1"
	SKIP=$((SKIP + 1))
}

run_program()
{
	local name="$1"
	shift
	timeout "${TIMEOUT}s" ./codexion "$@" \
		>"$TMP_DIR/$name.out" 2>"$TMP_DIR/$name.err"
	return $?
}

assert_invalid()
{
	local name="$1"
	shift
	local status

	run_program "$name" "$@"
	status=$?
	if [ "$status" -eq 1 ] \
		&& grep -q 'Error: invalid arguments' "$TMP_DIR/$name.err"; then
		pass "$name rejects invalid arguments"
	else
		fail "$name expected exit 1 and invalid-arguments error (got $status)"
	fi
}

assert_finishes()
{
	local name="$1"
	shift
	local status

	run_program "$name" "$@"
	status=$?
	if [ "$status" -eq 0 ]; then
		pass "$name finishes successfully"
	else
		fail "$name did not finish successfully (status $status)"
	fi
}

assert_contains()
{
	local name="$1"
	local pattern="$2"
	if grep -q "$pattern" "$TMP_DIR/$name.out"; then
		pass "$name contains '$pattern'"
	else
		fail "$name does not contain '$pattern'"
	fi
}

assert_not_contains()
{
	local name="$1"
	local pattern="$2"
	if grep -q "$pattern" "$TMP_DIR/$name.out"; then
		fail "$name unexpectedly contains '$pattern'"
	else
		pass "$name does not contain '$pattern'"
	fi
}

printf '%s\n' '=== CODEXION EXTRA TESTS ==='
printf 'Project: %s\n' "$ROOT_DIR"
printf 'Timeout per normal run: %ss\n' "$TIMEOUT"
printf 'Timeout for Valgrind: %ss\n\n' "$VALGRIND_TIMEOUT"

printf '%s\n' '--- Build ---'
if make re >"$TMP_DIR/build.log" 2>&1; then
	pass 'make re'
else
	fail 'make re'
	cat "$TMP_DIR/build.log"
	printf '\nSummary: PASS=%s FAIL=%s SKIP=%s\n' "$PASS" "$FAIL" "$SKIP"
	exit 1
fi

printf '%s\n' '--- Invalid arguments ---'
assert_invalid 'missing_args'
assert_invalid 'too_few_args' 2 800 20 20 20 1 0
assert_invalid 'too_many_args' 2 800 20 20 20 1 0 fifo extra
assert_invalid 'zero_coders' 0 800 20 20 20 1 0 fifo
assert_invalid 'negative_coders' -1 800 20 20 20 1 0 fifo
assert_invalid 'zero_burnout' 2 0 20 20 20 1 0 fifo
assert_invalid 'zero_compile' 2 800 0 20 20 1 0 fifo
assert_invalid 'zero_debug' 2 800 20 0 20 1 0 fifo
assert_invalid 'zero_refactor' 2 800 20 20 0 1 0 fifo
assert_invalid 'zero_required' 2 800 20 20 20 0 0 fifo
assert_invalid 'negative_time' 2 -1 20 20 20 1 0 fifo
assert_invalid 'negative_required' 2 800 20 20 20 -1 0 fifo
assert_invalid 'negative_cooldown' 2 800 20 20 20 1 -1 fifo
assert_invalid 'bad_scheduler' 2 800 20 20 20 1 0 round_robin
assert_invalid 'text_argument' two 800 20 20 20 1 0 fifo
assert_invalid 'long_overflow' 2 9223372036854775808 20 20 20 1 0 fifo
assert_invalid 'required_overflow' 2 800 20 20 20 2147483648 0 fifo

printf '%s\n' '--- Functional edge cases ---'
assert_finishes 'one_coder' 1 80 10 10 10 1 0 fifo
assert_contains 'one_coder' 'burned out'
assert_not_contains 'one_coder' 'is compiling'

assert_finishes 'two_coders_fifo' 2 1000 10 10 10 2 0 fifo
assert_finishes 'two_coders_edf' 2 1000 10 10 10 2 0 edf
assert_finishes 'odd_coders' 5 2000 5 5 5 2 0 edf
assert_finishes 'many_coders' 50 2000 1 1 1 1 0 fifo
assert_finishes 'cooldown' 4 3000 5 5 5 2 50 fifo
assert_finishes 'forced_burnout' 4 50 20 20 20 1 100 fifo
assert_contains 'forced_burnout' 'burned out'

printf '%s\n' '--- Output format and invariants ---'
run_program 'format' 4 2000 5 5 5 2 0 edf
format_status=$?
if [ "$format_status" -eq 0 ] \
	&& awk 'NF < 3 || $1 !~ /^[0-9]+$/ || $2 !~ /^[0-9]+$/ { bad = 1 } END { exit bad }' \
		"$TMP_DIR/format.out"; then
	pass 'every log line has timestamp, coder id and message'
else
	fail 'log format has an invalid timestamp, coder id or missing message'
fi

acquire_count=$(grep -c 'has taken a dongle' "$TMP_DIR/format.out" || true)
if [ "$((acquire_count % 2))" -eq 0 ]; then
	pass 'dongle acquisition messages are paired'
else
	fail 'dongle acquisition messages are not paired'
fi

printf '%s\n' '--- Valgrind ---'
if ! command -v valgrind >/dev/null 2>&1; then
	skip 'Valgrind is not installed; install it with: sudo apt install valgrind'
else
	# Valgrind slows pthread programs down considerably. Use a separate,
	# generous timeout so a timeout is not mistaken for a memory error.
	VG_LOG="$TMP_DIR/valgrind.log"
	VG_OUT="$TMP_DIR/valgrind.stdout"
	VG_ERR="$TMP_DIR/valgrind.stderr"

	timeout "${VALGRIND_TIMEOUT}s" valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--errors-for-leak-kinds=definite,indirect,possible \
		--error-exitcode=99 \
		--log-file="$VG_LOG" \
		./codexion 4 10000 20 20 20 3 0 edf \
		>"$VG_OUT" 2>"$VG_ERR"
	vg_status=$?

	if [ "$vg_status" -eq 124 ]; then
		fail "Valgrind timed out after ${VALGRIND_TIMEOUT}s"
		printf '%s\n' '--- Valgrind tail ---'
		tail -80 "$VG_LOG" 2>/dev/null || true
	elif [ ! -s "$VG_LOG" ]; then
		fail 'Valgrind produced no report'
		cat "$VG_ERR"
	elif [ "$vg_status" -eq 0 ] \
		&& grep -Eq 'in use at exit:[[:space:]]*0 bytes' "$VG_LOG" \
		&& grep -q 'All heap blocks were freed -- no leaks are possible' "$VG_LOG" \
		&& grep -Eq 'ERROR SUMMARY:[[:space:]]*0 errors' "$VG_LOG"; then
		pass 'Valgrind reports no leaks or errors'
	else
		fail "Valgrind found errors or leaks (exit status $vg_status)"
		printf '%s\n' '--- Valgrind summary lines ---'
		grep -E 'in use at exit|definitely lost|indirectly lost|possibly lost|still reachable|ERROR SUMMARY' \
			"$VG_LOG" 2>/dev/null || true
		printf '%s\n' '--- Valgrind tail ---'
		tail -80 "$VG_LOG"
	fi
fi

printf '\nSummary: PASS=%s FAIL=%s SKIP=%s\n' "$PASS" "$FAIL" "$SKIP"

if [ "$FAIL" -eq 0 ]; then
	exit 0
fi
exit 1
