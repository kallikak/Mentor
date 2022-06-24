'use strict';

// convenient value to set to remove tests
const donotshowtests = false;

function loadTests(testid, context)
{
    if (donotshowtests)
        return;

    const samples = [];
    const def = {
        lfo: {shape: 'triangle', freq: 20, pitch: 0, amp: 0},
        osc: {shape: 'sawtooth', level: 0.5},
        filter: {cutoff: 1, q: 0, type: 'lowpass', lfo: 0, env: 0, invert: false},
        ampEnv: {a: 0, d: 1, s: 1, r: 0.05},
        filterEnv: {a: 0, d: 1, s: 1, r: 0.05},
        volume: 0.5
    };

    function loadWavetypeTests()
    {   
        const shapes = shuffle(['sine', 'triangle', 'square', 'sawtooth']);
        for (let i = 0; i < 4; ++i)
        {
            def.osc.shape = shapes[i];
            samples.push({patch: Config.copyConfig(def), note:69, duration: 1000});
        }
        
        const testtext = "Listen to each of the samples below, and try to identify the wave type." + 
            " Use the controls above to select the matching waveform, and check to see if you got it right.";
        
        const scorefn = function(checkConfig)
        {
            return checkConfig.osc.shape == context.config().osc.shape ? "✓" : "☓";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, testtext, context);
    }

    function loadPulsewidthTests()
    {   
        const vals = shuffle([...Array(10)].map((_,i )=> 0.5 + i * 0.05));
        for (let i = 0; i < 4; ++i)
            samples.push({patch:{pulsewidth: vals[i]}, note:69, duration: 1000});
            
        const testtext = 'Listen to each of the samples below, and try to match the sound. ' + 
            'Adjust the sound with the pulse width control, and then click the corresponding ' +
            '<span class="emph">Check</span> button to see how close you are.';
        
        const scorefn = function(checkConfig)
        {
            const pulse = PolyBlepPulse.getPulseOscillator();
            if (!pulse || !pulsewidth)
                return "☓";
            let diff = Math.abs(checkConfig.pulsewidth - pulsewidth);
            if (diff < 0.03)
                diff = 0;
            return (100 * (1 - diff)).toFixed(0) + "%";
        }

        const extra = function(cfg, note, onoff)
        {
            if (onoff)
            {
                if (!pulse)
                {
                    pulse = audioContext.createPulseOscillator();
                    pulse.connect(output);
                    pulse.start();
                }            
                if (pitchSlider)
                    note = pitchSlider.value / 100;
                pulse.frequency.setValueAtTime(noteToFreq(note), audioContext.currentTime);
                pulse.width.setValueAtTime(onoff ? cfg.pulsewidth : 0.5, audioContext.currentTime);
            }
            else
            {
                if (pulse)
                {
                    pulse.stop();
                    pulse.disconnect();
                    pulse = null;
                }
                fixButtonState();
                setButtonEnable(true);
            }
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, testtext, {alt: extra});
    }

    function loadMixingTests()
    {   
        const vals = shuffle([...Array(10)].map((_,i )=> 20 + i * 16));
        for (let i = 0; i < 4; ++i)
        {
            def.mix = vals[i];
            samples.push({patch: Config.copyConfig(def), note:69, duration: 1000});
        }

        const extra = function(cfg, note, onoff)
        {
            if (onoff)
            {
                context.play(cfg.mix);
            }
            else
            {
                Synth.allNotesOff();
                fixButtonState();
                setButtonEnable(true);
            }
        }
            
        const testtext = 'Listen to each of the samples below, and try to match the wavetype mix. ' + 
            'Click the corresponding ' +
            '<span class="emph">Check</span> button to see how close you are.';
        
        const scorefn = function(checkConfig)
        {
            const maxdev = Math.max(checkConfig.mix, 200 - checkConfig.mix);
            let score = 100 * (1 - Math.abs(context.val() - checkConfig.mix) / maxdev);
            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, testtext, {alt: extra});
    }

    function loadDetuneTests()
    {
        const vals = shuffle([...Array(10)].map((_,i )=> -25 + i * 5));
        for (let i = 0; i < 4; ++i)
        {
            def.detune = vals[i];
            def.osc.shape = choose(['sine', 'triangle', 'square', 'sawtooth']);
            samples.push({patch: Config.copyConfig(def), note:0, duration: 2000});
        }

        let saveconfig;
        const extra = function(cfg, note, onoff)
        {
            if (onoff)
            {
                saveconfig = Config.copyConfig(cfg);
                if (pitchSlider)
                    note = pitchSlider.value / 100;
                context.play(note, cfg.detune);
                Synth.setToConfig(cfg);
            }
            else
            {
                Synth.allNotesOff(true);
                Synth.setToConfig(saveconfig);
                fixButtonState();
                setButtonEnable(true);
            }
        }
        
        const scorefn = function(checkConfig)
        {
            if (context.val().detune === undefined)
                return "☓";
                
            const detunediff = Math.abs(Math.abs(checkConfig.detune) - Math.abs(context.val().detune));
            const shapeok = checkConfig.osc.shape === context.val().shape;

            let score = (shapeok ? 50 : 0) + (50 - 2 * detunediff);

            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, {alt: extra});
    }

    function loadLowpassTests(withresonance)
    {   
        const vals = shuffle([...Array(10)].map((_,i )=> 0.5 + i * 0.05));
        const resvals = withresonance ? shuffle([...Array(10)].map((_,i )=> 0 + i * 2)) 
            : [...Array(4)].fill(0);

        for (let i = 0; i < 4; ++i)
        {
            def.filter.cutoff = vals[i];
            def.filter.q = resvals[i];
            def.osc.shape = choose(['triangle', 'square', 'sawtooth']);
            samples.push({patch: Config.copyConfig(def), note:0, duration: 1000});
        }
        
        const scorefn = function(checkConfig)
        {
            const cutoffdiff = Math.abs(checkConfig.filter.cutoff - context.config().filter.cutoff);
            const resdiff = Math.abs(checkConfig.filter.q - context.config().filter.q) / 20;
            const shapeok = checkConfig.osc.shape === context.config().osc.shape;

            let ws = withresonance ? [33, 34, 33] : [50, 50, 0];
            let score = (shapeok ? ws[0] : 0) + ws[1] * (1 - cutoffdiff) + ws[2] * (1 - resdiff);

            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, context);
    }

    function loadHipassTests()
    {   
        const vals = shuffle([...Array(10)].map((_,i )=> 0.2 + i * 0.05));

        def.filter.type = "highpass";
        for (let i = 0; i < 4; ++i)
        {
            def.filter.cutoff = vals[i];
            def.osc.shape = choose(['triangle', 'square', 'sawtooth']);
            samples.push({patch: Config.copyConfig(def), note:0, duration: 1000});
        }
        
        const scorefn = function(checkConfig)
        {
            const cutoffdiff = Math.abs(checkConfig.filter.cutoff - context.config().filter.cutoff);
            const shapeok = checkConfig.osc.shape === context.config().osc.shape;

            let score = (shapeok ? 50 : 0) + 50 * (1 - cutoffdiff);

            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, context);
    }

    function loadADSRTests(includeSR)
    {   
        const as = shuffle([...Array(5)].map((_,i )=> 0 + i * 0.2));
        const ds = shuffle([...Array(5)].map((_,i )=> 0.1 + i * 0.1));
        const ss = shuffle([...Array(5)].map((_,i )=> 0 + i * 0.2));
        const rs = shuffle([...Array(5)].map((_,i )=> 0 + i * 0.1));

        def.volume = 1;
        for (let i = 0; i < 4; ++i)
        {
            if (includeSR)
                def.ampEnv = {a:as[i], d:ds[i], s:ss[i], r:rs[i]};
            else
                def.ampEnv = {a:as[i], d:ds[i], s:0, r:0};
            def.osc.shape = choose(['triangle', 'square', 'sawtooth']);
            samples.push({patch: Config.copyConfig(def), note:0, duration: 3000});
        }
        
        const scorefn = function(checkConfig)
        {
            const adiff = Math.abs(checkConfig.ampEnv.a - context.config().ampEnv.a);
            const ddiff = Math.abs(checkConfig.ampEnv.d - context.config().ampEnv.d);
            const sdiff = Math.abs(checkConfig.ampEnv.s - context.config().ampEnv.s);
            const rdiff = Math.abs(checkConfig.ampEnv.r - context.config().ampEnv.r);
            const shapeok = checkConfig.osc.shape === context.config().osc.shape;

            let score;
            if (includeSR)
                score = (shapeok ? 20 : 0) + 20 * [(1 - adiff) + (1 - ddiff) + (1 - sdiff) + (1 - rdiff)];
            else
                score = (shapeok ? 34 : 0) + 33 * [(1 - adiff) + (1 - ddiff)];

            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, context);
    }

    function loadFEnvTests(negative)
    {
        const covals = shuffle([...Array(10)].map(
            (_,i )=> negative ? 0.7 + i * 0.03 : 0.3 + i * 0.05));
        const resvals = shuffle([...Array(10)].map((_,i )=> 0 + i * 2));
        const envamt = shuffle([...Array(10)].map((_,i )=> 0.4 + i * 0.06));

        const as = shuffle([...Array(5)].map((_,i )=> 0 + i * 0.2));
        const ds = shuffle([...Array(5)].map((_,i )=> 0.1 + i * 0.1));
        const ss = shuffle([...Array(5)].map((_,i )=> 0 + i * 0.2));
        const rs = shuffle([...Array(5)].map((_,i )=> 0 + i * 0.1));

        def.volume = 1;
        def.osc.level = 1;
        for (let i = 0; i < 4; ++i)
        {
            def.filter.cutoff = covals[i];
            def.filter.q = resvals[i];
            def.filter.env = envamt[i];
            def.filter.invert = negative ? Math.random() > 0.5 : false;
            def.ampEnv = {a:as[i], d:ds[i], s:ss[i], r:rs[i]};
            def.filterEnv = {a:as[i], d:ds[i], s:ss[i], r:rs[i]};
            def.osc.shape = choose(['triangle', 'square', 'sawtooth']);
            samples.push({patch: Config.copyConfig(def), note:0, duration: 3000});
        }
        
        const scorefn = function(checkConfig)
        {
            const diffs = [
                Math.abs(checkConfig.filter.cutoff - context.config().filter.cutoff),
                Math.abs(checkConfig.filter.q - context.config().filter.q) / 20,
                Math.abs(checkConfig.filter.env - context.config().filter.env),
                Math.abs(checkConfig.ampEnv.a - context.config().ampEnv.a),  
                Math.abs(checkConfig.ampEnv.d - context.config().ampEnv.d),
                Math.abs(checkConfig.ampEnv.s - context.config().ampEnv.s),
                Math.abs(checkConfig.ampEnv.r - context.config().ampEnv.r)
            ];
            const invok = Math.abs(checkConfig.filter.invert - context.config().filter.invert);
            const shapeok = checkConfig.osc.shape === context.config().osc.shape;

            let score = (shapeok ? 20 : 0) + (invok ? 20 : 0) + 
                60 / 7 * diffs.reduce((a,b) => a + (1 - b));

            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, context);
    }

    function loadLFOTests(filter)
    {
        const freqs = shuffle([1, 2, 3, 5, 7, 10, 12, 15, 20, 30, 40, 50, 75, 100]);
        const pitchamt = shuffle([20, 40, 70, 100, 200, 300, 500, 800, 1200]);
        const filteramt = shuffle([...Array(10)].map((_,i )=> 40 + i * 6));
        const covals = shuffle([...Array(10)].map((_,i )=> 0.25 + i * 0.05));
        const resvals = shuffle([...Array(10)].map((_,i )=> 0 + i * 2));

        def.volume = 1;
        for (let i = 0; i < 4; ++i)
        {
            def.lfo.freq = freqs[i] / 5;
            if (filter)
            {
                def.filter.cutoff = covals[i];
                def.filter.q = resvals[i];
                def.filter.lfo = filteramt[i];
                def.osc.shape = choose(['triangle', 'square', 'sawtooth']);
            }
            else
            {
                def.lfo.pitch = pitchamt[i];
                def.osc.shape = choose(['sine', 'triangle', 'square', 'sawtooth']);
            }
            samples.push({patch: Config.copyConfig(def), note:0, duration: 3000});
        }
        
        const scorefn = function(checkConfig)
        {
            const pitchamtdiff = Math.abs(checkConfig.lfo.pitch - context.config().lfo.pitch) / 1414;
            const freqdiff = Math.abs(checkConfig.lfo.freq - context.config().lfo.freq);
            const codiff = Math.abs(checkConfig.filter.cutoff - context.config().filter.cutoff);
            const resdiff = Math.abs(checkConfig.filter.q - context.config().filter.q) / 20;
            const lfodiff = Math.abs(checkConfig.filter.q - context.config().filter.q);
            const shapeok = checkConfig.osc.shape === context.config().osc.shape;

            let score;
            if (filter)
            {
                score = (shapeok ? 20 : 0) + 1.5 * (20 - freqdiff) + 
                    50 / 3 * ((1 - codiff) + (1 - resdiff) + (100 - lfodiff) / 100);
            }
            else
            {
                score = (shapeok ? 25 : 0) + 2.5 * (20 - freqdiff) + 
                    (100 - pitchamtdiff) / 4;
            }

            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, context);
    }

    function loadLFOShapeTests()
    {
        fixButtonState = () => {};  // don't want to remove the button class
        const freqs = shuffle([1, 2, 3, 5, 7, 10, 12, 15, 20, 30, 40, 50, 75, 100]);
        const pitchamt = shuffle([20, 40, 70, 100, 200, 300, 500, 800, 1200]);
        const filteramt = shuffle([...Array(10)].map((_,i )=> 10 + i * 9));
        const covals = shuffle([...Array(10)].map((_,i )=> 0.25 + i * 0.05));
        const resvals = shuffle([...Array(10)].map((_,i )=> 0 + i * 2));

        def.volume = 1;
        for (let i = 0; i < 4; ++i)
        {
            def.lfo.freq = freqs[i] / 5;
            def.filter.cutoff = covals[i];
            def.filter.q = resvals[i];
            def.filter.lfo = filteramt[i];
            def.lfo.shape = choose(['sine', 'triangle', 'square', 'sawtooth', 'ramp']);
            def.lfo.pitch = pitchamt[i];
            samples.push({patch: Config.copyConfig(def), note:0, duration: 3000});
        }
        
        const scorefn = function(checkConfig)
        {
            const pitchamtdiff = Math.abs(checkConfig.lfo.pitch - context.config().lfo.pitch) / 1414;
            const freqdiff = Math.abs(checkConfig.lfo.freq - context.config().lfo.freq);
            const codiff = Math.abs(checkConfig.filter.cutoff - context.config().filter.cutoff);
            const resdiff = Math.abs(checkConfig.filter.q - context.config().filter.q) / 20;
            const lfodiff = Math.abs(checkConfig.filter.lfo - context.config().filter.lfo);
            const shapeok = checkConfig.lfo.shape === context.config().lfo.shape;

            let score = (shapeok ? 30 : 0) + (20 - freqdiff) + 
                    50 / 4 * ((1 - codiff) + (1 - resdiff) + (1 - pitchamtdiff) / 1 + (100 - lfodiff) / 100);

            console.log("-----------------");
            console.log(checkConfig.lfo.shape, context.config().lfo.shape);
            console.log("pitchamt, freq, co, res, lfo");
            console.log(checkConfig.lfo.pitch, checkConfig.lfo.freq, checkConfig.filter.cutoff, 
                checkConfig.filter.q, checkConfig.filter.lfo);
            console.log(context.config().lfo.pitch, context.config().lfo.freq, context.config().filter.cutoff, 
                context.config().filter.q, context.config().filter.lfo);
            console.log(pitchamtdiff, freqdiff, codiff, resdiff, lfodiff);
            console.log(score);

            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, context);
    }

    function loadEffectsTests()
    {
        const levels = shuffle([...Array(10)].map((_,i )=> 20 + i * 8));
        const depths = shuffle([...Array(10)].map((_,i )=> 20 + i * 8));
        const speeds = shuffle([...Array(10)].map((_,i )=> 20 + i * 8));
        
        for (let i = 0; i < 4; ++i)
        {
            const effect = choose([CHORUS, FLANGER, PHASER]);
            const shape = choose(['triangle', 'square', 'sawtooth']);
            samples.push({patch:{shape, effect, level: levels[i], depth: depths[i], speed: speeds[i]}, 
                note:0, duration: 3000});
        }
        
        const scorefn = function(checkConfig)
        {
            const shapeok = checkConfig.shape === context.config().shape;
            const effectok = checkConfig.effect === context.config().effect;
            const depthdiff = Math.abs(checkConfig.depth - context.config().depth) / 100;
            const speeddiff = Math.abs(checkConfig.speed - context.config().speed) / 100;
            const leveldiff = Math.abs(checkConfig.level - context.config().level) / 100;

            let score = (shapeok ? 20 : 0) + (effectok ? 32 : 0) + 
                16 * ((1 - depthdiff) + (1 - speeddiff) + (1 - leveldiff));
            if (score > 97)
                score = 100;
            return score.toFixed(0) + "%";
        }

        const extra = function(cfg, note, onoff)
        {
            if (onoff)
            {
                effect = cfg.effect;
                context.play(cfg.shape);
                wetGain.gain.value = cfg.level / 100;
                effectNode.setDepth(cfg.depth / 100);
                effectNode.setSpeed(cfg.speed / 100);
            }
            else
            {
                context.stop();
                fixButtonState();
                setButtonEnable(true);
            }
        }
        
        setupTesters("controls", "testtable", "testtext", samples, scorefn, null, {alt: extra});   
    }

    switch (testid)
    {
        case "wavetypes":
            loadWavetypeTests();
            break;
        case "pulsewidth":
            loadPulsewidthTests();
            break;
        case "mixing":
            loadMixingTests();
            break;
        case "detune":
            loadDetuneTests();
            break;
        case "lowpass":
            loadLowpassTests(false);
            break;
        case "lowpassres":
            loadLowpassTests(true);
            break;
        case "hipass":
            loadHipassTests();
            break;
        case "ad":
            loadADSRTests(false);
            break;
        case "adsr":
            loadADSRTests(true);
            break;
        case "fenv":
            loadFEnvTests(false);
            break;
        case "negfenv":
            loadFEnvTests(true);
            break;
        case "lfopitch":
            loadLFOTests(false);
            break;
        case "lfofilter":
            loadLFOTests(true);
            break;
        case "lfoshapes":
            loadLFOShapeTests();
            break;
        case "chorus":
            loadEffectsTests();
            break;
        default:
            break;
    }
}
