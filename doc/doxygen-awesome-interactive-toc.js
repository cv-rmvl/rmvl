// SPDX-License-Identifier: MIT
/**

Doxygen Awesome
https://github.com/jothepro/doxygen-awesome-css

Copyright (c) 2022 - 2025 jothepro

*/

class DoxygenAwesomeInteractiveToc {
    static topOffset = 38
    static hideMobileMenu = true
    static headers = []
    static throttledUpdate = null

    static init() {
        window.addEventListener("load", () => {
            DoxygenAwesomeInteractiveToc.headers = []
            let toc = document.querySelector(".contents > .toc")
            if (toc) {
                toc.classList.add("interactive")
                if (!DoxygenAwesomeInteractiveToc.hideMobileMenu) {
                    toc.classList.add("open")
                }
                document.querySelector(".contents > .toc > h3")?.addEventListener("click", () => {
                    if (toc.classList.contains("open")) {
                        toc.classList.remove("open")
                    } else {
                        toc.classList.add("open")
                    }
                })

                document.querySelectorAll(".contents > .toc > ul a").forEach((node) => {
                    let href = node.getAttribute("href")
                    if (!href || !href.startsWith("#")) {
                        return
                    }
                    let id = href.substring(1)
                    let headerNode = document.getElementById(id)
                    if (!headerNode) {
                        return
                    }
                    DoxygenAwesomeInteractiveToc.headers.push({
                        node: node,
                        headerNode: headerNode
                    })
                })

                // Doxygen scroll container differs by layout (treeview on/off), so listen to both.
                DoxygenAwesomeInteractiveToc.throttledUpdate ??= this.throttle(DoxygenAwesomeInteractiveToc.update, 100)
                document.getElementById("doc-content")?.addEventListener("scroll", DoxygenAwesomeInteractiveToc.throttledUpdate, { passive: true })
                window.addEventListener("scroll", DoxygenAwesomeInteractiveToc.throttledUpdate, { passive: true })
                window.addEventListener("resize", DoxygenAwesomeInteractiveToc.throttledUpdate, { passive: true })

                DoxygenAwesomeInteractiveToc.update()
            }
        })
    }

    static update() {
        let active = DoxygenAwesomeInteractiveToc.headers[0]?.node
        DoxygenAwesomeInteractiveToc.headers.forEach((header) => {
            let position = header.headerNode.getBoundingClientRect().top
            header.node.classList.remove("active")
            header.node.classList.remove("aboveActive")
            if (position < DoxygenAwesomeInteractiveToc.topOffset) {
                active = header.node
                active?.classList.add("aboveActive")
            }
        })
        active?.classList.add("active")
        active?.classList.remove("aboveActive")
    }

    static throttle(func, delay) {
        let lastCall = 0;
        return function (...args) {
            const now = new Date().getTime();
            if (now - lastCall < delay) {
                return;
            }
            lastCall = now;
            return setTimeout(() => { func(...args) }, delay);
        };
    }
}