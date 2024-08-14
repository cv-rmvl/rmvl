import re
import argparse
from collections import defaultdict


def re_match(mode: str, line: str) -> re.Match | None:
    """
    ### 正则表达式匹配 C++ 头文件中的某一行

    #### 参数
    `mode`: `str` ─ 匹配模式，可选值为:
    - `normal_class`: 匹配普通类定义
    - `aggregate_class`: 匹配聚合类定义
    - `normal_aggregate_class`: 匹配普通类和聚合类定义
    - `inherited_class`: 匹配继承类定义
    - `global_function`: 匹配全局函数定义
    - `constructor`: 匹配类构造函数定义
    - `method`: 匹配类方法定义
    - `static_method`: 匹配类静态方法定义
    - `const_method`: 匹配类常量方法定义
    - `convert_method`: 匹配类用户定义转换函数定义
    - `member`: 匹配类成员变量定义
    - `variable`: 匹配全局变量定义

    `line`: `str` ─ 包含 C++ 头文件行的字符串

    #### 返回值
    `(re.Match | None)`: 匹配结果，如果没有匹配则返回 `None`
    """
    if mode == "normal_class":
        return re.search(r"(class|struct)\s+RMVL_EXPORTS_W\s+(\w+)\s*(?:final)?$", line)
    elif mode == "aggregate_class":
        return re.search(
            r"(class|struct)\s+RMVL_EXPORTS_W_AG\s+(\w+)\s*(?:final)?$", line
        )
    elif mode == "normal_aggregate_class":
        return re.search(
            r"(class|struct)\s+RMVL_EXPORTS_(?:W|W_AG)\s+(\w+)\s*(?:final)?$", line
        )
    elif mode == "inherited_class":
        return re.search(
            r"(class|struct)\s+RMVL_EXPORTS_W\s+(\w+)\s*(?:final)?:\s+(public)?\s+(\w+)\s*$",
            line,
        )
    elif mode == "global_function":
        return re.search(
            r"RMVL_EXPORTS_W\s*([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)",
            line,
        )
    # RMVL_W
    elif mode == "constructor":
        return re.search(
            r"RMVL_W\s+(?:constexpr\s+)?(\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)", line
        )
    elif mode == "method":
        return re.search(
            r"RMVL_W\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(operator\(\)|operator==|operator!=|operator\+|operator-|\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)",
            line,
        )
    elif mode == "static_method":
        return re.search(
            r"RMVL_W\s+static\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)",
            line,
        )
    elif mode == "const_method":
        return re.search(
            r"RMVL_W\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*const",
            line,
        )
    elif mode == "convert_method":
        return re.search(
            r"RMVL_W\s+operator\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s*\(\)",
            line,
        )
    # RMVL_W_RW
    elif mode == "member":
        return re.search(r"RMVL_W_RW\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(\w+)", line)
    elif mode == "variable":
        return re.search(
            r"RMVL_W_RW\s+(?:constexpr\s+|inline\s+)([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(\w+)",
            line,
        )
    else:
        return None


def split_parameters(params) -> tuple[list[str], list[str], list[str]]:
    """
    ### split parameters and default values from a string containing parameters

    #### Parameters
    `params`: `str` ─ string containing parameters and optional default values

    #### Returns
    `tuple[list[str], list[str], list[str]]`: a tuple containing three lists:
    - List of parameter types
    - List of parameter names
    - List of default values, if a parameter has no default value, the corresponding element will be `None`

    #### Example
    ```py
    split_parameters('int a, const float b=3.14, string c')
    # (['int', 'const float', 'string'], ['a', 'b', 'c'], [None, '3.14', None])

    ```
    """

    type_names = []
    param_names = []
    default_values = []

    datas = [p.strip() for p in params.split(",") if p.strip()]
    for data in datas:
        # Check if there is a default value
        if "=" in data:
            param_part, default = data.split("=", 1)
            default = default.strip()
        else:
            param_part = data
            default = None

        param_part = param_part.strip()

        # Find the last word (parameter name)
        parts = param_part.split()
        param_name = parts[-1]
        type_name = " ".join(parts[:-1])

        # Handle pointers and references in parameter names
        while param_name.startswith(("*", "&")):
            type_name += param_name[0]
            param_name = param_name[1:]

        type_names.append(type_name)
        param_names.append(param_name)
        default_values.append(default)

    return type_names, param_names, default_values


