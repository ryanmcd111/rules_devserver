console.log("inside devserver_loader.js");

setInterval(() => {
    fetch("/devserver/manifest")
        .then(function (response) {
            return response.json();
        })
        .then(function (manifest) {
            console.warn(manifest);
        })
}, 1000);
