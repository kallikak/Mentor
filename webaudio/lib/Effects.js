'use strict';

const Noise = {};

(function(namespace) { 
    
    namespace.WHITE_NOISE = 0;
    namespace.PINK_NOISE = 1;
    namespace.BROWN_NOISE = 2;
    namespace.BLUE_NOISE = 3;
    namespace.NOISE_NAMES = ["White", "Pink", "Brown", "Blue"];

    // thanks to https://noisehack.com/generate-noise-web-audio-api/
    namespace.createNoise = function(noisetype, audioContext)
    {
        var bufferSize = 256;
        var noiseGen = audioContext.createScriptProcessor(bufferSize, 0, 1);
        noiseGen.onaudioprocess = function(e) 
        {
            const output = e.outputBuffer.getChannelData(0);

            let b0, b1, b2, b3, b4, b5, b6;
            b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.0;

            let a1, a2;
            if (noisetype == namespace.BLUE_NOISE)
            {
                // high pass butterworth filter at 3000Hz
                const f = 2000;
                const Q = 1;
                const w0 = 2 * Math.PI * f / audioContext.sampleRate;
                const cosw0 = Math.cos(w0);
                const alpha = Math.sin(w0) / (2 * Q);

                const a0 =   1 + alpha;
                b0 =  (1 + cosw0) / 2 / a0;
                b1 = -(1 + cosw0) / a0;
                b2 = b0;
                a1 =  (-2 * cosw0) / a0;
                a2 =  (1 - alpha) / a0;
            }
            let lastlastWhite = 0;
            let lastWhite = 0;
            for (let i = 0; i < bufferSize; i++) 
            {
                const white = Math.random() * 2 - 1;
                if (noisetype == namespace.PINK_NOISE)
                {
                    b0 = 0.99886 * b0 + white * 0.0555179;
                    b1 = 0.99332 * b1 + white * 0.0750759;
                    b2 = 0.96900 * b2 + white * 0.1538520;
                    b3 = 0.86650 * b3 + white * 0.3104856;
                    b4 = 0.55000 * b4 + white * 0.5329522;
                    b5 = -0.7616 * b5 - white * 0.0168980;
                    output[i] = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362;
                    output[i] *= 0.11; // (roughly) compensate for gain
                    b6 = white * 0.115926;
                }
                else if (noisetype == namespace.BROWN_NOISE)
                {
                    output[i] = (b0 + (0.02 * white)) / 1.02;
                    b0 = output[i];
                    output[i] *= 3.5; // (roughly) compensate for gain
                }
                // else if (i > 2 && noisetype == namespace.BLUE_NOISE)
                // {
                //     const c = Math.tan(Math.PI * 5000 / audioContext.sampleRate);
                //     var a1 = 1 / (1 + Math.SQRT2 * c + c * c);
                //     var a2 = -2 * a1;
                //     var a3 = a1;
                //     b1 = 2. * (c * c - 1) * a1;
                //     b2 = (1 - Math.SQRT2 * c + c * c) * a1;
                //     output[i] = a1 * white + a2 * lastWhite + a3 * lastlastWhite - 
                //         b1 * output[i - 1] - b2 * output[i - 2];
                // }
                else if (i > 2 && noisetype == namespace.BLUE_NOISE)
                {
                    output[i] = b0 * white + b1 * lastWhite + b2 * lastlastWhite
                        - a1 * output[i - 1] - a2 * output[i - 2];
                }
                else // if (noisetype == BLUE_NOISE) we use white plus highpass filter at 3kHz
                    output[i] = white;
                lastlastWhite = lastWhite;
                lastWhite = white;
            }
        }

        return noiseGen;
    }
})(Noise);

var Reverb = {};

(function(namespace) {
     
    namespace.connectReverb = function(audioContext, synthOutput, duration = 4, decay = 4)
    {
        function impulseResponse(duration, decay, reverse) 
        {
            duration = Math.max(duration, 1);
            var sampleRate = audioContext.sampleRate;
            var length = sampleRate * duration;
            var impulse = audioContext.createBuffer(2, length, sampleRate);
            var impulseL = impulse.getChannelData(0);
            var impulseR = impulse.getChannelData(1);
    
            if (!decay)
                decay = 2.0;
            for (var i = 0; i < length; i++)
            {
                var n = reverse ? length - i : i;
                impulseL[i] = (Math.random() * 2 - 1) * Math.pow(1 - n / length, decay);
                impulseR[i] = (Math.random() * 2 - 1) * Math.pow(1 - n / length, decay);
            }
            return impulse;
        }
    
        function updateImpulseBuffer(duration, decay)
        {
            const impulseBuffer = impulseResponse(duration, decay, false);
            convolver.buffer = impulseBuffer;
        }

        var convolver = audioContext.createConvolver();

        updateImpulseBuffer(duration, decay);

        const reverbLevel = audioContext.createGain();
        const dryLevel = audioContext.createGain();
        synthOutput.disconnect();
        synthOutput.connect(convolver);
        synthOutput.connect(dryLevel);
        reverbLevel.gain.value = 0.75;
        dryLevel.gain.value = 1 - reverbLevel.gain.value;
        convolver.connect(reverbLevel);
        reverbLevel.connect(audioContext.destination);
        dryLevel.connect(audioContext.destination);

        convolver.setGain = function(u)
        {
            reverbLevel.gain.value = u;
            dryLevel.gain.value = 1 - u;
        }

        return convolver;
    }
})(Reverb);

