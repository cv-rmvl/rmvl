"""
C++ to Python binding code generator
"""

import json
import os
import re
from typing import Union, List, Tuple
import argparse
from collections import defaultdict


def re_match(mode: str, line: str) -> Union[re.Match, None]:
    """
    ### Regex match a line in C++ header file
    #### Parameters
    `mode`: `str` ─ Matching mode, optional values are:
    - `normal_class`: Match normal class definition
    - `aggregate_class`: Match aggregate class definition
    - `normal_aggregate_class`: Match normal and aggregate class definition
    - `inherited_class`: Match inherited class definition
    - `global_function`: Match global function definition
    - `using`: Match using directive
    - `constructor`: Match class constructor
    - `method`: Match class method definition
    - `static_method`: Match class static method definition
    - `const_method`: Match class const method definition
    - `convert_method`: Match class user-defined conversion function definition
    - `member`: Match class member variable definition
    - `variable`: Match global variable definition

    `line`: `str` ─ String containing C++ header file line
    #### Returns
    `Union[re.Match, None]`: Match result, return `None` if no match
    """
    if mode == "normal_class":
        return re.search(r"class\s+RMVL_EXPORTS_W\s+(\w+)\s*(?:final)?$", line)
    elif mode == "aggregate_class":
        return re.search(r"struct\s+RMVL_EXPORTS_W_AG\s+(\w+)\s*(?:final)?$", line)
    elif mode == "normal_aggregate_class":
        return re.search(
            r"(class|struct)\s+RMVL_EXPORTS_(?:W|W_AG)\s+(\w+)\s*(?:final)?$", line
        )
    elif mode == "inherited_class":
        return re.search(
            r"class\s+RMVL_EXPORTS_W\s+(\w+)\s*(?:final)?:\s+(public)?\s+(\w+)\s*$",
            line,
        )
    elif mode == "global_function":
        return re.search(
            r"RMVL_EXPORTS_W\s*([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)",
            line,
        )
    elif mode == "using":
        return re.search(r"using\s+(\w+)\s*=\s*(.+);", line)
    # RMVL_W_SUBST
    elif mode == "subst":
        return re.search(r"RMVL_W_SUBST\(\"(\w+)\"\)", line)
    # RMVL_W
    elif mode == "constructor":
        return re.search(
            r"RMVL_W\s+(?:constexpr\s+)?(\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)", line
        )
    elif mode == "method":
        return re.search(
            r"RMVL_W\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(?:&)?(operator\(\)|operator\[\]|operator==|operator!=|operator\+|operator-|\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)",
            line,
        )
    elif mode == "static_method":
        return re.search(
            r"RMVL_W\s+static\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)",
            line,
        )
    elif mode == "const_method":
        return re.search(
            r"RMVL_W\s+([\w<>:,\s&]+(?:<[^<>]*>)*)\s+(?:&)?(operator\(\)|operator\[\]|operator==|operator!=|operator\+|operator-|\w+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*const",
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


def split_parameters(params: str) -> Tuple[List[str], List[str], List[str]]:
    """
    ### split parameters and default values from a string containing parameters
    #### Parameters
    `params`: `str` ─ string containing parameters and optional default values
    #### Returns
    `Tuple[List[str], List[str], List[str]]`: a Tuple containing three Lists:
    - List of parameter types
    - List of parameter names
    - List of default values, if a parameter has no default value, the corresponding element will be `None`
    #### Example
    >>> split_parameters('int a, const float b=3.14, string c')
    >>> # (['int', 'const float', 'string'], ['a', 'b', 'c'], [None, '3.14', None])
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
    "auto": "Any",
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
    # cv types
    "Mat": "np.ndarray",
    "Matx": "Tuple",
    "Point": "Tuple",
    "Vec": "Tuple",
}

def remove_cvref(s: str) -> str:
    """
    ### Remove `const`, `&`, `*` from a C++ type string
    #### Parameters
    `s` ─ C++ type string
    #### Returns
    `(str)`: string without cvref
    """
    s = re.sub(r"const\s+|&", "", s)
    return s

def remove_t(s: str) -> str:
    """
    ### Remove `static`, `extern`, `const`, `&`, `*`, `namespace`, `inline`, `constexpr` from a C++ type string
    #### Parameters
    `s` ─ C++ type string
    #### Returns
    `(str)`: string without modifiers
    """
    # remove static, extern
    s = re.sub(r"static\s+|extern\s+", "", s)
    # remove cvref
    s = remove_cvref(s)
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
    >>> type_convert('vector<int>')
    returns
    >>> 'List[int]'
    """

    # Remove `const`, `&`, `*`, `namespace`, `inline`, `constexpr`
    cpp_type = remove_t(cpp_type).strip()

    def convert_vector(match: re.Match) -> str:
        """
        Convert `vector<...>` to `List[...]`
        """
        inner : str = match.group(1)
        converted = type_convert(inner)
        return f"List[{converted}]"

    def convert_pair_tuple(match: re.Match) -> str:
        """
        Convert `pair<...>`, `Tuple<...>` to `Tuple[...]`
        """
        inner : str = match.group(2)
        converted = ", ".join(type_convert(x.strip()) for x in inner.split(","))
        return f"tuple[{converted}]"

    def convert_unordered_map(match: re.Match) -> str:
        """
        Convert `unordered_map<...>` to `dict[...]`
        """
        inner : str = match.group(1)
        key, value = map(type_convert, (x.strip() for x in inner.split(",")))
        return f"dict[{key}, {value}]"

    def convert_function(match: re.Match) -> str:
        """
        Convert `function<return_type(arg_types)>` to `Callable[[arg_types], return_type]`
        """
        return_type = type_convert(match.group(1))
        arg_types = ", ".join(
            type_convert(x.strip()) for x in match.group(2).split(",")
        )
        return f"Callable[[{arg_types}], {return_type}]"

    def convert_array(match: re.Match) -> str:
        """
        Convert `array<..., size>` to `List[...]`
        """
        inner = type_convert(match.group(1))
        return f"List[{inner}]"

    def convert_identifier(match: re.Match) -> str:
        """
        Convert identifier to Python type
        """
        return types.get(match.group(0), match.group(0))

    def convert_matx(match: re.Match) -> str:
        """
        Convert `Matx<...>` to `Tuple`
        """
        tp, m, n = match.group(1), match.group(2), match.group(3)
        inner = type_convert(tp)
        return f"Tuple[{', '.join([inner] * int(m) * int(n))}]"

    def convert_vec(match: re.Match) -> str:
        """
        Convert `Vec<...>` to `Tuple`
        """
        tp, n = match.group(1), match.group(2)
        inner = type_convert(tp)
        return f"Tuple[{', '.join([inner] * int(n))}]"

    # bitset<...> -> int
    cpp_type = re.sub(r"^bitset<\d+>", "int", cpp_type)
    # vector<...> -> List[...]
    cpp_type = re.sub(r"^vector<(.+)>$", convert_vector, cpp_type)
    # array<..., size> -> List[...]
    cpp_type = re.sub(r"^array<([^,]+),\s*\d+>$", convert_array, cpp_type)
    # pair<...>, tuple<...> -> tuple[...]
    cpp_type = re.sub(r"^(pair|tuple)<(.+)>$", convert_pair_tuple, cpp_type)
    # unordered_map<...> -> dict[...]
    cpp_type = re.sub(r"^unordered_map<(.+)>$", convert_unordered_map, cpp_type)
    # function<return_type(arg_types)> -> callable[[arg_types], return_type]
    cpp_type = re.sub(r"^function<([^(]+)\(([^)]*)\)>$", convert_function, cpp_type)
    # convert cv types
    cpp_type = re.sub(r"^Matx<([^,]+),\s*(\d+),\s*(\d+)>$", convert_matx, cpp_type)
    cpp_type = re.sub(r"^Point$", "Tuple[int, int]", cpp_type)
    cpp_type = re.sub(r"^Point2[fd]$", "Tuple[float, float]", cpp_type)
    cpp_type = re.sub(r"^Point3i$", "Tuple[int, int, int]", cpp_type)
    cpp_type = re.sub(r"^Point3[fd]$", "Tuple[float, float, float]", cpp_type)
    cpp_type = re.sub(r"^Vec<([^,]+),\s*(\d+)>$", convert_vec, cpp_type)
    # convert identifier
    cpp_type = re.sub(r"^\b\w+\b$", convert_identifier, cpp_type)

    return cpp_type


def generate_python_binding(lines: List[str], misc: Union[str, None]) -> str:
    """
    ### Generate py binding code from C++ header file
    #### Parameters
    - `lines`: List of strings containing C++ header file lines
    #### Returns
    `(str)`: generated py binding code
    """

    binding_code = []
    # To store functions for overload detection
    class_content = []
    cur_class = None
    functions = defaultdict(list)
    convert_content = []
    # To store enum class definitions
    enum_classes = []
    enums = list[str]()
    in_enum = False
    cur_enum_class = None
    cur_enum = None

    for line in lines:
        line = line.strip()

        if line.startswith("//") or not line:
            continue
        # Enum class and enum detection
        elif line.startswith("enum class") or (in_enum and line.startswith("{")):
            in_enum = True
            mch = re.search(r"enum class\s+(\w+)", line)
            if mch:
                cur_enum_class = [mch.group(1), []]
        elif line.startswith("enum ") or (in_enum and line.startswith("{")):
            in_enum = True
            mch = re.search(r"enum\s+(\w+)", line)
            if mch:
                cur_enum = mch.group(1)
        elif in_enum and "}" in line:
            in_enum = False
            if cur_enum_class:
                enum_classes.append(cur_enum_class)
                types[cur_enum_class[0]] = cur_enum_class[0]
                cur_enum_class = None
            elif cur_enum:
                types[cur_enum] = cur_enum
                cur_enum = None
        elif in_enum and (cur_enum_class or cur_enum):
            match = re.match(r"(\w+)", line)
            if match:
                value = match.group(1).strip()
                if cur_enum_class:
                    cur_enum_class[1].append(value)
                elif cur_enum:
                    enums.append(value)
                continue
        # Class or function with 'RMVL_EXPORTS_W' macro
        elif "RMVL_EXPORTS_W" in line:
            # Start of a new class
            if line.startswith("class") or line.startswith("struct"):
                # Normal class
                mch = re_match("normal_class", line)
                if mch:
                    cur_class = mch.group(1)
                    class_content.append(
                        f'    py::class_<{cur_class}>(m, "{cur_class}", py::dynamic_attr())'
                    )
                # Aggregate class
                mch = re_match("aggregate_class", line)
                if mch:
                    cur_class = mch.group(1)
                    class_content.append(
                        f'    py::class_<{cur_class}>(m, "{cur_class}", py::dynamic_attr())'
                    )
                    class_content.append("        .def(py::init<>())")
                # Inherited class
                mch = re_match("inherited_class", line)
                if mch:
                    cur_class = mch.group(1)
                    class_content.append(
                        f'    py::class_<{cur_class}, {mch.group(3)}>(m, "{cur_class}")'
                    )

            # Global normal function
            else:
                mch = re_match("global_function", line)
                if mch:
                    return_type, func_name, params = mch.groups()
                    type_list, id_list, default_list = split_parameters(params)
                    functions[func_name].append((type_list, id_list, default_list))
        # Class method with 'RMVL_W_SUBST' macro
        elif line.startswith("RMVL_W_SUBST"):
            mch = re_match("subst", line)
            if mch and misc:
                alias_str = mch.group(1)    
                with open(misc, "r") as f:
                    json_content: dict = json.load(f)
                    if cur_class:
                        if alias_str not in json_content:
                            class_content.append(
                                f"        // No binding information for \"{alias_str}\""
                            )
                        else:
                            bind_alias = json_content[alias_str].get("bind", [])
                            for bind_alias_each in bind_alias:
                                class_content.append(f"        {bind_alias_each}")
                    else:
                        if alias_str not in json_content:
                            binding_code.append(
                                f"    // No binding information for \"{alias_str}\""
                            )
                        else:
                            bind_alias = json_content[alias_str].get("bind", [])
                            for bind_alias_each in bind_alias:
                                binding_code.append(f"    {bind_alias_each}")
                    
        # Class method with 'RMVL_W[_xxx]' macro
        elif line.startswith("RMVL_W") and cur_class:
            # Constructor
            mch = re_match("constructor", line)
            if mch:
                name, params = mch.groups()
                type_list, id_list, default_list = split_parameters(params)
                class_content.append(
                    f'        .def(py::init<{", ".join(type_list)}>(),'
                )
                for type, id, default in zip(type_list, id_list, default_list):
                    if default == "{}":
                        class_content.append(
                            f'             "{id}"_a = {remove_cvref(type)}{{}},'
                        )
                    elif default:
                        class_content.append(f'             "{id}"_a = {default},')
                    else:
                        class_content.append(f'             "{id}"_a,')
                class_content[-1] = class_content[-1][:-1] + ")"
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
                            f'             "{id}"_a = {remove_cvref(type)}{{}},'
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
                            f'             "{id}"_a = {remove_cvref(type)}{{}},'
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
                # operator[] overload
                elif method_name == "operator[]":
                    class_content.append(
                        f'        .def("__setitem__", &{cur_class}::operator[],'
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
                            f'             "{id}"_a = {remove_cvref(type)}{{}},'
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
                binding_code.append(
                    f'    m.attr("{var_name}") = py::cast({var_name});'
                )

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
                    binding_code.append(f'          "{id}"_a = {remove_cvref(type)}{{}},')
                elif default:
                    binding_code.append(f'          "{id}"_a = {default},')
                else:
                    binding_code.append(f'          "{id}"_a,')
            binding_code[-1] = binding_code[-1][:-1] + ");"
        else:
            for type_list, id_list, default_list in overloads:
                binding_code.append(
                    f'    m.def("{func_name}", py::overload_cast<{", ".join(type_list)}>(&{func_name}),'
                )
                for type, id, default in zip(type_list, id_list, default_list):
                    if default == "{}":
                        binding_code.append(
                            f'          "{id}"_a = {remove_cvref(type)}{{}},'
                        )
                    elif default:
                        binding_code.append(f'          "{id}"_a = {default},')
                    else:
                        binding_code.append(f'          "{id}"_a,')
                binding_code[-1] = binding_code[-1][:-1] + ");"

    # Process enums
    tmp_bd_code = []
    for enum_name, enum_values in enum_classes:
        tmp_bd_code.append(f'    py::enum_<{enum_name}>(m, "{enum_name}")')
        for value in enum_values:
            tmp_bd_code.append(f'        .value("{value}", {enum_name}::{value})')
        tmp_bd_code.append("        .export_values();")
    for enum in enums:
        tmp_bd_code.append(f'    m.attr("{enum}") = py::cast<int>({enum});')
    binding_code.insert(0, "\n".join(tmp_bd_code))
    return "\n".join(binding_code)


def generate_pyi(lines: list[str], doc: str | None, misc: str | None) -> str:
    """
    ### Generate .pyi file content and documents configurations from C++ header file
    #### Parameters
    - `lines`: list of strings containing C++ header file lines
    - `doc`: path of the pydoc cfg file
    #### Returns
    `(str)`: generated .pyi file content
    """

    pyi_content = []
    # To store functions for overload detection
    class_content = []
    para_class_fordoc = []
    cur_class = None
    functions = defaultdict(list)
    # To store enum class definitions
    enum_classes = []
    enums = []
    in_enum = False
    cur_enum_class = None
    cur_enum = None
    # To store typings
    typings = []

    for line in lines:
        line = line.strip()

        if line.startswith("//") or not line:
            continue

        # Using directive
        if line.startswith("using"):
            mch = re_match("using", line)
            if mch:
                alias, target = mch.groups()
                typings.append(f"{alias} = {type_convert(target)}")
                continue
        # Enum class and enum detection
        elif line.startswith("enum class") or (in_enum and line.startswith("{")):
            in_enum = True
            mch = re.search(r"enum class\s+(\w+)", line)
            if mch:
                cur_enum_class = [mch.group(1), []]
        elif line.startswith("enum ") or (in_enum and line.startswith("{")):
            in_enum = True
            mch = re.search(r"enum\s+(\w+)", line)
            if mch:
                cur_enum = mch.group(1)
        elif in_enum and "}" in line:
            in_enum = False
            if cur_enum_class:
                enum_classes.append(cur_enum_class)
                types[cur_enum_class[0]] = cur_enum_class[0]
                cur_enum_class = None
            elif cur_enum:
                types[cur_enum] = cur_enum
                cur_enum = None
        elif in_enum and (cur_enum_class or cur_enum):
            match = re.match(r"(\w+)", line)
            if match:
                value = match.group(1).strip()
                if cur_enum_class:
                    cur_enum_class[1].append(value)
                elif cur_enum:
                    enums.append(value)
                continue

        # Class or function with 'RMVL_EXPORTS_W' macro
        elif "RMVL_EXPORTS_W" in line:
            # Class definition
            if line.startswith("class") or line.startswith("struct"):
                # Normal class or aggregate class
                mch = re_match("normal_aggregate_class", line)
                if mch:
                    cur_class = mch.group(2)
                    class_content.append(f"class {cur_class}:")
                    if cur_class.endswith("Param"):
                        para_class_fordoc.append(cur_class)
                # Inherited class
                mch = re_match("inherited_class", line)
                if mch:
                    cur_class = mch.group(1)
                    class_content.append(f"class {cur_class}({mch.group(3)}):")
            # Global function definition
            else:
                mch = re_match("global_function", line)
                if mch:
                    return_type, func_name, params = mch.groups()
                    type_list, id_list, default_list = split_parameters(params)
                    functions[func_name].append(
                        (return_type, type_list, id_list, default_list)
                    )
        # Class method with 'RMVL_W_SUBST' macro
        elif line.startswith("RMVL_W_SUBST"):
            mch = re_match("subst", line)
            if mch and misc:
                alias_str = mch.group(1)
                with open(misc, "r") as f:
                    json_content: dict = json.load(f)
                    if cur_class:
                        if alias_str not in json_content:
                            class_content.append(
                                f"    # No binding information for \"{alias_str}\""
                            )
                        else:
                            bind_alias = json_content[alias_str].get("pyi", [])
                            for bind_alias_each in bind_alias:
                                class_content.append(f"    {bind_alias_each}")
                    else:
                        if alias_str not in json_content:
                            pyi_content.append(
                                f"# No binding information for \"{alias_str}\""
                            )
                        else:
                            bind_alias = json_content[alias_str].get("pyi", [])
                            for bind_alias_each in bind_alias:
                                pyi_content.append(f"{bind_alias_each}")
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
                    f"    @overload\n    def __init__(self, {param_str}) -> {name}: ..."
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
                    f"    @overload\n    def {method_name}(self, {param_str}) -> {type_convert(return_type)}: ..."
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

    # Process typings
    for typing in typings:
        pyi_content.append(typing)

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
                pyi_content.append(f"@overload")
                pyi_content.append(
                    f"def {func_name}({param_str}) -> {type_convert(return_type)}: ..."
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
                f"def {func_name}({param_str}) -> {type_convert(return_type)}: ..."
            )

    # Process enum classes and enums
    for enum_name, enum_values in enum_classes:
        tmp_pyi_content = []
        tmp_pyi_content.append(f"class {enum_name}(Enum):")
        for value in enum_values:
            tmp_pyi_content.append(f"    {value}: {enum_name} = ...")
        tmp_pyi_content.append("")
        pyi_content.insert(0, "\n".join(tmp_pyi_content))
    tmp_pyi_content = []
    for enum in enums:
        tmp_pyi_content.append(f"{enum}: int = ...")
    tmp_pyi_content.append("")
    pyi_content.insert(0, "\n".join(tmp_pyi_content))

    # Generate doc cfg
    if doc:
        pyrmvl_cst_path = f"{doc}/pyrmvl_cst.cfg"
        pyrmvl_fns_path = f"{doc}/pyrmvl_fns.cfg"
        if os.path.exists(pyrmvl_cst_path):
            with open(pyrmvl_cst_path, "a") as file:
                for enum_name, enum_values in enum_classes:
                    for idx, value in enumerate(enum_values):
                        file.write(f"rm::{enum_name} {value} {idx}\n")
                for idx, enum in enumerate(enums):
                    file.write(f"rm {enum} {idx}\n")
        if os.path.exists(pyrmvl_fns_path):
            with open(pyrmvl_fns_path, "a") as file:
                for para_class in para_class_fordoc:
                    file.write(f"rm::para::{para_class} read path [[success_?]]\n")
                    file.write(f"rm::para::{para_class} write path [[success_?]]\n")
    return "\n".join(pyi_content)


def test():
    file = "modules/algorithm/include/rmvl/algorithm/numcal.hpp"
    with open(file, "r") as file:
        lines = file.readlines()
    print(generate_pyi(lines))
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
        help="'bind' for python binding code, 'pyi' for python interface file and doc config file",
    )
    parser.add_argument(
        "--doc", type=str, help="path of the documents configuration file"
    )
    parser.add_argument(
        "--misc", type=str, help="path of the miscellaneous configuration file"
    )

    args = parser.parse_args()
    file = args.input
    with open(file, "r") as file:
        lines = file.readlines()

    if args.mode == "bind":
        binding_code = generate_python_binding(lines, args.misc)
    elif args.mode == "pyi":
        binding_code = generate_pyi(lines, args.doc, args.misc)
    else:
        raise ValueError("\033[31;1mInvalid mode\033[0m. Use 'bind' or 'pyi'")
    print(binding_code)
