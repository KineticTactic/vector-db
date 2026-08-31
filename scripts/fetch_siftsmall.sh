#!/usr/bin/env bash
# Downloads the siftsmall dataset used by the Phase 0 Recall@100 validation.
#
# Fetches siftsmall.tar.gz from the TEXMEX corpus and extracts it into
# data/siftsmall/ at the repository root. The data/ directory is gitignored.
# Re-running is a no-op once the three required files are present and the right
# size; pass --force to download again.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
data_dir="$repo_root/data"
dest_dir="$data_dir/siftsmall"
archive="$data_dir/siftsmall.tar.gz"
url="ftp://ftp.irisa.fr/local/texmex/corpus/siftsmall.tar.gz"

force=0
[ "${1:-}" = "--force" ] && force=1

# Sizes follow from the .fvecs/.ivecs layout (4-byte dimension + 4 bytes per
# element), so a wrong size means a truncated or wrong download.
names=(siftsmall_base.fvecs siftsmall_query.fvecs siftsmall_groundtruth.ivecs)
sizes=(5160000 51600 40400)

file_size() { wc -c <"$1" | tr -d ' '; }

dataset_ok() {
    local i
    for i in "${!names[@]}"; do
        local path="$dest_dir/${names[$i]}"
        [ -f "$path" ] || return 1
        [ "$(file_size "$path")" = "${sizes[$i]}" ] || return 1
    done
    return 0
}

if dataset_ok && [ "$force" -eq 0 ]; then
    echo "siftsmall already present in $dest_dir - nothing to do."
    exit 0
fi

mkdir -p "$data_dir"

echo "Downloading $url ..."
if ! curl -fL --retry 2 -o "$archive" "$url"; then
    echo "Download failed." >&2
    echo "If FTP is blocked on this network, download siftsmall.tar.gz manually from" >&2
    echo "http://corpus-texmex.irisa.fr/ and extract it so the files land in:" >&2
    echo "  $dest_dir" >&2
    exit 1
fi

echo "Extracting into $data_dir ..."
tar -xzf "$archive" -C "$data_dir"
rm -f "$archive"

for i in "${!names[@]}"; do
    path="$dest_dir/${names[$i]}"
    if [ ! -f "$path" ]; then
        echo "missing after extraction: $path" >&2
        exit 1
    fi
    actual="$(file_size "$path")"
    if [ "$actual" != "${sizes[$i]}" ]; then
        echo "${names[$i]} is $actual bytes, expected ${sizes[$i]}" >&2
        exit 1
    fi
    printf '  ok  %-28s %10s bytes\n' "${names[$i]}" "$actual"
done

echo "siftsmall ready in $dest_dir"
