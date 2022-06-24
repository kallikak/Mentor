'use strict';

const Synth = {};

const Config = {};

(function(context) {
    const ascale = 2;
    const dscale = 10;
    const rscale = 10;

    context.copyConfig = function(c)
    {
        const cfg = JSON.parse(JSON.stringify(c));

        cfg.getAttackValue = () => cfg.ampEnv.a * ascale;
        cfg.getDecayValue = () => cfg.ampEnv.d * dscale;
        cfg.getReleaseValue = () => cfg.ampEnv.r * rscale;
    
        cfg.getFilterAttackValue = () => cfg.filterEnv.a * ascale;
        cfg.getFilterDecayValue = () => cfg.filterEnv.d * dscale;
        cfg.getFilterReleaseValue = () => cfg.filterEnv.r * rscale;

        return cfg;
    }

    const config = Config.copyConfig(
        {
            lfo: {shape: 'triangle', freq: 2, pitch: 0, amp: 0},
            osc: {shape: 'sawtooth', level: 0.5},
            filter: {cutoff: 1, q: 0, type: 'lowpass', lfo: 0, env: 0, invert: false},
            ampEnv: {a: 0.2, d: 1, s: 1, r: 0.2},
            filterEnv: {a: 0.2, d: 1, s: 1, r: 0.2},
            volume: 0.5
        });

    context.getDefault = function()
    {
        return Config.copyConfig(config);
    }

    context.setEnvelope = function(config, a, d, s, r, setAmp, setFilter)
    {
        if (setAmp)
            config.ampEnv = {a, d, s, r};
        if (setFilter)
            config.filterEnv = {a, d, s, r};
        return config;
    }

    context.setHighPass = function(config, f)
    {
        config.filter.type = 'highpass';
        config.filter.cutoff = f;
        return config;
    }

    context.setLowPass = function(config, f, q)
    {
        config.filter.type = 'lowpass';
        config.filter.cutoff = f;
        config.filter.Q = q;
        return config;
    }
})(Config);

