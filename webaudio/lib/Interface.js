'use strict';

function manageButtonState(btn, managedisable)
{
    const isstop = btn.id === "stopButton";
    document.getElementsByClassName("buttons")[0].querySelectorAll('button').forEach((e) => {
        e.classList.remove("bluebuttonpressed")
        if (managedisable && e.id !== "stopButton")
            e.disabled = !isstop;
    });
    btn.classList.add("bluebuttonpressed");

    if (managedisable)
    {
        if (btn.id === "stopButton")
            document.querySelectorAll('canvas').forEach((c) => c.classList.add('disabled'));
        else
            document.querySelectorAll('canvas').forEach((c) => c.classList.remove('disabled'));
    }
}

function makeElement(type, id, classtype, inner)
{
    const elem = document.createElement(type);
    if (classtype)
        elem.setAttribute("class", classtype);
    if (id)
        elem.setAttribute("id", id);
    if (inner)
        elem.innerHTML = inner;
    return elem;
}       

function makeScopes(containerid, scopeid, spectrumid, chartid, widths, height)
{
    let scopeCanvas = null;
    let spectrumCanvas = null;
    let chartCanvas = null;

    let headerrow = makeElement("tr");
    let scoperow = makeElement("tr");
    if (scopeid)
    {
        let cell = makeElement("td", scopeid + "header", null);
        cell.innerHTML = "Waveform";
        headerrow.appendChild(cell);
        scopeCanvas = makeElement("canvas", scopeid, "scope");
        scopeCanvas.setAttribute("width", widths[0]);
        scopeCanvas.setAttribute("height", height);
        cell = makeElement("td", null, null);
        cell.appendChild(scopeCanvas);
        scoperow.appendChild(cell);
    }
    if (spectrumid)
    {
        let cell = makeElement("td", spectrumid + "header", null);
        cell.innerHTML = "Spectrum";
        headerrow.appendChild(cell);
        spectrumCanvas = makeElement("canvas", spectrumid, "scope");
        spectrumCanvas.setAttribute("width", widths[1]);
        spectrumCanvas.setAttribute("height", height);
        cell = makeElement("td", null, null);
        cell.appendChild(spectrumCanvas);
        scoperow.appendChild(cell);
    }
    if (chartid)
    {
        let cell = makeElement("td", chartid + "header", null);
        cell.innerHTML = "Envelope";
        headerrow.appendChild(cell);
        chartCanvas = makeElement("canvas", chartid, "scope");
        chartCanvas.setAttribute("width", widths[2]);
        chartCanvas.setAttribute("height", height);
        cell = makeElement("td", null, null);
        cell.appendChild(chartCanvas);
        scoperow.appendChild(cell);
    }

    document.getElementById(containerid).appendChild(headerrow);
    document.getElementById(containerid).appendChild(scoperow);
}    

function makeParagraph(appendto)
{
    document.getElementById(appendto).appendChild(makeElement("p"));
}

function makeSlider(id, label, width, min, max, value, slidersid)
{
    const p = makeElement("p", id + "Control", "sliderControl");
    const slider = makeElement("input", id + "Slider", "slider");
    const output = makeElement("span", id + "Output", "controlAmount");

    slider.setAttribute("min", min); 
    slider.setAttribute("max", max); 
    slider.setAttribute("value", value); 
    slider.setAttribute("type", "range");
    slider.style.cssText = `width:${width}px;`;

    p.innerHTML = `<span class="controlLabel">${label}</span>` + slider.outerHTML + output.outerHTML;

    document.getElementById(slidersid).appendChild(p);
    return {slider, output};
}

function makeSliderCallback(id, callback, immediate, valFn, labelFn)
{
    const slider = document.getElementById(id + "Slider");
    slider.oninput = () => { 
        const v = valFn(slider.value);
        callback(v);
        if (labelFn)
            document.getElementById(id + "Output").innerHTML = labelFn(v); 
    }
    if (immediate)
        slider.oninput();
    return slider;
}

function getDefault(defaults, i, val)
{
    return defaults && defaults[i] != undefined ? defaults[i] : val;
}

function addStandardPitchSlider(slidersId, addAmp, defaults)
{
    let pitchSlider, ampSlider;
    makeSlider("pitch", "Pitch:", 400, 3000, 10300, getDefault(defaults, 0, 6900), slidersId);
    pitchSlider = makeSliderCallback("pitch", 
        f => Synth.setOscFrequency(f), true, 
        v => Synth.noteToFreq(v / 100), f => f.toFixed(0) + "Hz");
    if (addAmp)
    {
        makeSlider("amp", "Amplitude:", 400, 0, 100, getDefault(defaults, 1, 50), slidersId);
        ampSlider = makeSliderCallback("amp", 
            a => Synth.setOscLevel(a / 100), true,
            v => Number(v), a => a.toFixed(0) + "%");
    }
    return [pitchSlider, ampSlider].filter(s => s != null);
}