var Delay = {};

(function(namespace) {
     
    namespace.connectDelay = function(audioContext, synthOutput)
    {
        const delay = audioContext.createDelay();
        delay.delayTime.value = 0.2;
        const feedback = audioContext.createGain();
        feedback.gain.value = 0.3;
        
        synthOutput.connect(delay);
        delay.connect(feedback);
        feedback.connect(delay);
        feedback.connect(audioContext.destination);

        delay.feedback = feedback;

        return delay;
    }
})(Delay);

var Chorus = {};

(function(namespace) {
     
    namespace.connectChorus = function(audioContext, inputNode, outputNode)
    {
        const chorus = audioContext.createGain();
    
        let delay = audioContext.createDelay();
        let speed = audioContext.createOscillator();
        let depth = audioContext.createGain();
    
        delay.delayTime.value = 0.035;   // 0.005 to 0.055
        depth.gain.value = 0.001;   // 0.0005 to 0.004:
        speed.type = 'sine';
        speed.frequency.value = 1.5;  // 0.5 to 15
    
        speed.connect(depth);
        depth.connect(delay.delayTime);
    
        chorus.connect(outputNode);
        chorus.connect(delay);
        delay.connect(outputNode);
    
        speed.start(0);
    
        chorus.setSpeed = u => speed.frequency.setValueAtTime(0.5 + 4.5 * u, audioContext.currentTime);
        chorus.setDepth = u => depth.gain.value = u * 0.004;
        chorus.delay = delay;
        chorus.stop = () => {
            speed.stop();
            speed.disconnect();
            delay.disconnect();
            depth.disconnect();
            chorus.disconnect();
        }
        
        if (inputNode)
            inputNode.connect(chorus);
            
        return chorus;
    }
})(Chorus);

var Flanger = {};

(function(namespace) {
     
    namespace.connectFlanger = function(audioContext, inputNode, outputNode)
    {
        const flanger = audioContext.createGain();
    
        let delay = audioContext.createDelay();
        let speed = audioContext.createOscillator();
        let depth = audioContext.createGain();
        let feedback = audioContext.createGain();
    
        delay.delayTime.value = 0.005;   // 0.001 to 0.02
        depth.gain.value = 0.002;   // 0.0005 to 0.005:
        feedback.gain.value = 0.5;  // 0 to 1
        speed.type = 'sine';
        speed.frequency.value = 0.25;  // 0.05 to 5
    
        speed.connect(depth);
        depth.connect(delay.delayTime);
    
        flanger.connect(outputNode);
        flanger.connect(delay);
        delay.connect(outputNode);
        delay.connect(feedback);
        feedback.connect(flanger);
    
        speed.start(0);
    
        flanger.setSpeed = u => speed.frequency.setValueAtTime(0.05 + 2.45 * u, audioContext.currentTime);
        // flanger.setSpeed = u => speed.frequency.value = 0.05 + 2.45 * u;
        flanger.setDepth = u => depth.gain.value = u * 0.005;
        flanger.delay = delay;
        flanger.feedback = feedback;
        flanger.stop = () => {
            speed.stop();
            speed.disconnect();
            delay.disconnect();
            depth.disconnect();
            feedback.disconnect();
            flanger.disconnect();
        }

        if (inputNode)
            inputNode.connect(flanger);
    
        return flanger;
    }
})(Flanger);

var Phaser = {};

(function(namespace) {
     
    namespace.connectPhaser = function(audioContext, inputNode, outputNode)
    {
        const phaser = audioContext.createGain();
    
        let speed = audioContext.createOscillator();
        let depth = audioContext.createGain();
        let feedback = audioContext.createGain();
        let lowpass = audioContext.createBiquadFilter();
    
        let filters = [];
        let i;
    
        depth.gain.value = 25;
        feedback.gain.value = 0.75;  // 0 to 1
        speed.type = 'sine';
        speed.frequency.value = 2.5;  // 0.05 to 5
        speed.amplitude = 1;
        lowpass.frequency.value = 5000;
    
        speed.connect(depth);
        for (i = 0; i < 4; ++i)
        {
            filters[i] = audioContext.createBiquadFilter();
            filters[i].type = "allpass";
            if (i)
                filters[i - 1].connect(filters[i]);
            depth.connect(filters[i].frequency);
        }
    
        phaser.connect(outputNode);
        phaser.connect(filters[0]);
        filters[3].connect(lowpass);
        lowpass.connect(outputNode);
        filters[3].connect(feedback);
        feedback.connect(phaser);
    
        speed.start(0);
    
        phaser.setSpeed = u => speed.frequency.setValueAtTime(0.05 + 4.5 * u, audioContext.currentTime);
        // phaser.setSpeed = u => speed.frequency.value = 0.05 + 4.5 * u;
        phaser.setDepth = u => depth.gain.value = u * 100;
        phaser.speed = speed;
        phaser.depth = depth;
        phaser.feedback = feedback;
        phaser.stop = () => {
            speed.stop();
            speed.disconnect();
            for (i = 0; i < 4; ++i)
                filters[i].disconnect();
            depth.disconnect();
            feedback.disconnect();
            phaser.disconnect();
        }

        if (inputNode)
            inputNode.connect(phaser);
    
        return phaser;
    }
})(Phaser);
