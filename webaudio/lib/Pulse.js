'use strict';

const Pulse = {};

(function(namespace) { 

    // Pre-calculate the WaveShaper curves so that we can reuse them.
    const pulseCurve = new Float32Array(256);
    for (let i = 0; i < 128; i++) 
    {
        pulseCurve[i] = -1;
        pulseCurve[i + 128] = 1;
    }

    const constantOneCurve = new Float32Array(2);
    constantOneCurve[0] = 1;
    constantOneCurve[1] = 1;

    namespace.setupPulseSupport = function(audioContext) 
    {
        audioContext.createPulseOscillator = function() 
        {
            // Use a normal oscillator as the basis of our new oscillator.
            const node = audioContext.createOscillator();
            node.type = "sawtooth";
        
            // Shape the output into a pulse wave.
            const pulseShaper = audioContext.createWaveShaper();
            pulseShaper.curve = pulseCurve;
            node.connect(pulseShaper);
        
            // Use a GainNode as our new "width" audio parameter.
            const widthGain = audioContext.createGain();
            widthGain.gain.value = 0; // Default width.
            node.width = widthGain.gain; // Add parameter to oscillator node.
            widthGain.connect(pulseShaper);
        
            // Pass a constant value of 1 into the widthGain – so the "width" setting is
            // duplicated to its output.
            const constantOneShaper = audioContext.createWaveShaper();
            constantOneShaper.curve = constantOneCurve;
            node.connect(constantOneShaper);
            constantOneShaper.connect(widthGain);
        
            // Override the oscillator's "connect" method so that the new node's output
            // actually comes from the pulseShaper.
            node.connect = function() 
            {
                pulseShaper.connect.apply(pulseShaper, arguments);
                return node;
            }
        
            //Override the oscillator's "disconnect" method.
            node.disconnect = function()
            {
                pulseShaper.disconnect.apply(pulseShaper, arguments);
                return node;
            }
        
            return node;
        }
    }
})(Pulse);

// Based on https://github.com/skratchdot/web-audio-api-v2-issue-7/blob/master/public/pulse-oscillator.js

var PolyBlepPulse = {};

(function(namespace) { 
    
    namespace.setupPulseSupport = function(audioContext) 
    {
        const bitwiseOrZero = (t) => t | 0;
        const sqr = (x) => x * x;

        let freq = 0;
        let amplitude = 1;
        let t = 0;
        let freqInSecondsPerSample = 0;
        let sampleRate = audioContext.sampleRate;
        
        // Adapted from "Phaseshaping Oscillator Algorithms for Musical Sound
        // Synthesis" by Jari Kleimola, Victor Lazzarini, Joseph Timoney, and Vesa
        // Valimaki.
        // http://www.acoustics.hut.fi/publications/papers/smc2010-phaseshaping/
        const blep = (t, dt) => {
            if (t < dt) {
                return -sqr(t / dt - 1);
            } else if (t > 1 - dt) {
                return sqr((t - 1) / dt + 1);
            } else {
                return 0;
            }
        };

        const inc = () => {
            t += freqInSecondsPerSample;
            t -= bitwiseOrZero(t);
        };

        const getFreqInHz = () => {
            return freqInSecondsPerSample * sampleRate;
        };

        const sin = () => {
            return amplitude * Math.sin(TWOPI * t);
        };

        const saw = () => {
            let t1 = t + 0.5;
            t1 -= bitwiseOrZero(t1);
    
            let y = 2 * t1 - 1;
            y -= blep(t1, freqInSecondsPerSample);
    
            return this.amplitude * y;
        };

        const rect = (pulseWidth) => {
            let t2 = t + 1 - pulseWidth;
            t2 -= bitwiseOrZero(t2);
    
            let y = -2 * pulseWidth;
            if (t < pulseWidth)
                y += 2;
    
            y += blep(t, freqInSecondsPerSample) - blep(t2, freqInSecondsPerSample);
    
            return amplitude * y;
        };

        function createPulseWave(context, freq, detune, width)
        {
            const bufferSize = 512;
            // 3 input channels - frequency, detune, width
            const merger = context.createChannelMerger(3);
            freq.connect(merger, 0, 0);
            detune.connect(merger, 0, 1);
            width.connect(merger, 0, 2);
            
            const node = context.createScriptProcessor(bufferSize, 3, 1);
            node.onaudioprocess = function(e) 
            {
                const freqinput = e.inputBuffer.getChannelData(0);
                const detuneinput = e.inputBuffer.getChannelData(1);
                const widthinput = e.inputBuffer.getChannelData(2);
                const output = e.outputBuffer.getChannelData(0);
                
                let i = 0, min = 0, max = 0;
                // get our current param values
                const frequency = freqinput[i];
                const detune = detuneinput[i];
                const pulseWidth = widthinput[i];
                // calculate frequency
                const newf = Math.abs(frequency * Math.pow(2, detune / 1200));
                if (newf !== freq) 
                {
                    freq = newf;
                    freqInSecondsPerSample = freq / sampleRate;
                }
                for (i = 0; i < bufferSize; i++) 
                {
                    output[i] = getFreqInHz() >= sampleRate / 4 ? sin() : rect(pulseWidth);
                    if (output[i] > max)
                        max = output[i];
                    if (output[i] < min)
                        min = output[i];
                    inc();
                }
                const offset = (max + min) / 2;
                for (i = 0; i < bufferSize; ++i)
                    output[i] -= offset;
            }
            
            merger.connect(node);
            return node;
        }

        function createDCOffset() 
        {
            const buffer = audioContext.createBuffer(1, 2, audioContext.sampleRate);
            const data = buffer.getChannelData(0);
            for (let i = 0; i < 2; i++)
                data[i] = 1;
            const bufferSource = audioContext.createBufferSource();
            bufferSource.buffer = buffer;
            bufferSource.loop = true;
            return bufferSource;
        }

        audioContext.createPulseOscillator = function() 
        {
            let audioContext = this;
            
            const freqSource = createDCOffset();
            const freqAmount = audioContext.createGain();
            const detuneSource = createDCOffset();
            const detuneAmount = audioContext.createGain();
            const widthSource = createDCOffset();
            const widthAmount = audioContext.createGain();
            const output = audioContext.createGain();

            freqAmount.gain.value = 220;
            detuneAmount.gain.value = 0;
            widthAmount.gain.value = 0.5;
            
            freqSource.connect(freqAmount);
            detuneSource.connect(detuneAmount);
            widthSource.connect(widthAmount);

            const pulse = createPulseWave(audioContext, freqAmount, detuneAmount, widthAmount);
            pulse.connect(output);

            pulse.frequency = freqAmount.gain;
            pulse.detune = detuneAmount.gain;
            pulse.width = widthAmount.gain;

            pulse.connect = (...a) => output.connect.apply(output, a);
            pulse.disconnect = (...a) => output.disconnect.apply(output, a);

            pulse.start = t => {
                freqSource.start(t);
                detuneSource.start(t);
                widthSource.start(t);
            };

            pulse.stop = t => {
                widthSource.disconnect(t);
                detuneSource.disconnect(t);
                widthSource.disconnect(t);
                pulse.disconnect(t);
                pulse.onaudioprocess = null;
            };

            return pulse;
        }
    }
})(PolyBlepPulse);