types = {
    # fundamental types
    "void": "None",
    "int": "int",
    "uint8_t": "int",
    "uint16_t": "int",
    "uint32_t": "int",
    "size_t": "int",
    "float": "float",
    "double": "float",
    "bool": "bool",
    "string": "str",
    "string_view": "str",
    # tuple types
    "pair<int, int>": "tuple[int, int]",
    "pair<float, float>": "tuple[float, float]",
    "pair<double, double>": "tuple[float, float]",
    "pair<vector<int>, int>": "tuple[list[int], int]",
    "pair<vector<float>, float>": "tuple[list[float], float]",
    "pair<vector<double>, double>": "tuple[list[float], float]",
    # list types
    "vector<int>": "list[int]",
    "vector<float>": "list[float]",
    "vector<double>": "list[float]",
    "vector<string>": "list[str]",
    "vector<vector<int>>": "list[list[int]]",
    "vector<vector<float>>": "list[list[float]]",
    "vector<vector<double>>": "list[list[float]]",
    "vector<vector<string>>": "list[list[str]]",
    # callable types
    "function<void()>": "Callable[[], None]",
    "function<void(int)>": "Callable[[int], None]",
    "function<float(float)>": "Callable[[float], float]",
    "function<double(double)>": "Callable[[float], float]",
    "function<double(double, vector<double>)>": "Callable[[float, list[float]], float]",
}


def remove_t(s: str) -> str:
    """
    ### Remove `const`, `&`, `*`, `namespace`, `inline`, `constexpr` from a C++ type string
    #### Parameters
    `s` ─ C++ type string
    #### Returns
    `(str)`: string without modifiers
    """
    # remove_cvref
    s = re.sub(r"const\s+|&", "", s)
    # remove namespace
    s = re.sub(r"\w+::", "", s)
    # remove inline and constexpr
    s = re.sub(r"constexpr\s+|inline\s+", "", s)
    return s


def type_convert(cpp_type: str) -> str:
    """
    ### Convert C++ type to Python type annotation

    #### Parameters
    `cpp_type` ─ string containing C++ type

    #### Returns
    `(str)`: Python type annotation

    #### Example
    ```python
    type_convert('vector<int>')

    ```
    returns

    ```
    'list[int]'

    ```
    """

    return types.get(remove_t(cpp_type), "Any")


