'use strict';

var Keyboard = {};

(function(context) { 

    context.installKeyboardHTML = function(containerId, keyboardId, callback)
    {
        const container = document.getElementById(containerId);
        const kbdDiv = document.createElement('div');
        kbdDiv.setAttribute("id", keyboardId);
        kbdDiv.innerHTML = '<div id="${keyboardId}" class="keyboard-holder">' +
            '<div class="white key" id="key_48" style="left: 0px;"></div>' +
            '<div class="black key" id="key_49" style="left: 22px;"></div>' +
            '<div class="white key" id="key_50" style="left: 35px;"></div>' +
            '<div class="black key" id="key_51" style="left: 57px;"></div>' +
            '<div class="white key" id="key_52" style="left: 69px;"></div>' +
            '<div class="white key" id="key_53" style="left: 103px;"></div>' +
            '<div class="black key" id="key_54" style="left: 126px;"></div>' +
            '<div class="white key" id="key_55" style="left: 138px;"></div>' +
            '<div class="black key" id="key_56" style="left: 161px;"></div>' +
            '<div class="white key" id="key_57" style="left: 172px;"></div>' +
            '<div class="black key" id="key_58" style="left: 195px;"></div>' +
            '<div class="white key" id="key_59" style="left: 207px;"></div>' +
            '<div class="white key" id="key_60" style="left: 242px;"></div>' +
            '<div class="black key" id="key_61" style="left: 264px;"></div>' +
            '<div class="white key" id="key_62" style="left: 276px;"></div>' +
            '<div class="black key" id="key_63" style="left: 299px;"></div>' +
            '<div class="white key" id="key_64" style="left: 310px;"></div>' +
            '<div class="white key" id="key_65" style="left: 345px;"></div>' +
            '<div class="black key" id="key_66" style="left: 367px;"></div>' +
            '<div class="white key" id="key_67" style="left: 379px;"></div>' +
            '<div class="black key" id="key_68" style="left: 402px;"></div>' +
            '<div class="white key" id="key_69" style="left: 414px;"></div>' +
            '<div class="black key" id="key_70" style="left: 436px;"></div>' +
            '<div class="white key" id="key_71" style="left: 448px;"></div>' +
            '<div class="white key" id="key_72" style="left: 483px;"></div>';
        container.appendChild(kbdDiv);
        
        // now add the callbacks
        let mousedown = false;
        let candrag = true;
        if (callback)
        {
            var x = container.getElementsByClassName("key");
            container.onpointerdown = () => { 
                mousedown = true;
            }
            container.onpointerup = () => { 
                mousedown = false;
            }
            for (var i = 0; i < x.length; i++) 
            {
                x[i].onpointerdown = (e) => {
                    callback(Number(e.target.id.substr(4)), true);
                    e.target.classList.add('activekey');
                };
                x[i].onpointerup = (e) => {
                    callback(Number(e.target.id.substr(4)), false);
                    e.target.classList.remove('activekey');
                };
                x[i].onpointerleave = (e) => { 
                    if (mousedown) {
                        callback(Number(e.target.id.substr(4)), false); 
                        e.target.classList.remove('activekey');
                    }
                };
                if (candrag)
                {
                    x[i].onpointerenter = (e) => { 
                        if (mousedown) {
                            callback(Number(e.target.id.substr(4)), true); 
                            e.target.classList.add('activekey');
                        }
                    };
                }
            }
        }
    }

})(Keyboard);  