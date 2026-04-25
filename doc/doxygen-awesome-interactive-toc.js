// SPDX-License-Identifier: MIT
/**

Doxygen Awesome
https://github.com/jothepro/doxygen-awesome-css

Copyright (c) 2022 - 2025 jothepro

*/

class DoxygenAwesomeInteractiveToc {
    static topOffset = 38
    static scrollTopThreshold = 220
    static hideMobileMenu = true
    static headers = []
    static scrollTopButton = null
    static throttledUpdate = null

    static init() {
        window.addEventListener("load", () => {
            DoxygenAwesomeInteractiveToc.headers = []

            DoxygenAwesomeInteractiveToc.initScrollTopButton()
            DoxygenAwesomeInteractiveToc.throttledUpdate ??= this.throttle(DoxygenAwesomeInteractiveToc.handleViewportChange, 100)
            document.getElementById("doc-content")?.addEventListener("scroll", DoxygenAwesomeInteractiveToc.throttledUpdate, { passive: true })
            window.addEventListener("scroll", DoxygenAwesomeInteractiveToc.throttledUpdate, { passive: true })
            window.addEventListener("resize", DoxygenAwesomeInteractiveToc.throttledUpdate, { passive: true })

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
            }

            DoxygenAwesomeInteractiveToc.handleViewportChange()
        })
    }

    static initScrollTopButton() {
        if (DoxygenAwesomeInteractiveToc.scrollTopButton) {
            return
        }

        const button = document.createElement("button")
        button.type = "button"
        button.classList.add("scroll-top-button")
        button.setAttribute("aria-label", "返回顶部")
        button.setAttribute("title", "返回顶部")
        const svg=`<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 640 640"><!--!Font Awesome Free v7.2.0 by @fontawesome - https://fontawesome.com License - https://fontawesome.com/license/free Copyright 2026 Fonticons, Inc.--><path fill="currentColor" d="M342.6 73.4C330.1 60.9 309.8 60.9 297.3 73.4L137.3 233.4C124.8 245.9 124.8 266.2 137.3 278.7C149.8 291.2 170.1 291.2 182.6 278.7L288 173.3L288 544C288 561.7 302.3 576 320 576C337.7 576 352 561.7 352 544L352 173.3L457.4 278.7C469.9 291.2 490.2 291.2 502.7 278.7C515.2 266.2 515.2 245.9 502.7 233.4L342.7 73.4z"/></svg>`
        button.innerHTML = `<span class="scroll-top-button-icon" aria-hidden="true">${svg}</span>`
        button.addEventListener("click", (event) => {
            event.preventDefault()
            DoxygenAwesomeInteractiveToc.scrollToTop()
        })

        document.body.appendChild(button)
        DoxygenAwesomeInteractiveToc.scrollTopButton = button
    }

    static handleViewportChange() {
        DoxygenAwesomeInteractiveToc.update()
        DoxygenAwesomeInteractiveToc.updateScrollTopButtonVisibility()
    }

    static getScrollTop() {
        return Math.max(
            window.pageYOffset || document.documentElement.scrollTop || 0,
            document.getElementById("doc-content")?.scrollTop || 0
        )
    }

    static updateScrollTopButtonVisibility() {
        const button = DoxygenAwesomeInteractiveToc.scrollTopButton
        if (!button) {
            return
        }

        if (DoxygenAwesomeInteractiveToc.getScrollTop() > DoxygenAwesomeInteractiveToc.scrollTopThreshold) {
            button.classList.add("visible")
        } else {
            button.classList.remove("visible")
        }
    }

    static scrollToTop() {
        window.scrollTo({ top: 0, behavior: "smooth" })
        document.getElementById("doc-content")?.scrollTo({ top: 0, behavior: "smooth" })
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
            if (now - lastCall < delay) return;
            lastCall = now;
            return setTimeout(() => { func(...args) }, delay);
        };
    }
}