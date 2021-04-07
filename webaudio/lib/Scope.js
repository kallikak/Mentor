'use strict';

var Scope = {};

(function(namespace) { 
    let audioCtx;
    let analyser;
    let drawFilter;
            
    let scopeCanvas;
    let scopeCtx;
    let bufferLength;
    let dataArray

    let spectrumCanvas;
    let spectrumCtx;;
    let freqData;
    let filterOverlay;
    let frequencyHz;
    let magResponse;
    let phaseResponse;
            
    let chartCanvas;
    let chartCtx;
    let chartPoints = [];
    let chartSlowdown = 3;

    let scopeConfig;
    let active = false;

    namespace.getConfigTemplate = function()
    {
        const cfg = {};

        cfg.scopeId = 'scope';
        cfg.spectrumId = 'spectrum';
        cfg.chartId = null;
        cfg.fftSize = 4096;
        cfg.showFilter = false;
        cfg.filtertype = 'lowpass';
        cfg.chartSlowdown = 3;
        cfg.use_dBs = true;

        return cfg;
    }

    namespace.setup = function(cfg, ctx, inputNode)
    {
        scopeConfig = cfg;
        audioCtx = ctx;

        analyser = audioCtx.createAnalyser();
        analyser.fftSize = scopeConfig.fftSize;
        analyser.smoothingTimeConstant = 0;

        if (scopeConfig.scopeId)
        {
            scopeCanvas = document.getElementById(scopeConfig.scopeId);
            scopeCtx = scopeCanvas.getContext('2d');
            scopeCtx.fillStyle = 'rgb(30, 30, 30)';
            scopeCtx.fillRect(0, 0, scopeCanvas.width, scopeCanvas.height);
        }

        if (scopeConfig.spectrumId)
        {
            spectrumCanvas = document.getElementById(scopeConfig.spectrumId);
            spectrumCtx = spectrumCanvas.getContext('2d');
            spectrumCtx.fillStyle = 'rgb(30, 30, 30)';
            spectrumCtx.fillRect(0, 0, spectrumCanvas.width, spectrumCanvas.height);
            filterOverlay = scopeConfig.showFilter;
            const w = spectrumCtx.canvas.width;
            frequencyHz = new Float32Array(w);
            magResponse = new Float32Array(w);
            phaseResponse = new Float32Array(w);
        
            const df = audioCtx.sampleRate / analyser.fftSize;
            for (let i = 0; i < w; ++i)
            {
                // 2 * since I average adjacent points in the plots
                frequencyHz[i] = 2 * i * df;
            }
        }

        if (scopeConfig.chartId)
        {
            chartCanvas = document.getElementById(scopeConfig.chartId);
            chartCtx = chartCanvas.getContext('2d');
            chartCtx.fillStyle = 'rgb(30, 30, 30)';
            chartCtx.fillRect(0, 0, chartCanvas.width, chartCanvas.height);
        }
        
        bufferLength = analyser.frequencyBinCount;
        dataArray = new Uint8Array(bufferLength);
        freqData = new Float32Array(analyser.frequencyBinCount);

        inputNode.connect(analyser);
        if (scopeCtx)
            scopeCtx.clearRect(0, 0, scopeCtx.width, scopeCtx.height);

        if (filterOverlay)
        {
            drawFilter = audioCtx.createBiquadFilter();
            drawFilter.type = scopeConfig.filtertype;
        }

        chartSlowdown = scopeConfig.chartSlowdown;

        namespace.draw();

        return analyser;
    }

    namespace.draw = function() 
    {
        let volume = null;
        if (scopeCtx)
        {
            volume = namespace.drawScope(analyser, scopeCtx);
        }
        else if (chartCtx)
        {
            volume = getChartPoint(analyser);
        }

        if (chartCtx)
        {
            if (!active)
            {
                for (let i = 0; i < chartSlowdown; ++i)
                {
                    chartPoints.push({t:audioCtx.currentTime, v:0});
                    if (chartPoints.length == chartSlowdown * chartCanvas.width)
                        chartPoints.shift();
                }
            }
            else
            {
                chartPoints.push({t:audioCtx.currentTime, v:volume});
                if (chartPoints.length == chartSlowdown * chartCanvas.width)
                    chartPoints.shift();
            }
            namespace.drawChart(chartCtx, chartPoints);
        }

        if (spectrumCtx)
            namespace.drawSpectrum(analyser, spectrumCtx);

        if (active)
            requestAnimationFrame(namespace.draw);
    }

    namespace.setActive = function(a)
    {
        if (a == active)
            return;
        active = a;
        requestAnimationFrame(namespace.draw);
    }

    namespace.drawScope = function(analyser, ctx, triggerlevel = 1, runcount = 0) 
    {
        var w = ctx.canvas.width;
        var h = ctx.canvas.height;
        ctx.fillStyle = 'rgb(30, 30, 30)';
        // ctx.fillRect(0, 0, w, h);

        analyser.getByteTimeDomainData(dataArray);

        ctx.lineWidth = 1;
        ctx.strokeStyle = 'rgb(0, 255, 0)';
        ctx.beginPath();

        let sliceWidth = 4 * w * 2 / bufferLength;
        let x = 0;
        let triggered = triggerlevel < 0;
        // if (triggerlevel < 0)
        //     console.log("Didn't trigger");
        let range = 5;
        let recent = [];
        let max = 0;
        let min = 2;
        let crossedmiddle = false;

        function checkTriggerLevel(i)
        {
            let ok = true;
            for (let i = 0; ok && i < range; ++i)
            {
                if (i < range / 2 - 1)
                    ok = ok && recent[i] < triggerlevel;
                else if (i > range / 2)
                    ok = ok && recent[i] <= triggerlevel;
                else
                    ok = ok && recent[i] >= triggerlevel;
            }
            return ok;
        }

        function checkRisingCrossMiddle(i)
        {
            const below = recent.findIndex(x => x < 1);
            const above = recent.findIndex(x => x > 1);
            return below >= 0 && above >= 0 && below < above;
        }

        for (let i = 0; x <= w && i < bufferLength; i += 2) 
        {
            const v = (dataArray[i] + dataArray[i + 1]) / 256.0 ;
            const y = 0.2 * h / 2 + v * 0.8 * h / 2;
            if (v > max)
                max = v;
            if (v < min)
                min = v;
            if (!triggered || !crossedmiddle)
            {
                recent.push(v);
                if (recent.length > range)
                    recent.shift();
                else
                    continue;

                if (!triggered)
                    triggered = checkTriggerLevel(i);
                
                if (triggered)
                {
                    if (!crossedmiddle)
                        crossedmiddle = checkRisingCrossMiddle(i);
                    if (crossedmiddle)
                    {
                        ctx.moveTo(x, y);
                        x += sliceWidth;
                    }
                }
            }
            else if (triggered && crossedmiddle)
            {
                ctx.lineTo(x, y);
                ctx.fillRect(x - sliceWidth, 0, x, h);
                x += sliceWidth;
            }
        }

        const levels = [1, max / 2, max, -1 ];
        if (!triggered)
            namespace.drawScope(analyser, ctx, levels[++runcount], runcount);
        else if (crossedmiddle)
            ctx.stroke();

        return max - min;
    };

    let scalem = 1;
    let shiftb = 1;
    namespace.setDbScale = function(m, b)
    {
        scalem = m;
        shiftb = b;
    }
    
    function dbToY(db, h) 
    {
        let y = h / 150 * (50 - db);
        y *= scalem;
        y -= shiftb;
        return y; 
    }

    let lastoverlayupdate = 0;
    let saveq = 0;
    let saveco = 0;

    namespace.drawSpectrum = function(analyser, ctx) 
    {
        // var scaling = h / 256 / 1.5;
        var w = ctx.canvas.width;
        var h = ctx.canvas.height;

        ctx.fillStyle = 'rgb(30, 30, 30)';
        ctx.fillRect(0, 0, w, h);
        
        analyser.getFloatFrequencyData(freqData);

        ctx.lineWidth = 1;
        ctx.strokeStyle = 'rgb(0, 200, 0)';
        ctx.beginPath();

        if (scopeConfig.use_dBs)
        {
            for (var x = 0; x < 2 * w; x += 2)
                ctx.lineTo(x / 2, (dbToY(freqData[x], h) + dbToY(freqData[x + 1], h)) / 2);
            // ctx.lineTo(x, 50 - freqData[x]);
            // ctx.lineTo(x, h - 30 - freqData[x] * scaling);
        }
        else
        {
            let max = -Infinity;
            for (var x = 0; x < 2 * w; x += 2)
            {
                const val = freqData[x] + freqData[x + 1];
                freqData[x / 2] = val / 2;
                if (val > max) 
                    max = val;
            }
            max /= 2;

            // want f * Math.pow(10, max / 20) = h - 50
            // so f = (h - 50) * Math.pow(10, -max / 20)
            const scale = (h - 50) * Math.pow(10, -max / 20);
            for (var x = 0; x < w; x++)
            {
                const y = h - scale * Math.pow(10, freqData[x] / 20);
                ctx.lineTo(x, y);
            }
        }

        ctx.stroke();
        if (filterOverlay)
        {
            var params = Synth.getFilterParams();
            // computedFrequency(t) = frequency(t) * pow(2, detune(t) / 1200)
            var f = params.f * Math.pow(2, params.detune / 1200);
            if (!magResponse || saveq != params.q || saveco != params.cutoff || 
                audioCtx.currentTime - lastoverlayupdate > 0.1)
            {
                // console.log("Recalc: " + f);
                namespace.drawFilterCurve(f, params.q, ctx, false, true);
                lastoverlayupdate = audioCtx.currentTime;
                saveq = params.q;
                saveco = params.cutoff;
            }
            else
                namespace.drawFilterCurve(f, params.q, ctx, false, false);
        }
    }

    namespace.drawFilterCurve = function(cutoff, q, ctx, clear, recalc) 
    {
        if (!scopeConfig.use_dBs)   // only supported in decibels
            return;
        const w = ctx.canvas.width;
        const h = ctx.canvas.height;
        let i;
    
        if (clear)
        {
            ctx.fillStyle = 'rgb(30, 30, 30)';
            ctx.fillRect(0, 0, w, h);
        }

        if (recalc)
        {
		    drawFilter.frequency.value = cutoff;
		    drawFilter.Q.value = q;

            drawFilter.getFrequencyResponse(frequencyHz, magResponse, phaseResponse);
            // voices[0].filterNode.getFrequencyResponse(frequencyHz, magResponse, phaseResponse);
        }

        ctx.strokeStyle = 'rgb(255, 255, 0)';
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        for (i = 0; i < w; ++i) 
        {
            const response = magResponse[i];
            let dbResponse = 20.0 * Math.log(response) / Math.LN10;
            
            const x = i;
            const y = dbToY(dbResponse, h);

            if (i == 0)
                ctx.moveTo(x,y);
            else
                ctx.lineTo(x, y);
        }
        ctx.stroke();
    }

    namespace.getChartPoint = function(analyser) 
    {
        const bufferLength = analyser.frequencyBinCount;
        const dataArray = new Uint8Array(bufferLength);
        analyser.getByteTimeDomainData(dataArray);

        let max = 0;
        let min = 2;

        for (let i = 0; i < bufferLength / 2; i += 3) 
        {
            const v = dataArray[i] / 128.0;
            if (v > max)
                max = v;
            if (v < min)
                min = v;
        }

        return max - min;
    };

    namespace.drawChart = function(ctx, chartPoints) 
    {
        var w = ctx.canvas.width;
        var h = ctx.canvas.height;
        ctx.fillStyle = 'rgb(30, 30, 30)';
        ctx.fillRect(0, 0, w, h);

        ctx.lineWidth = 2;
        ctx.strokeStyle = 'rgb(0, 255, 0)';
        ctx.beginPath();

        if (chartPoints.length < chartSlowdown)
            return;

        let pt = 0;
        let k;
        for (k = 0; k < chartSlowdown; ++k)
            pt += chartPoints[k].v / chartSlowdown;
        ctx.moveTo(0, h * (1 - 0.5 * pt));
        for (let i = chartSlowdown; i < chartPoints.length - chartSlowdown + 1; i += chartSlowdown)
        {
            let pt = 0;
            for (k = 0; k < chartSlowdown; ++k)
                pt += chartPoints[i + k].v / chartSlowdown;
            ctx.lineTo(i / chartSlowdown, h * (1 - 0.5 * pt));
        }

        ctx.stroke();
    }

    namespace.install = function(config, audioCtx, inputNode)
    {
        if (document.readyState == 'loading') 
        {
            document.addEventListener('DOMContentLoaded', () => namespace.setup(config, audioCtx, inputNode));
        } 
        else 
        {
            namespace.setup(config, audioCtx, inputNode);
        }
    }
})(Scope);