def generate_python_binding(lines: list[str]) -> str:
    """
    ### Generate py binding code from C++ header file
    #### Parameters
    `lines`: `list[str]` ─ list of strings containing C++ header file lines

    #### Returns
    `(str)`: generated py binding code
    """

    binding_code = []
    # To store functions for overload detection
    class_content = []
    cur_class = None
    functions = defaultdict(list)
    convert_content = []
    # To store enum definitions
    enums = []
    in_enum = False
    cur_enum = None

    for line in lines:
        line = line.strip()

        if line.startswith("//") or not line:
            continue
        # Enum detection
        if line.startswith("enum class") or (in_enum and line.startswith("{")):
            in_enum = True
            enum_match = re.search(r"enum class\s+(\w+)", line)
            if enum_match:
                cur_enum = [enum_match.group(1), []]
        elif in_enum and "}" in line:
            in_enum = False
            if cur_enum:
                enums.append(cur_enum)
                cur_enum = None
        elif in_enum and cur_enum:
            enum_values = line.split(",")
            for value in enum_values:
                value = value.strip()
                if value and not value.startswith("}") and not value.startswith("//"):
                    cur_enum[1].append(value.split("=")[0].strip())

        # Class or function with 'RMVL_EXPORTS_W' macro
        elif "RMVL_EXPORTS_W" in line:
            # Start of a new class
            if line.startswith("class") or line.startswith("struct"):
                # Normal class
                mch = re_match("normal_class", line)
                if mch:
                    cur_class = mch.group(2)
                    class_content.append(
                        f'    py::class_<{cur_class}>(m, "{cur_class}")'
                    )
                # Aggregate class
                mch = re_match("aggregate_class", line)
                if mch:
                    cur_class = mch.group(2)
                    class_content.append(
                        f'    py::class_<{cur_class}>(m, "{cur_class}")'
                    )
                    class_content.append("        .def(py::init<>())")
                # Inherited class
                mch = re_match("inherited_class", line)
                if mch:
                    cur_class = mch.group(2)
                    class_content.append(
                        f'    py::class_<{cur_class}, {mch.group(4)}>(m, "{cur_class}")'
                    )

            # Global normal function
            else:
                mch = re_match("global_function", line)
                if mch:
                    return_type, func_name, params = mch.groups()
                    type_list, id_list, default_list = split_parameters(params)
                    functions[func_name].append((type_list, id_list, default_list))

        # Class method with 'RMVL_W[_xxx]' macro
        elif line.startswith("RMVL_W") and cur_class:
            # Constructor
            mch = re_match("constructor", line)
            if mch:
                name, params = mch.groups()
                type_list, id_list, default_list = split_parameters(params)
                class_content.append(
                    f'        .def(py::init<{", ".join(type_list)}>())'
                )
                continue
            # Convert method
            mch = re_match("convert_method", line)
            if mch:
                cvt_class = mch.groups()[0]
                convert_content.append(
                    f"    py::implicitly_convertible<{cur_class}, {cvt_class}>();"
                )
                continue
            # Static method
            mch = re_match("static_method", line)
            if mch:
                return_type, method_name, params = mch.groups()
                type_list, id_list, default_list = split_parameters(params)
                class_content.append(
                    f'        .def_static("{method_name}", py::overload_cast<{", ".join(type_list)}>(&{cur_class}::{method_name}),'
                )
                for type, id, default in zip(type_list, id_list, default_list):
                    if default == "{}":
                        class_content.append(
                            f'             "{id}"_a = {remove_t(type)}{{}},'
                        )
                    elif default:
                        class_content.append(f'             "{id}"_a = {default},')
                    else:
                        class_content.append(f'             "{id}"_a,')
                class_content[-1] = class_content[-1][:-1] + ")"
                continue
            # Const method
            mch = re_match("const_method", line)
            if mch:
                return_type, method_name, params = mch.groups()
                type_list, id_list, default_list = split_parameters(params)
                class_content.append(
                    f'        .def("{method_name}", py::overload_cast<{", ".join(type_list)}>(&{cur_class}::{method_name}, py::const_),'
                )
                for type, id, default in zip(type_list, id_list, default_list):
                    if default == "{}":
                        class_content.append(
                            f'             "{id}"_a = {remove_t(type)}{{}},'
                        )
                    elif default:
                        class_content.append(f'             "{id}"_a = {default},')
                    else:
                        class_content.append(f'             "{id}"_a,')
                class_content[-1] = class_content[-1][:-1] + ")"
                continue
            # Method
            mch = re_match("method", line)
            if mch:
                return_type, method_name, params = mch.groups()
                type_list, id_list, default_list = split_parameters(params)
                # operator() overload
                if method_name == "operator()":
                    class_content.append(
                        f'        .def("__call__", &{cur_class}::operator(),'
                    )
                # operator== overload
                elif method_name == "operator==":
                    class_content.append(
                        f'        .def("__eq__", &{cur_class}::operator==,'
                    )
                # operator!= overload
                elif method_name == "operator!=":
                    class_content.append(
                        f'        .def("__ne__", &{cur_class}::operator!=,'
                    )
                # operator+ overload
                elif method_name == "operator+":
                    class_content.append(
                        f'        .def("__add__", &{cur_class}::operator+,'
                    )
                # operator- overload
                elif method_name == "operator-":
                    class_content.append(
                        f'        .def("__sub__", &{cur_class}::operator-,'
                    )
                # Normal method
                else:
                    class_content.append(
                        f'        .def("{method_name}", py::overload_cast<{", ".join(type_list)}>(&{cur_class}::{method_name}),'
                    )
                for type, id, default in zip(type_list, id_list, default_list):
                    if default == "{}":
                        class_content.append(
                            f'             "{id}"_a = {remove_t(type)}{{}},'
                        )
                    elif default:
                        class_content.append(f'             "{id}"_a = {default},')
                    else:
                        class_content.append(f'             "{id}"_a,')
                class_content[-1] = class_content[-1][:-1] + ")"
                continue
            # Class member variable
            mch = re_match("member", line)
            if mch:
                type_name, var_name = mch.groups()
                class_content.append(
                    f'        .def_readwrite("{var_name}", &{cur_class}::{var_name})'
                )

        # Global variable with 'RMVL_W_RW' macro
        elif line.startswith("RMVL_W_RW") and not cur_class:
            mch = re_match("variable", line)
            if mch:
                type_name, var_name = mch.groups()
                binding_code.append(f'    m.attr("{var_name}") = {var_name};')

        # End of the class
        elif line == "};" and cur_class:
            binding_code.extend(class_content)
            binding_code.append("    ;")
            cur_class = None
            class_content = []

    # Process class convert methods
    binding_code.extend(convert_content)

    # Process global functions
    for func_name, overloads in functions.items():
        if len(overloads) == 1:
            type_list, id_list, default_list = overloads[0]
            binding_code.append(f'    m.def("{func_name}", &{func_name},')
            for type, id, default in zip(type_list, id_list, default_list):
                if default == "{}":
                    binding_code.append(
                        f'             "{id}"_a = {remove_t(type)}{{}},'
                    )
                elif default:
                    binding_code.append(f'             "{id}"_a = {default},')
                else:
                    binding_code.append(f'             "{id}"_a,')
            binding_code[-1] = binding_code[-1][:-1] + ");"
        else:
            for type_list, id_list, default_list in overloads:
                binding_code.append(
                    f'    m.def("{func_name}", py::overload_cast<{", ".join(type_list)}>(&{func_name}),'
                )
                for type, id, default in zip(type_list, id_list, default_list):
                    if default == "{}":
                        binding_code.append(
                            f'          "{id}"_a = {remove_t(type)}{{}},'
                        )
                    elif default:
                        binding_code.append(f'          "{id}"_a = {default},')
                    else:
                        binding_code.append(f'          "{id}"_a,')
                binding_code[-1] = binding_code[-1][:-1] + ");"

    # Process enums
    tmp_bd_code = []
    for enum_name, enum_values in enums:
        tmp_bd_code.append(f'    py::enum_<{enum_name}>(m, "{enum_name}")')
        for value in enum_values:
            tmp_bd_code.append(f'        .value("{value}", {enum_name}::{value})')
        tmp_bd_code.append("        .export_values();")

    binding_code.insert(0, "\n".join(tmp_bd_code))
    return "\n".join(binding_code)


