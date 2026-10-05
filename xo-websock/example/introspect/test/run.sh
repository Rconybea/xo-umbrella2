#!/usr/bin/env bash
#
# xo-websock/example/introspect/test/run.sh
#
# Runs the introspect page's browser tests: each drives the real page in
# headless chrome over the Chrome DevTools Protocol, against a fresh
# websock_ex_introspect server.  See .xo-backlog/xo-websock/issues/14.
#
# usage: run.sh SERVER [TEST..]
#   SERVER  path to the built websock_ex_introspect
#   TEST    test names (e.g. rows anchor); default: all
#
# needs: node (>= 22: global WebSocket and fetch), curl, and chrome --
#   $CHROME if set, else google-chrome, chromium, or /opt/google/chrome/chrome
#
# Screenshots go to a temporary directory, printed at the end.  Exit status:
# 0 iff every test passed.

set -u

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# source links (rows, receiver) resolve against the repo root
src_tree="$(cd "${here}/../../../.." && pwd)"

if [[ $# -lt 1 ]]; then
    echo "usage: $0 SERVER [TEST..]" >&2
    exit 2
fi

server="$1"
shift

all_tests=(rows visibility noticker anchor transitions sink receiver nested
           sender_expand endpoint_expand sesstable_expand urlrouter_expand
           sub_expand router_expand session_expand cdp_menu expand edge_kinds
           header refs_resolve)

if [[ $# -gt 0 ]]; then
    tests=("$@")
else
    tests=("${all_tests[@]}")
fi

if [[ ! -x "${server}" ]]; then
    echo "$0: server not executable: ${server}" >&2
    exit 2
fi

for tool in node curl; do
    if ! command -v "${tool}" >/dev/null; then
        echo "$0: ${tool} not found" >&2
        exit 2
    fi
done

chrome="${CHROME:-}"
if [[ -z "${chrome}" ]]; then
    for c in google-chrome chromium /opt/google/chrome/chrome; do
        if command -v "${c}" >/dev/null; then
            chrome="${c}"
            break
        fi
    done
fi
if [[ -z "${chrome}" ]]; then
    echo "$0: no chrome found: set CHROME" >&2
    exit 2
fi

# a port the kernel says is free now
free_port() {
    node -e 'const s = require("net").createServer().listen(0, () => { console.log(s.address().port); s.close(); })'
}

# true once URL $1 answers, within ~10s
wait_for() {
    for _ in $(seq 100); do
        curl -s -o /dev/null "$1" && return 0
        sleep 0.1
    done
    return 1
}

work="$(mktemp -d -t introspect-test.XXXXXX)"
chrome_pid=""
server_pid=""

cleanup() {
    [[ -n "${server_pid}" ]] && kill "${server_pid}" 2>/dev/null
    [[ -n "${chrome_pid}" ]] && kill "${chrome_pid}" 2>/dev/null
    wait 2>/dev/null
}
trap cleanup EXIT

cdp_port="$(free_port)"
"${chrome}" --headless=new --remote-debugging-port="${cdp_port}" \
            --user-data-dir="${work}/chrome-profile" \
            --noerrdialogs --no-first-run --ozone-platform=headless \
            >"${work}/chrome.log" 2>&1 &
chrome_pid=$!

if ! wait_for "http://localhost:${cdp_port}/json/version"; then
    echo "$0: chrome did not start: see ${work}/chrome.log" >&2
    exit 2
fi

n_pass=0
failed=()

for t in "${tests[@]}"; do
    script="${here}/${t}.mjs"
    if [[ ! -f "${script}" ]]; then
        echo "${t}: no such test (${script})"
        failed+=("${t}")
        continue
    fi

    port="$(free_port)"
    extra=()
    case "${t}" in
        rows|receiver) extra=(--src-tree="${src_tree}") ;;
    esac

    "${server}" "${port}" "${extra[@]}" >"${work}/${t}.server.log" 2>&1 &
    server_pid=$!

    if ! wait_for "http://localhost:${port}/"; then
        echo "${t}: FAIL (server did not start: ${work}/${t}.server.log)"
        failed+=("${t}")
        kill "${server_pid}" 2>/dev/null; wait "${server_pid}" 2>/dev/null; server_pid=""
        continue
    fi

    # each test's own arguments
    case "${t}" in
        cdp_menu)     args=("${cdp_port}" "http://localhost:${port}/") ;;
        expand)       args=("${cdp_port}" "${port}" "${work}/${t}-1.png" "${work}/${t}-2.png") ;;
        refs_resolve) args=("${cdp_port}" "${port}") ;;
        *)            args=("${cdp_port}" "${port}" "${work}/${t}.png") ;;
    esac

    if timeout 180 node "${script}" "${args[@]}" >"${work}/${t}.log" 2>&1; then
        echo "${t}: ok"
        n_pass=$((n_pass + 1))
    else
        echo "${t}: FAIL"
        grep -E '^FAIL|Error' "${work}/${t}.log" | head -5 | sed 's/^/    /'
        failed+=("${t}")
    fi

    kill "${server_pid}" 2>/dev/null; wait "${server_pid}" 2>/dev/null; server_pid=""
done

echo "${n_pass} passed, ${#failed[@]} failed; logs and screenshots in ${work}"
[[ ${#failed[@]} -eq 0 ]]
