#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
if [ "$#" -ne 1 ]; then echo "usage: $0 <1|2|3>" >&2; exit 2; fi
case "$1" in 1|2|3) target=$1 ;; *) echo "milestone must be 1, 2, or 3" >&2; exit 2 ;; esac
current=0
if [ -f .milestone ]; then current=$(cat .milestone); fi
case "$current" in ''|*[!0-9]*) echo "Invalid .milestone state" >&2; exit 1 ;; esac
case "$target" in
  1) patch=patches/0001-m1-pseudo3d.patch ;;
  2) patch=patches/0002-m2-player-collision.patch ;;
  3) patch=patches/0003-m3-graves-input.patch ;;
esac
if [ "$target" -le "$current" ]; then echo "M$target is already applied (current M$current); use a fresh copy or reverse the patch first." >&2; exit 1; fi
if [ "$target" -ne $((current + 1)) ]; then echo "Apply milestones in order. Current M$current; next is M$((current + 1))." >&2; exit 1; fi
git apply --check "$patch"
git apply "$patch"
echo "$target" > .milestone
echo "Applied M$target. Review changes and run the smoke test."