def generate_pyi(lines: list[str]) -> str:
    """
    ### Generate .pyi file content from C++ header file
    #### Parameters
    `lines` ─ list of strings containing C++ header file lines

    #### Returns
    `(str)`: generated .pyi file content
    """

    pyi_content = []
    # To store functions for overload detection
    class_content = []
    cur_class = None
    functions = defaultdict(list)
    # To store enum definitions
    enums = []
    in_enum = False
    cur_enum = None

    for line in lines:
        line = line.strip()

        if line.startswith("//") or not line:
            continue

        # Enum detection
        if line.startswith("enum class") or (in_enum and line.startswith("{")):
            in_enum = True
            enum_match = re.search(r"enum class\s+(\w+)", line)
            if enum_match:
                cur_enum = [enum_match.group(1), []]
        elif in_enum and "}" in line:
            in_enum = False
            if cur_enum:
                enums.append(cur_enum)
                types[cur_enum[0]] = cur_enum[0]
                cur_enum = None
        elif in_enum and cur_enum:
            enum_values = line.split(",")
            for value in enum_values:
                value = value.strip()
                if value and not value.startswith("}") and not value.startswith("//"):
                    cur_enum[1].append(value.split("=")[0].strip())

        # Class or function with 'RMVL_EXPORTS_W' macro
        elif "RMVL_EXPORTS_W" in line:
            # Class definition
            if line.startswith("class") or line.startswith("struct"):
                # Normal class or aggregate class
                mch = re_match("normal_aggregate_class", line)
                if mch:
                    cur_class = mch.group(2)
                    class_content.append(f"class {cur_class}:")
                # Inherited class
                mch = re_match("inherited_class", line)
                if mch:
                    cur_class = mch.group(2)
                    class_content.append(f"class {cur_class}({mch.group(4)}):")
            # Global function definition
            else:
                mch = re_match("global_function", line)
                if mch:
                    return_type, func_name, params = mch.groups()
                    type_list, id_list, default_list = split_parameters(params)
                    functions[func_name].append(
                        (return_type, type_list, id_list, default_list)
                    )

        # Class method with 'RMVL_W[_xxx]' macro
        elif line.startswith("RMVL_W") and cur_class:
            # Constructor
            mch = re_match("constructor", line)
            if mch:
                name, params = mch.groups()
                type_list, id_list, default_list = split_parameters(params)
                param_str = ", ".join(
                    [
                        f"{id}: {type_convert(tp)}" + (f" = {d}" if d else "")
                        for id, tp, d in zip(id_list, type_list, default_list)
                    ]
                ).replace("::", ".")
                class_content.append(
                    f"    @typing.overload\n    def __init__(self, {param_str}) -> None: ..."
                )
                continue
            # Convert method
            mch = re_match("convert_method", line)
            if mch:
                continue
            # Class method and static method
            mch = re_match("method", line)
            if mch:
                return_type, method_name, params = mch.groups()
                type_list, id_list, default_list = split_parameters(params)
                param_str = ", ".join(
                    [
                        f"{id}: {type_convert(t)}" + (f" = {d}" if d else "")
                        for id, t, d in zip(id_list, type_list, default_list)
                    ]
                ).replace("::", ".")
                if method_name == "operator()":
                    method_name = "__call__"
                elif method_name == "operator==":
                    method_name = "__eq__"
                elif method_name == "operator!=":
                    method_name = "__ne__"
                elif method_name == "operator+":
                    method_name = "__add__"
                elif method_name == "operator-":
                    method_name = "__sub__"
                class_content.append(
                    f"    @typing.overload\n    def {method_name}(self, {param_str}) -> {type_convert(return_type)}: ..."
                )
                continue
            # Class member variable
            mch = re_match("member", line)
            if mch:
                type_name, var_name = mch.groups()
                class_content.append(f"    {var_name}: {type_convert(type_name)} = ...")

        # Global variable with 'RMVL_W_RW' macro
        elif line.startswith("RMVL_W_RW") and not cur_class:
            mch = re_match("variable", line)
            if mch:
                type_name, var_name = mch.groups()
                pyi_content.append(f"{var_name}: {type_convert(type_name)} = ...")

        # End of the class
        elif line == "};" and cur_class:
            pyi_content.extend(class_content)
            pyi_content.append("")
            types[cur_class] = cur_class
            cur_class = None
            class_content = []

    # Process global functions
    for func_name, overloads in functions.items():
        if len(overloads) > 1:
            for return_type, type_list, id_list, default_list in overloads:
                param_str = ", ".join(
                    [
                        f"{id}: {type_convert(t)}" + (f" = {d}" if d else "")
                        for id, t, d in zip(id_list, type_list, default_list)
                    ]
                ).replace("::", ".")
                pyi_content.append(f"@typing.overload")
                pyi_content.append(
                    f"@typing.overload\ndef {func_name}({param_str}) -> {type_convert(return_type)}: ..."
                )
        else:
            return_type, type_list, id_list, default_list = overloads[0]
            param_str = ", ".join(
                [
                    f"{id}: {type_convert(t)}" + (f" = {d}" if d else "")
                    for id, t, d in zip(id_list, type_list, default_list)
                ]
            ).replace("::", ".")
            pyi_content.append(
                f"@typing.overload\ndef {func_name}({param_str}) -> {type_convert(return_type)}: ..."
            )

    # Process enums
    for enum_name, enum_values in enums:
        tmp_pyi_content = []
        tmp_pyi_content.append(f"class {enum_name}(Enum):")
        for value in enum_values:
            tmp_pyi_content.append(f"    {value}: {enum_name} = ...")
        tmp_pyi_content.append("")
        pyi_content.insert(0, "\n".join(tmp_pyi_content))

    return "\n".join(pyi_content)


def test():
    file = "modules/algorithm/include/rmvl/algorithm.hpp"
    with open(file, "r") as file:
        lines = file.readlines()
    print(generate_python_binding(lines))
    exit()


if __name__ == "__main__":
    # test()
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "input",
        type=str,
        help="Input file path containing C++ header file content",
    )
    parser.add_argument(
        "mode",
        type=str,
        help="'bind' for python binding code, 'pyi' for python interface file",
    )

    args = parser.parse_args()
    file = args.input
    with open(file, "r") as file:
        lines = file.readlines()

    if args.mode == "bind" or args.mode == "0":
        binding_code = generate_python_binding(lines)
    elif args.mode == "pyi" or args.mode == "1":
        binding_code = generate_pyi(lines)
    else:
        raise ValueError("\033[31;1mInvalid mode\033[0m. Use 'bind (0)' or 'pyi (1)'")
    print(binding_code)
