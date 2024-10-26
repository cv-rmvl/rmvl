import sys

sys.dont_write_bytecode = True  # Don't generate .pyc files / __pycache__ directories

import argparse
import json

py_signatures = dict[str, list[dict[str, str]]]()


def add_fn(ns: str, name: str, arg: str, ret: str):
    """
    ### Add a function signature to the "py_signatures" dictionary
    #### Parameters
    - `ns` ─ namespace
    - `name` ─ function name, used as the key in the dictionary (or `[[call]]` for callable objects)
    - `arg` ─ function arguments
    - `ret` ─ function return type (or `[[con]]` for constructors, `[[this]]` for this object)
    #### Notes
    - Strings in `ret` that appear in the form of `[[xxx]]` represent special return value types, such as `[[con]]` and `[[this]]`.
    - Other strings in the form of `[[xxx]]` do not have special meanings and will be replaced with `<xxx>`.
    - All `"_"` in the return value will be replaced with spaces `" "`.
    """
    full_name = f"{ns}::{name}"
    dst_name = f"{ns}::{name}"
    dst_arg = arg
    dst_ret = ret
    
    # replace special function names
    if name == "[[call]]":
        full_name = f"{ns}::operator()"
        dst_name = f"{ns}::__call__"
    elif name == "[[eq]]":
        full_name = f"{ns}::operator=="
        dst_name = f"{ns}::__eq__"
    elif name == "[[ne]]":
        full_name = f"{ns}::operator!="
        dst_name = f"{ns}::__ne__"
    elif name == "[[get]]":
        full_name = f"{ns}::operator[]"
        dst_name = f"{ns}::__getitem__"
    elif name == "[[set]]":
        full_name = f"{ns}::operator[]"
        dst_name = f"{ns}::__setitem__"

    # replace special return values
    if ret == "[[con]]":
        dst_name = ns
        dst_ret = f"<{name} object>"
    elif ret == "[[this]]":
        dst_ret = f"<{ns} object>"

    # add to the dictionary
    if full_name not in py_signatures:
        py_signatures[full_name] = []
    ns = ns.replace("::", ".")
    dst_name = dst_name.replace("::para::", ".").replace("::", ".")
    dst_arg = arg.replace(",", ", ")
    dst_ret = dst_ret.replace("::", ".").replace(",", ", ").replace("[[", "<").replace("]]", ">").replace("_", " ")
    
    py_signatures[full_name].append(
        {"name": dst_name, "arg": dst_arg, "ret": dst_ret}
    )


def add_const(ns: str, name: str, val: str):
    """
    ### Add a constant signature to the "py_signatures" dictionary
    #### Parameters
    - `ns` ─ namespace
    - `name` ─ constant value name
    - `val` ─ constant value
    """
    full_name = f"{ns}::{name}"
    if full_name not in py_signatures:
        py_signatures[full_name] = []
    dst_name = f"{ns}::{name}"
    
    dst_name = dst_name.replace("::", ".")
    dst_val = val.replace(",", ", ")
    py_signatures[full_name].append(
        {"name": dst_name, "value": dst_val}
    )


def gen_doc(file):
    """
    ### Generate the JSON file
    #### Parameters
    - `file` ─ output JSON file
    """
    with open(file, "wt") as f:
        json.dump(py_signatures, f, indent=4)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("output_file", type=str, help="Output JSON file")
    parser.add_argument(
        "--fns",
        type=str,
        help="Function signatures, separated by '#', contains 4 elements: 'ns', 'name', 'arg', 'ret'",
    )
    parser.add_argument(
        "--consts",
        type=str,
        help="Constant signatures, separated by '#', contains 3 elements: 'ns', 'name', 'val'",
    )
    parser.add_argument("--clear", action="store_true", help="Clear all signatures")

    args = parser.parse_args()

    if args.clear:
        py_signatures.clear()
        gen_doc(args.output_file)
        sys.exit(0)

    # load existing signatures
    try:
        with open(args.output_file, "rt") as f:
            py_signatures = json.load(f)
    except FileNotFoundError:
        py_signatures = {}

    if args.fns:
        fns = args.fns.split("#")
        for i in range(0, len(fns), 4):
            add_fn(fns[i], fns[i + 1], fns[i + 2], fns[i + 3])

    if args.consts:
        consts = args.consts.split("#")
        for i in range(0, len(consts), 3):
            add_const(consts[i], consts[i + 1], consts[i + 2])

    gen_doc(args.output_file)
