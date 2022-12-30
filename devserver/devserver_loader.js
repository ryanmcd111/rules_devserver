console.log("inside devserver_loader.js");

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

            for (let key in manifest) {
                if (manifest[key] !== __DEVSERVER_MANIFEST__[key]) {
                    console.log("Reloading due to change in " + key);
                    window.location.reload();
                }
            }
        })
}, 1000);
