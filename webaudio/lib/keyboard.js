'use strict';

var Keyboard = {};

(function(context) { 

    context.installKeyboardHTML = function(containerId, keyboardId, callback)
    {
        const keymap = [
            { key:'Z', id:'key_48' }, { key:'S', id:'key_49' },
            { key:'X', id:'key_50' }, { key:'D', id:'key_51' },
            { key:'C', id:'key_52' }, 
            { key:'V', id:'key_53' }, { key:'G', id:'key_54' },
            { key:'B', id:'key_55' }, { key:'H', id:'key_56' },
            { key:'N', id:'key_57' }, { key:'J', id:'key_58' },
            { key:'M', id:'key_59' }, 
            { key:'E', id:'key_60' }, { key:'4', id:'key_61' },
            { key:'R', id:'key_62' }, { key:'5', id:'key_63' },
            { key:'T', id:'key_64' }, 
            { key:'Y', id:'key_65' }, { key:'7', id:'key_66' },
            { key:'U', id:'key_67' }, { key:'8', id:'key_68' },
            { key:'I', id:'key_69' }, { key:'9', id:'key_70' },
            { key:'O', id:'key_71' }, 
            { key:'P', id:'key_72' }
        ];
        const container = document.getElementById(containerId);
        
        function managekey(keyid, keyel, play)
        {
            callback(Number(keyid.substr(4)), play);
            if (!keyel)
                keyel = document.getElementById(keyid);
            if (play)
                keyel.classList.add('activekey');
            else
                keyel.classList.remove('activekey');
        }

        let curkey = null;
        document.addEventListener("keydown", function(e) {
            if (e.repeat)
                return;
            if (curkey && callback)
                managekey(curkey.id, null, false);
            const key = keymap.find(k => k.key == e.key.toUpperCase());
            if (key) {
                curkey = key;
                const keyid = key.id;
                if (callback)
                    managekey(curkey.id, null, true);
            }
        });
        document.addEventListener("keyup", function(e) {
            const key = keymap.find(k => k.key == e.key.toUpperCase());
            if (key) {
                if (curkey && key.id === curkey.id)
                {
                    if (callback)
                        managekey(curkey.id, null, false);
                    curkey = null;
                }
            }
        });
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
                    managekey(e.target.id, e.target, true);
                };
                x[i].onpointerup = (e) => {
                    managekey(e.target.id, e.target, false);
                };
                x[i].onpointerleave = (e) => { 
                    if (mousedown) {
                        managekey(e.target.id, e.target, false);
                    }
                };
                if (candrag)
                {
                    x[i].onpointerenter = (e) => { 
                        if (mousedown) {
                            managekey(e.target.id, e.target, true);
                        }
                    };
                }
            }
        }
    }

})(Keyboard);  