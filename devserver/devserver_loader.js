function maybeReplaceJavaScript(src) {
    const script = document.querySelector(`script[src="${src}"]`);
    if (script) {
        const newScript = document.createElement("script");
        newScript.src = src;
        newScript.dataset.hotSwap = "true";
        script.parentNode.replaceChild(newScript, script);
    }
}

function liveReload(path) {
    console.log("Reloading due to change in " + path);
    window.location.reload();
}

setInterval(() => {
    fetch("/devserver/manifest")
        .then(function (response) {
            return response.json();
        })
        .then(function (manifest) {
            if (!window.__DEVSERVER_MANIFEST__) {
                __DEVSERVER_MANIFEST__ = manifest;
                return;
            }

            for (const path in manifest) {
                if (manifest[path] !== __DEVSERVER_MANIFEST__[path]) {
                    if (path.endsWith(".js")) {
                        let script = document.querySelector(`script[src="${path}"]`);
                        if (script && script.dataset.hotSwap === "true") {
                            maybeReplaceJavaScript(path);
                            __DEVSERVER_MANIFEST__ = manifest;
                        } else {
                            liveReload(path);
                        }
                    } else {
                        liveReload(path);
                    }
                }
            }
        })
}, 1000);

