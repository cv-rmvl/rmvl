// SPDX-License-Identifier: MIT
/**

Doxygen Awesome
https://github.com/jothepro/doxygen-awesome-css

Copyright (c) 2022 - 2025 jothepro

*/

class DoxygenAwesomeInteractiveToc {
    static topOffset = 38
    static scrollTopThreshold = 220
    static scrollTopIdleDelay = 1000
    static hideMobileMenu = true
    static headers = []
    static scrollTopButton = null
    static scrollTopIdleTimer = null
    static viewportUpdatePending = false

    static init() {
        window.addEventListener("load", () => {
            DoxygenAwesomeInteractiveToc.headers = []

            DoxygenAwesomeInteractiveToc.initScrollTopButton()
            document.getElementById("doc-content")?.addEventListener("scroll", DoxygenAwesomeInteractiveToc.requestViewportUpdate, { passive: true })
            window.addEventListener("scroll", DoxygenAwesomeInteractiveToc.requestViewportUpdate, { passive: true })
            window.addEventListener("resize", DoxygenAwesomeInteractiveToc.requestViewportUpdate, { passive: true })

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
        button.innerHTML = `
            <svg class="scroll-top-button-progress" viewBox="0 0 64 64" aria-hidden="true">
                <circle class="scroll-top-button-progress-track" cx="32" cy="32" r="29.5"></circle>
                <circle class="scroll-top-button-progress-value" cx="32" cy="32" r="29.5" pathLength="100"></circle>
            </svg>
            <span class="scroll-top-button-percent">0%</span>
            <span class="scroll-top-button-icon" aria-hidden="true">
                <svg xmlns="http://www.w3.org/2000/svg" height="24" width="24" viewBox="0 0 640 640">
                    <path d="M184 24C166.3 24 152 38.3 152 56C152 73.7 166.3 88 184 88L456 88C473.7 88 488 73.7 488 56C488 38.3 473.7 24 456 24L184 24zM342.6 137.4C330.1 124.9 309.8 124.9 297.3 137.4L137.3 297.4C124.8 309.9 124.8 330.2 137.3 342.7C149.8 355.2 170.1 355.2 182.6 342.7L288 237.3L288 576C288 593.7 302.3 608 320 608C337.7 608 352 593.7 352 576L352 237.3L457.4 342.7C469.9 355.2 490.2 355.2 502.7 342.7C515.2 330.2 515.2 309.9 502.7 297.4L342.7 137.4z"/>
                </svg>
            </span>
        `
        button.addEventListener("click", (event) => {
            event.preventDefault()
            DoxygenAwesomeInteractiveToc.scrollToTop()
        })

        document.body.appendChild(button)
        DoxygenAwesomeInteractiveToc.scrollTopButton = button
        DoxygenAwesomeInteractiveToc.updateScrollTopButton()
    }

    static handleViewportChange() {
        DoxygenAwesomeInteractiveToc.update()
        DoxygenAwesomeInteractiveToc.updateScrollTopButton()
    }

    static requestViewportUpdate() {
        if (DoxygenAwesomeInteractiveToc.viewportUpdatePending) {
            return
        }

        DoxygenAwesomeInteractiveToc.viewportUpdatePending = true
        window.requestAnimationFrame(() => {
            DoxygenAwesomeInteractiveToc.viewportUpdatePending = false
            DoxygenAwesomeInteractiveToc.handleViewportChange()
        })
    }

    static getScrollTop() {
        return Math.max(
            window.pageYOffset || document.documentElement.scrollTop || 0,
            document.getElementById("doc-content")?.scrollTop || 0
        )
    }

    static getScrollProgress() {
        const documentElement = document.documentElement
        const body = document.body
        const windowScrollTop = window.pageYOffset || documentElement.scrollTop || body.scrollTop || 0
        const windowScrollableHeight = Math.max(
            documentElement.scrollHeight,
            body.scrollHeight
        ) - window.innerHeight
        const docContent = document.getElementById("doc-content")
        const docContentScrollableHeight = docContent ? docContent.scrollHeight - docContent.clientHeight : 0
        const windowProgress = windowScrollableHeight > 0 ? windowScrollTop / windowScrollableHeight : 0
        const docContentProgress = docContentScrollableHeight > 0 ? docContent.scrollTop / docContentScrollableHeight : 0

        return Math.min(1, Math.max(0, windowProgress, docContentProgress))
    }

    static updateScrollTopButton() {
        const button = DoxygenAwesomeInteractiveToc.scrollTopButton
        if (!button) {
            return
        }

        const progress = DoxygenAwesomeInteractiveToc.getScrollProgress()
        const progressPercent = progress * 100
        const percent = Math.round(progressPercent)
        const progressValue = button.querySelector(".scroll-top-button-progress-value")
        const progressText = button.querySelector(".scroll-top-button-percent")
        const scrollTop = DoxygenAwesomeInteractiveToc.getScrollTop()

        if (progressValue) {
            progressValue.style.strokeDashoffset = 100 - progressPercent
        }
        if (progressText) {
            progressText.textContent = `${percent}%`
        }

        button.classList.remove("idle")

        if (scrollTop > 0 || button.classList.contains("visible")) {
            button.classList.add("visible")
            DoxygenAwesomeInteractiveToc.updateScrollTopButtonAfterIdle()
        } else {
            button.classList.remove("visible")
            DoxygenAwesomeInteractiveToc.clearScrollTopIdleTimer()
        }
    }

    static updateScrollTopButtonAfterIdle() {
        DoxygenAwesomeInteractiveToc.clearScrollTopIdleTimer()
        DoxygenAwesomeInteractiveToc.scrollTopIdleTimer = window.setTimeout(() => {
            const button = DoxygenAwesomeInteractiveToc.scrollTopButton
            if (!button) {
                return
            }

            if (DoxygenAwesomeInteractiveToc.getScrollTop() <= DoxygenAwesomeInteractiveToc.scrollTopThreshold) {
                button.classList.remove("visible")
                button.classList.remove("idle")
            } else {
                button.classList.add("visible")
                button.classList.add("idle")
            }
        }, DoxygenAwesomeInteractiveToc.scrollTopIdleDelay)
    }

    static clearScrollTopIdleTimer() {
        if (!DoxygenAwesomeInteractiveToc.scrollTopIdleTimer) {
            return
        }

        window.clearTimeout(DoxygenAwesomeInteractiveToc.scrollTopIdleTimer)
        DoxygenAwesomeInteractiveToc.scrollTopIdleTimer = null
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

}
