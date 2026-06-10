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
                    <path d="M152 44C134.3 44 120 58.3 120 76C120 93.7 134.3 108 152 108L488 108C505.7 108 520 93.7 520 76C520 58.3 505.7 44 488 44L152 44zM342.6 157.4C330.1 144.9 309.8 144.9 297.3 157.4L137.3 317.4C124.8 329.9 124.8 350.2 137.3 362.7C149.8 375.2 170.1 375.2 182.6 362.7L288 257.3L288 556C288 573.7 302.3 588 320 588C337.7 588 352 573.7 352 556L352 257.3L457.4 362.7C469.9 375.2 490.2 375.2 502.7 362.7C515.2 350.2 515.2 329.9 502.7 317.4L342.7 157.4z"/>
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
