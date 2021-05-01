
'use strict';

const FFT = function (size) {
    const me = {};

    let n, m;

    let cos = [];
    let sin = [];

    (function init(size) {
        n = size;
        m = Math.floor(Math.log2(n));

        // Make sure n is a power of 2
        if (n != (1 << m))
            throw "FFT length must be power of 2";

        // precompute tables
        cos = new Array(n / 2);
        sin = new Array(n / 2);

        for (let i = 0; i < n / 2; i++) {
            cos[i] = Math.cos(-2 * Math.PI * i / n);
            sin[i] = Math.sin(-2 * Math.PI * i / n);
        }
    })(size);

    me.fft = function (x, y) {
        let i;
        let j = 0;
        let n1;
        let n2 = n / 2;
        for (i = 1; i < n - 1; i++) {
            n1 = n2;
            while (j >= n1) {
                j -= n1;
                n1 /= 2;
            }
            j += n1;
            if (i < j) {
                [x[i], x[j]] = [x[j], x[i]];
                [y[i], y[j]] = [y[j], y[i]];
            }
        }

        n1 = 0;
        n2 = 1;
        for (i = 0; i < m; i++) {
            n1 = n2;
            n2 = 2 * n2;
            let a = 0;
            for (j = 0; j < n1; j++) {
                const c = cos[a];
                const s = sin[a];
                a += 1 << (m - i - 1);
                for (let k = j; k < n; k = k + n2) {
                    const t1 = c * x[k + n1] - s * y[k + n1];
                    const t2 = s * x[k + n1] + c * y[k + n1];
                    x[k + n1] = x[k] - t1;
                    y[k + n1] = y[k] - t2;
                    x[k] += t1;
                    y[k] += t2;
                }
            }
        }
    }
    return me;
}