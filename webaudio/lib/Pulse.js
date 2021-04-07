'use strict';

var Pulse = {};

(function(namespace) { 
    
    namespace.setupPulseSupport = function(audioContext) 
    {
        function createPulseWave(context, saw, ramp, width)
        {
            var bufferSize = 256;
            // 3 input channels - the ramp, the saw, and the width
            var merger = context.createChannelMerger(4);
            saw.connect(merger, 0, 0);
            ramp.connect(merger, 0, 1);
            width.connect(merger, 0, 2);
            
            var node = context.createScriptProcessor(bufferSize, 4, 1);
            node.onaudioprocess = function(e) 
            {
                var sawinput = e.inputBuffer.getChannelData(0);
                var rampinput = e.inputBuffer.getChannelData(1);
                var widthinput = e.inputBuffer.getChannelData(2);
                var output = e.outputBuffer.getChannelData(0);
                
                var pw = widthinput[0];
                var shift = 1.7 * (0.5 - pw);
                for (var i = 0; i < bufferSize; i++) 
                {
                    output[i] = sawinput[i] + rampinput[i] + shift;
                }
            }
            
            merger.connect(node);
            return node;
        }

        // https://github.com/cwilso/web-audio-samples
        // https://github.com/cwilso/webaudiodemos/tree/master/oscilloscope
        audioContext.createPulseOscillator = function() 
        {
            let audioContext = this;
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
            
            const sawtooth = audioContext.createOscillator();
            sawtooth.type = "sawtooth";
            sawtooth.frequency.value = 0;

            const ramp = audioContext.createOscillator();
            ramp.type = "sawtooth";
            ramp.frequency.value = 0;
            const inverter = audioContext.createGain();
            inverter.gain.value = -1;
            ramp.connect(inverter);
            const delayNode = audioContext.createDelay();
            inverter.connect(delayNode);
            
            const inputDriver = audioContext.createConstantSource();
            const input = audioContext.createGain();
            input.gain.value = 220;
            inputDriver.connect(input);
            input.connect(sawtooth.frequency);
            input.connect(ramp.frequency);
            
            const output = audioContext.createGain();
            const widthSource = audioContext.createConstantSource();
            widthSource.start();
            const widthGain = audioContext.createGain();
            widthSource.connect(widthGain);
            const pulse = createPulseWave(audioContext, sawtooth, delayNode, widthGain);
            pulse.connect(output);

            input.frequency = input.gain;
            input.delay = delayNode.delayTime;
            input.width = widthGain.gain;

            input.connect = (...a) => output.connect.apply(output, a);
            input.disconnect = (...a) => output.disconnect.apply(output, a);

            input.start = t => {
                inputDriver.start(t);
                sawtooth.start(t);
                ramp.start(t);
            };

            input.stop = t => {
                inputDriver.stop(t);
                sawtooth.stop(t);
                ramp.stop(t);
            };

            return input;
        }
    }
})(Pulse);
