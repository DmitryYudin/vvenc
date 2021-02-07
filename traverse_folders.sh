set -eu
readonly LLVM_ROOT='C:\work\devscripts\bin\app\llvm\11.0.1'

entrypoint()
{
    [[ ! -f .clang-format ]] && echo "error: can't find .clang-format" && exit 1

    export PATH=$(cygpath -p "$LLVM_ROOT/bin"):$PATH
    process_single_file() { echo "$1" && command clang-format -i "$1"; }

    traverse_folder "." process_single_file
}

traverse_folder()
{
    local dirRoot=$1; shift
    local foo=$1; shift
    local filepath
    for filepath in "$dirRoot"/*; do
        [[ -d "$filepath" ]] && traverse_folder "$filepath" "$foo" && continue
        [[ ! -f "$filepath" ]] && echo "skip: $filepath" && continue
        case ${filepath##*.} in c|cc|cpp|cxx|h|hh|hpp|hxx|inl) $foo "$filepath"; esac
    done
}

entrypoint "$@"