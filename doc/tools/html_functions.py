from __future__ import print_function
import sys

import logging
import os
import re
from pprint import pprint
import traceback

from doxygen_scan import Symbol

try:
    from bs4 import BeautifulSoup
    from bs4.element import Tag, NavigableString
except ImportError:
    raise ImportError(
        "Error: "
        "Install BeautifulSoup (bs4) for adding"
        " Python & Java signatures documentation"
    )


def load_html_file(file_dir: str) -> BeautifulSoup:
    """Uses BeautifulSoup to load an html"""
    with open(file_dir, "rb") as fp:
        data = fp.read()
    if os.name == "nt" or sys.version_info[0] == 3:
        data = data.decode(encoding="utf-8", errors="strict")
    data = re.sub(
        r"(\>)([ ]+)",
        lambda match: match.group(1) + ("!space!" * len(match.group(2))),
        data,
    )
    data = re.sub(
        r"([ ]+)(\<)",
        lambda match: ("!space!" * len(match.group(1))) + match.group(2),
        data,
    )
    if os.name == "nt" or sys.version_info[0] == 3:
        data = data.encode("utf-8", "ignore")
    soup = BeautifulSoup(data, "html.parser")
    return soup


def update_html(file: str, soup: BeautifulSoup) -> None:
    s = str(soup)
    s = s.replace("!space!", " ")
    if os.name == "nt" or sys.version_info[0] == 3:
        s = s.encode("utf-8", "ignore")
    with open(file, "wb") as f:
        f.write(s)


def insert_python_signatures(
    python_signatures: dict[str, list[dict[str, str]]],
    symbols_dict: dict[str, Symbol],
    filepath: str,
) -> None:
    soup = load_html_file(filepath)
    entries = soup.find_all(lambda tag: tag.name == "a" and tag.has_attr("id"))
    for e in entries:
        anchor = e["id"]
        if anchor in symbols_dict:
            s = symbols_dict[anchor]
            logging.info("Process: %r" % s)
            if s.type == "fn" or s.type == "method":
                process_fn(soup, e, python_signatures[s.cppname], s)
            elif s.type == "const":
                process_const(soup, e, python_signatures[s.cppname], s)
            else:
                logging.error("unsupported type: %s" % s)

    update_html(filepath, soup)


def process_fn(
    soup: BeautifulSoup,
    anchor: Tag,
    python_signature: list[dict[str, str]],
    symbol: Symbol,
) -> None:
    try:
        r = (
            anchor.find_next_sibling(class_="memitem")
            .find(class_="memproto")
            .find("table")
        )
        insert_python_fn_signature(soup, r, python_signature, symbol)
    except:
        logging.error("Can't process: %s" % symbol)
        traceback.print_exc()
        pprint(anchor)


def process_const(
    soup: BeautifulSoup,
    anchor: Tag,
    python_signature: list[dict[str, str]],
    symbol: Symbol,
):
    try:
        # pprint(anchor.parent)
        description = append(
            soup.new_tag("div", **{"class": ["python_language"]}),
            "Python: " + python_signature[0]["name"],
        )
        old = anchor.find_next_sibling("div", class_="python_language")
        if old is None:
            anchor.parent.append(description)
        else:
            old.replace_with(description)
        # pprint(anchor.parent)
    except:
        logging.error("Can't process: %s" % symbol)
        traceback.print_exc()
        pprint(anchor)


def insert_python_fn_signature(
    soup: BeautifulSoup, table: Tag, variants: list[dict[str, str]], symbol: Symbol
):
    description = create_python_fn_description(soup, variants)
    description["class"] = "python_language"
    soup = insert_or_replace(table, description, "table", "python_language")
    return soup


def create_python_fn_description(soup: BeautifulSoup, variants: list[dict[str, str]]):
    language = "Python:"
    table = soup.new_tag("table")
    # heading_row = soup.new_tag('th')
    table.append(
        append(
            soup.new_tag("tr"),
            append(soup.new_tag("th", colspan=999, style="text-align:left"), language),
        )
    )
    for v in variants:
        # logging.debug(v)
        add_signature_to_table(soup, table, v)
    # print(table)
    return table


def add_signature_to_table(soup: BeautifulSoup, table: Tag, signature: dict[str, str]):
    """Add a signature to an html table"""
    row = soup.new_tag("tr")
    row.append(soup.new_tag("td", style="width: 15px;"))
    row.append(append(soup.new_tag("td"), signature["name"] + "("))
    row.append(append(soup.new_tag("td", **{"class": "paramname"}), signature["arg"]))
    row.append(append(soup.new_tag("td"), ") -> "))
    row.append(append(soup.new_tag("td"), signature["ret"]))
    table.append(row)


def append(target: Tag, obj: str) -> Tag:
    target.append(obj)
    return target


def insert_or_replace(element_before: Tag, new_element: Tag, tag: str, tag_class: str):
    old = element_before.find_next_sibling(tag, class_=tag_class)
    if old is None:
        element_before.insert_after(new_element)
    else:
        old.replace_with(new_element)
