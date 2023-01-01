(function () {
    const container = document.getElementById('counter-container');
    if (container) {
        let count = 0;
        container.innerHTML = `Count: ${count}`;
        setInterval(() => {
            count++;
            container.innerHTML = `Count: ${count}`;
        }, 1000);
    }
})();