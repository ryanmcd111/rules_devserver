function add(a, b) {
    return a + b;
}

const container = document.getElementById('container');
if (container) {
    container.innerHTML = 'Served addition from JS: 1 + 2 = ' + add(1, 2);
}