function addStandardLFOSlider(slidersId, addPitchAmount, addFilterAmount, defaults)
{
    let lfoSlider, pamtSlider, famtSlider;
    makeSlider("lfo", "LFO Freq:", 400, 1, 40, getDefault(defaults, 0, 2), slidersId);
    lfoSlider = makeSliderCallback("lfo", 
    a => Synth.setLFOFrequency(a / 2), true,
    v => Number(v), a => (a / 2) + "Hz");
    if (addPitchAmount)
    {
        makeSlider("pamt", "Pitch Amount:", 400, 0, 1414, getDefault(defaults, 1, 0), slidersId);
        pamtSlider = makeSliderCallback("pamt", 
            a => Synth.setLFOOscAmount(a), true,
            v => Number(v), a => (a / 14.14).toFixed(0) + "%");
    }
    if (addFilterAmount)
    {
        makeSlider("famt", "Filter Amount:", 400, 0, 100, getDefault(defaults, 2, 0), slidersId);
        famtSlider = makeSliderCallback("famt", 
            a => Synth.setFilterLFOAmount(a), true,
            v => Number(v), a => a + "%");        
    }
    return [lfoSlider, pamtSlider, famtSlider].filter(s => s != null);
}

function addStandardFilterSlider(slidersId, addResonance, defaults)
{
    let filterSlider, resSlider;
    makeSlider("filter", "Filter cutoff:", 400, 0, 100, getDefault(defaults, 0, 100), slidersId);
    filterSlider = makeSliderCallback("filter", 
        co => Synth.setFilterCutoff(co), true, 
        v => Number(v) / 100, co => {
            const f = Synth.calcCutoffFrequency(co);
            return f > 1000 ? (f / 1000).toFixed(2) + "kHz" : f.toFixed(0) + "Hz"; 
        });
    if (addResonance)
    {
        makeSlider("res", "Resonance:", 400, 0, 200, getDefault(defaults, 1, 0), slidersId);
        resSlider = makeSliderCallback("res", 
            q => Synth.setResonance(q), true, 
            v => Number(v) / 10, q => q.toFixed(2) + "dB");
    }
    return [filterSlider, resSlider].filter(s => s != null);
}

function makeVerticalSlider(id, label, width, min, max, value, slidersid)
{
    const slider = makeElement("input", id + "Slider", "slider");

    slider.setAttribute("min", min); 
    slider.setAttribute("max", max); 
    slider.setAttribute("value", value); 
    slider.setAttribute("type", "range");
    // slider.style.cssText = `width:${width}px;`;

    document.getElementById(slidersid).appendChild(slider);
    return slider;
}

function addADSRSliders(slidersId, adOnly, defaults)
{
    let attackSlider, decaySlider, sustainSlider, releaseSlider;

    makeVerticalSlider("attack", null, 70, 0, 100, getDefault(defaults, 0, 0), slidersId);
    makeVerticalSlider("decay", null, 70, 0, 100, getDefault(defaults, 1, 100), slidersId);
        
    if (!adOnly)
    {
        makeVerticalSlider("sustain", null, 70, 0, 100, getDefault(defaults, 2, 100), slidersId);
        makeVerticalSlider("release", null, 70, 0, 100, getDefault(defaults, 3, 0), slidersId);
    }
    
    const container = document.getElementById(slidersId);
    container.appendChild(makeElement("br", null, null));
    container.appendChild(makeElement("span", null, null, "Attack"));
    container.appendChild(makeElement("span", null, null, "Decay"));

    attackSlider = makeSliderCallback("attack", 
        a => Synth.setAmpEnvA(a), true, 
        v => Number(v) / 100, null);

    decaySlider = makeSliderCallback("decay", 
        d => Synth.setAmpEnvD(d), true, 
        v => Number(v) / 100, null);
        
    if (!adOnly)
    {
        container.appendChild(makeElement("span", null, null, "Sustain"));
        container.appendChild(makeElement("span", null, null, "Release"));
        sustainSlider = makeSliderCallback("sustain", 
            s => Synth.setAmpEnvS(s), true, 
            v => Number(v) / 100, null);
    
        releaseSlider = makeSliderCallback("release", 
            r => Synth.setAmpEnvR(r), true, 
            v => Number(v) / 100, null);
    }

    return [attackSlider, decaySlider, sustainSlider, releaseSlider].filter(s => s != null);
}

function addButton(containerId, name, callback, classes = ["bluebutton"])
{
    const b = makeElement("button", name + "Button", classes ? classes[0] : null);
    if (classes.length > 1)
        classes.forEach(c => b.classList.add(c));
    b.innerHTML = name;
    document.getElementById(containerId).appendChild(b);
    b.onclick = e => callback(name, e.target);
}

// For when the app is in an iframe
function setupParentCommunication()
{
    const theApp = document.getElementsByClassName('webaudio')[0];
    window.parent.postMessage({
        file: document.location.href.split('/').reverse()[0],
        height:theApp.scrollHeight,
        width: theApp.scrollWidth
    },'*');
    window.addEventListener("message", e => {
        if (e.data.isopen)
        {
            window.parent.postMessage({
                file: document.location.href.split('/').reverse()[0],
                height: theApp.scrollHeight,
                width: theApp.scrollWidth
            },'*');
        }
        else
        {
            const stopButton = document.getElementById('stopButton');
            if (stopButton)
                stopButton.onclick({target:stopButton});
        }
    });
}



