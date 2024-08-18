import traceback
from xml.etree.ElementTree import Element

class Symbol(object):
    def __init__(self, anchor: str, type: str, cppname: str) -> None:
        self.anchor = anchor
        self.type = type
        self.cppname = cppname

    def __repr__(self):
        return '%s:%s@%s' % (self.type, self.cppname, self.anchor)

def add_to_file(files_dict: dict[str, list[Symbol]], file: str, anchor: Symbol) -> None:
    anchors = files_dict.setdefault(file, [])
    anchors.append(anchor)


def scan_namespace_constants(ns: Element, ns_name: str, files_dict: dict[str, list[Symbol]]) -> None:
    # enum
    constants = ns.findall("./member[@kind='enumvalue']")
    for c in constants:
        c_name = c.find("./name").text
        name = ns_name + '::' + c_name
        file = c.find("./anchorfile").text
        anchor = c.find("./anchor").text
        #print('    CONST: {} => {}#{}'.format(name, file, anchor))
        add_to_file(files_dict, file, Symbol(anchor, "const", name))
    # enum class
    ec_constants = ns.findall("./member[@kind='enumeration']")
    for ec in ec_constants:
        c_name = ec.find("./name").text
        name = ns_name + '::' + c_name
        enumvalues = ec.findall("./enumvalue")
        for ev in enumvalues:
            ev_name = ev.text
            ev_file = ev.get("file")
            ev_anchor = ev.get("anchor")
            ev_full_name = name + '::' + ev_name
            #print('    CONST: {} => {}#{}'.format(ev_full_name, ev_file, ev_anchor))
            add_to_file(files_dict, ev_file, Symbol(ev_anchor, "const", ev_full_name))

def scan_namespace_functions(ns: Element, ns_name: str, files_dict: dict[str, list[Symbol]]) -> None:
    functions = ns.findall("./member[@kind='function']")
    for f in functions:
        f_name = f.find("./name").text
        name = ns_name + '::' + f_name
        file = f.find("./anchorfile").text
        anchor = f.find("./anchor").text
        #print('    FN: {} => {}#{}'.format(name, file, anchor))
        add_to_file(files_dict, file, Symbol(anchor, "fn", name))

def scan_class_methods(c: Element, c_name: str, files_dict: dict[str, list[Symbol]]) -> None:
    methods = c.findall("./member[@kind='function']")
    for m in methods:
        m_name = m.find("./name").text
        name = c_name + '::' + m_name
        file = m.find("./anchorfile").text
        anchor = m.find("./anchor").text
        #print('    Method: {} => {}#{}'.format(name, file, anchor))
        add_to_file(files_dict, file, Symbol(anchor, "method", name))