(function(namespace) { 
    let voices = [];
    let audioContext = null;
    let config = Config.getDefault();

    let lfoOsc = null;
    let lfoAmpGain = null;
    let lfoFilterGain = null;
    let lfoOscGain = null;

    let volNode = null;

    namespace.noteToFreq = function(note) 
    {
        return 440 * Math.pow(2, (note - 69) / 12);
    }

    namespace.noteOn = function(note) 
    {
        let v = new Voice(note, lfoOscGain, lfoFilterGain);
        voices.push(v);
        // console.log(voices.length);
        return v;
    }

    namespace.freqOn = function(f) 
    {
        let v = new Voice(10, lfoOscGain, lfoFilterGain);
        v.setOscFrequency(f);
        voices.push(v);
        return v;
    }

    namespace.noteOff = function(voice, immediate, callback) 
    {
        voice.noteOff(immediate, () => {
            let index = voices.indexOf(voice); 
            const k = voices.length;
            if (index >= 0) 
                voices.splice(index, 1);  
            if (callback)
                callback();
        });
    }

    namespace.allNotesOff = function(immediate, callback) 
    {
        const enhancedcallback = () => {
            if (callback && voices.length == 0)
                callback();
        };
        const n = voices.length;    
        if (n == 0 && callback)
            callback();
        else
            voices.forEach(v => namespace.noteOff(v, immediate, enhancedcallback));
    }

    namespace.isPlaying = function()
    {
        return voices.length > 0;
    }

    namespace.setLFOShape = function(w) 
    {
        config.lfo.shape = w;
        if (w === 'ramp')
        {
            w = 'sawtooth';
            namespace.setLFOOscAmount(-Math.abs(config.lfo.pitch));
            namespace.setFilterLFOAmount(-Math.abs(config.filter.lfo));
            namespace.setLFOAmpAmount(-Math.abs(config.lfo.amp));
        }
        else
        {
            namespace.setLFOOscAmount(Math.abs(config.lfo.pitch));
            namespace.setFilterLFOAmount(Math.abs(config.filter.lfo));
            namespace.setLFOAmpAmount(Math.abs(config.lfo.amp));
        }
        // break and reestablish LFO modulation connections
        lfoOsc.stop();
        lfoOsc.disconnect();
        if (config.lfo.shape === 'sandh')
        {
            lfoOsc = audioContext.createSandH();
        }
        else
        {
            lfoOsc = audioContext.createOscillator();
            lfoOsc.type = w;
        }
        lfoOsc.frequency.value = config.lfo.freq;
        lfoOsc.connect(lfoAmpGain);
        lfoOsc.connect(lfoFilterGain);
        lfoOsc.connect(lfoOscGain);
        lfoOsc.start();
    }

    namespace.setLFOFrequency = function(f) 
    {
        config.lfo.freq = f;
        // FIXME need both because of differences in Chrome and Safari behaviour
        lfoOsc.frequency.value = config.lfo.freq;
        lfoOsc.frequency.setValueAtTime(f, audioContext.currentTime);
    }

    namespace.setLFOOscAmount = function(u) 
    {
        config.lfo.pitch = u;
        lfoOscGain.gain.setValueAtTime(u, audioContext.currentTime);
    }

    namespace.setLFOAmpAmount = function(u) 
    {
        config.lfo.amp = u;
        lfoAmpGain.gain.value = u;
    }

    namespace.setFilterCutoff = function(u) 
    {
        config.filter.cutoff = u;
        voices.forEach(v => {
            if (v)
                v.setFilterCutoff(u);
        });
    }

    namespace.setFilterType = function(t) 
    {
        config.filter.type = t;
        voices.forEach(v => {
            if (v)
                v.setFilterType(t);
        });
    }

    // set a specific frequency for the cutoff
    namespace.setFilterCutoffFreq = function(f) 
    {
        config.filter.cutoff = namespace.calcInverseCutoffFrequency(f);   
        voices.forEach(v => {
            if (v)
                v.setFilterCutoffFreq(f);
        });
    }

    namespace.setResonance = function(u) 
    {
        config.filter.q = u;
        voices.forEach(v => {
            if (v)
                v.setResonance(u);
        });
    }

    namespace.setFilterLFOAmount = function(u) 
    {
        config.filter.lfo = u;
        lfoFilterGain.gain.setValueAtTime(config.filter.lfo * 24, audioContext.currentTime);
    }

    namespace.setFilterEnvAmount = function(u) 
    {
        config.filter.env = u;
    }

    namespace.setOscFrequency = function(w) 
    {
        voices.forEach(v => {
            if (v)
                v.setOscFrequency(w);
        });
    }

    namespace.setOscShape = function(w) 
    {
        config.osc.shape = w;
        voices.forEach(v => {
            if (v)
                v.setOscShape(w);
        });
    }

    namespace.setPeriodicWave = function(real, imag)
    {
        config.osc.shape = 'periodic';
        config.osc.real = real;
        config.osc.imag = imag;
        voices.forEach(v => {
            if (v)
                v.setPeriodicWave(real, imag);
        });
    }

    namespace.setOscLevel = function(u) 
    {
        config.osc.level = u;
        voices.forEach(v => {
            if (v)
                v.setOscLevel(u);
        });
    }

    namespace.setAmpEnvA = function(u) { config.ampEnv.a = u; }
    namespace.setAmpEnvD = function(u) { config.ampEnv.d = u; }
    namespace.setAmpEnvS = function(u) { config.ampEnv.s = u; }
    namespace.setAmpEnvR = function(u) { config.ampEnv.r = u; }

    namespace.setFilterEnvA = function(u) { config.filterEnv.a = u; }
    namespace.setFilterEnvD = function(u) { config.filterEnv.d = u; }
    namespace.setFilterEnvS = function(u) { config.filterEnv.s = u; }
    namespace.setFilterEnvR = function(u) { config.filterEnv.r = u; }

    namespace.calcCutoffFrequency = function(cutoff) 
    {
        const nyquist = audioContext.sampleRate / 2;
        const noctaves = Math.log(nyquist / 10.0) / Math.LN2;
        return Math.pow(2.0, noctaves * (cutoff - 1.0)) * nyquist;
    }

    namespace.calcInverseCutoffFrequency = function(f) 
    {
        const nyquist = audioContext.sampleRate / 2;
        const noctaves = Math.log(nyquist / 10.0) / Math.LN2;
        return Math.log2(f / nyquist) / noctaves + 1;
    }

    namespace.getFilterParams = function()
    {
        return {f: namespace.calcCutoffFrequency(config.filter.cutoff), q: config.filter.q, 
            detune: voices.length ? voices[0].getFilterDetune() : 0};
    }

    namespace.setVolume = function(v)
    {
        config.volume = v;
        volNode.gain.linearRampToValueAtTime(v, audioContext.currentTime + 0.2);
    }

    function Voice(note, lfoOscGain, lfoFilterGain) 
    {
        const me = {};

        me.setOscShape = function(value) 
        {
            if (me.osc)
                me.osc.type = value;
            me.setOscLevel(config.osc.level);
        }

        me.setPeriodicWave = function(real, imag)
        {
            if (me.osc)
            {
                // me.osc.type = 'periodic';
                me.osc.real = real;
                me.osc.imag = imag;
                const wave = audioContext.createPeriodicWave(config.osc.real, config.osc.imag, {disableNormalization: true});
                me.osc.setPeriodicWave(wave);
            }
        }

        me.setOscFrequency = function(value) 
        {
            if (me.osc)
            {
                me.freq = value;
                me.osc.frequency.setValueAtTime(value, audioContext.currentTime);
            }
        }

        me.glideToFrequency = function(value, glidetime) 
        {
            if (me.osc) 
            {
                me.osc.frequency.cancelScheduledValues(audioContext.currentTime);
                me.osc.frequency.setValueAtTime(me.freq, audioContext.currentTime);
                me.osc.frequency.exponentialRampToValueAtTime(value, audioContext.currentTime + glidetime);
                me.freq = value;
            }
        }

        me.setOscDetune = function(value) 
        {
            if (me.osc)
                me.osc.detune.setValueAtTime(value, audioContext.currentTime);
        }

        me.setOscLevel = function(value) 
        {
            me.oscGain.gain.setValueAtTime((config.osc.shape === 'sawtooth' ? -1 : 1) * value, audioContext.currentTime);
        }

        me.setFilterType = function(type) 
        {
            me.filter.type = type;
        }

        me.setFilterCutoff = function(value) 
        {
            me.filter.frequency.setValueAtTime(namespace.calcCutoffFrequency(value), audioContext.currentTime);
        }

        me.setFilterCutoffFreq = function(f) 
        {
            me.filter.frequency.setValueAtTime(f, audioContext.currentTime);
        }

        me.setResonance = function(value) 
        {
            me.filter.Q.setValueAtTime(value, audioContext.currentTime);
        }

        me.getFilterDetune = function()
        {
            return me.filter.detune.value;
        }

        me.noteOff = function(immediate, callback) 
        {
            if (!me.osc)
                return; // already cleaned up
            // me.osc.onended = callback;
            var now =  audioContext.currentTime;
            var release = config.getReleaseValue();	

            me.envelope.gain.cancelScheduledValues(now);
            me.envelope.gain.setValueAtTime(me.envelope.gain.value, now);  // this is necessary because of the linear ramp
            if (immediate)
                me.envelope.gain.setValueAtTime(0, now + 0.01);  
            else
            {
                me.envelope.gain.setTargetAtTime(0, now + 0.01, release / 5);
                me.envelope.gain.cancelScheduledValues(now + release + 0.01);
                me.envelope.gain.setValueAtTime(0, now + release + 0.01);
            }
            me.filter.detune.cancelScheduledValues(now);
            me.filter.detune.setTargetAtTime(config.invert ? 7200 : 0, now + 0.01, config.getFilterReleaseValue() / 5 + 0.01);
            const endTime = now + (immediate ? 0.012 : release);
            try
            {
                me.envelope.gain.cancelScheduledValues(endTime);
                setTimeout(() => {
                    me.envelope.disconnect();
                    if (me.osc)
                    {
                        me.osc.stop();
                        // give stop a bit of time to do its stuff before disconnecting
                        setTimeout(() => {
                            if (me.osc)
                            {
                                me.osc.disconnect();
                                lfoOscGain.disconnect(me.osc.detune);
                                lfoFilterGain.disconnect(me.filter.detune);
                                me.osc = null;
                            }
                            if (callback)
                                callback();
                        }, 30);
                    }
                }, (endTime - now) * 1000);
            }
            catch (e) {}
        }

        // create osc
        me.osc = audioContext.createOscillator();
        
        me.note = note;
        if (note >= 0)
            me.setOscFrequency(Synth.noteToFreq(note));
        if (config.osc.shape === 'periodic')
        {
            const wave = audioContext.createPeriodicWave(config.osc.real, config.osc.imag, {disableNormalization: true});
            me.osc.setPeriodicWave(wave);
        }
        else
        {
            me.osc.type = config.osc.shape;
        }

        me.oscGain = audioContext.createGain();
        me.negate = config.osc.shape === 'sawtooth';
        me.setOscLevel(config.osc.level);

        me.osc.connect(me.oscGain);
        
        lfoOscGain.connect(me.osc.detune);

        // create the LP filter
        me.filter = audioContext.createBiquadFilter();
        me.filter.type = config.filter.type;
        me.filter.Q.value = config.filter.q;
        me.filter.frequency.value = namespace.calcCutoffFrequency(config.filter.cutoff); 

        me.oscGain.connect(me.filter);

        // connect the LFO to the filters
        lfoFilterGain.gain.value = config.filter.lfo * 24;
        lfoFilterGain.connect(me.filter.detune);

        // create the volume envelope
        me.envelope = audioContext.createGain();
        me.filter.connect(me.envelope);
        me.envelope.connect(volNode);

        // set up the volume and filter envelopes
        var now = audioContext.currentTime;
        var envAttackEnd = now + config.getAttackValue() + 0.001;

		me.envelope.gain.cancelScheduledValues(now);
        me.envelope.gain.setValueAtTime(0.0, now);
        me.envelope.gain.linearRampToValueAtTime(1.0, envAttackEnd);
        me.envelope.gain.setTargetAtTime(config.ampEnv.s, envAttackEnd, config.getDecayValue() / 5);

        let filterAttackLevel = config.filter.env * 7200;  // Range: 0-7200: 6 octave range
        let filterSustainLevel = filterAttackLevel * config.filterEnv.s; // range: 0-7200
        if (filterSustainLevel < 0.00001)
            filterSustainLevel = 0.00001;
        let filterAttackEnd = config.getFilterAttackValue();
        if (config.filter.invert) {
            filterAttackLevel *= -1;
            filterSustainLevel *= -1;
        }

        if (!filterAttackEnd) 
            filterAttackEnd = 0.05; // tweak to get target decay to work properly
        me.filter.detune.setValueAtTime(0, now);
        me.filter.detune.linearRampToValueAtTime(filterAttackLevel, now + filterAttackEnd);
        me.filter.detune.setTargetAtTime(filterSustainLevel, 
            now + filterAttackEnd, config.getFilterDecayValue() / 5);
        me.osc.start(now);

        return me;
    }
    
    namespace.setToConfig = function(cfg)
    {
        namespace.setLFOShape(cfg.lfo.shape);
        namespace.setLFOFrequency(cfg.lfo.freq);
        namespace.setLFOOscAmount(cfg.lfo.pitch);
        namespace.setLFOAmpAmount(cfg.lfo.amp);
            
        // namespace.setOscFrequency = function(w) 
        namespace.setOscShape(cfg.osc.shape);
        namespace.setOscLevel(cfg.osc.level);
    
        namespace.setFilterCutoff(cfg.filter.cutoff);
        namespace.setResonance(cfg.filter.q);
        namespace.setFilterType(cfg.filter.type);
        namespace.setFilterLFOAmount(cfg.filter.lfo);
        namespace.setFilterEnvAmount(cfg.filter.env);
        config.filter.invert = cfg.filter.invert;
    
        namespace.setAmpEnvA(cfg.ampEnv.a);
        namespace.setAmpEnvD(cfg.ampEnv.d);
        namespace.setAmpEnvS(cfg.ampEnv.s);
        namespace.setAmpEnvR(cfg.ampEnv.r);
    
        namespace.setFilterEnvA(cfg.filterEnv.a);
        namespace.setFilterEnvD(cfg.filterEnv.d);
        namespace.setFilterEnvS(cfg.filterEnv.s);
        namespace.setFilterEnvR(cfg.filterEnv.r);
    
        namespace.setVolume(cfg.volume);
    }
    
    namespace.initAudio = function(cfg) 
    {
        if (cfg)
            config = cfg;
        window.AudioContext = window.AudioContext || window.webkitAudioContext;
        try 
        {
            audioContext = new AudioContext();
        }
        catch(e) 
        {
            alert('Unable to create the Web Audio API.');
        }

        installSandH();

        volNode = audioContext.createGain();
        volNode.gain.value = cfg.volume;

        // Need a top level LFO too
        if (config.lfo.shape === "sandh")
        {
            lfoOsc = audioContext.createSandH();
        }
        else
        {
            lfoOsc = audioContext.createOscillator();
            lfoOsc.type = config.lfo.shape;
        }
        lfoOsc.frequency.setValueAtTime(config.lfo.freq, audioContext.currentTime);

        lfoAmpGain = audioContext.createGain();
        lfoOsc.connect(lfoAmpGain);
        lfoAmpGain.connect(volNode.gain);
        lfoAmpGain.gain.value = config.lfo.amp;

        lfoOscGain = audioContext.createGain();
        lfoOsc.connect(lfoOscGain);
        lfoOscGain.gain.value = config.lfo.pitch;

        lfoFilterGain = audioContext.createGain();
        lfoOsc.connect(lfoFilterGain);

        lfoOsc.start(0);

        // const compressor = audioContext.createDynamicsCompressor();
        // volNode.connect(compressor);
        // compressor.connect(audioContext.destination);
        
        volNode.connect(audioContext.destination);
        
        namespace.output = volNode;

        return audioContext;
    }

    function installSandH()
    {
        function createSampleAndHold()
        {
            const bufferSize = 4096;
            let samplecount = 0;
            let sample = Math.random() * 2 - 1;

            const rateGain = audioContext.createGain();
            rateGain.gain.value = 0.5;

            const node = audioContext.createScriptProcessor(bufferSize, 0, 1);
            node.onaudioprocess = function(e) 
            {
                const output = e.outputBuffer.getChannelData(0);
                const rate = rateGain.gain.value;
                const ratecount = rate == 0 ? 999999 : audioContext.sampleRate / rate;
                for (let i = 0; i < bufferSize; i++) 
                {
                    const val = Math.random() * 2 - 1;
                    if (++samplecount >= ratecount)
                    {
                        sample = val;
                        samplecount = 0;
                    }
                    output[i] = sample;
                }
            }

            node.frequency = rateGain.gain;

            node.start = function(...a) {};
            node.stop = function(...a) {};

            return node;
        }

        audioContext.createSandH = function()
        {
            return createSampleAndHold();
        }
    }
})(Synth);
