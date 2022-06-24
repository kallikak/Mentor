'use strict';

function choose(a)
{
    if (a.length)
        return a[Math.floor(Math.random() * a.length)];
    else
        return null;
}

const shuffle = function(values) 
{
    const size = values.length;
    for (let i = size - 1; i > 0; --i)
     {
        const j = Math.floor((i + 1) * Math.random());
        const swap = values[j];
        values[j] = values[i];
        values[i] = swap;
      }
      return values;
};

function fixButtonState()
{
    document.querySelectorAll("#controls")[0].querySelectorAll('button').forEach((e) => {
        if (e.id === "stopButton")
            e.classList.add("bluebuttonpressed")
        else
            e.classList.remove("bluebuttonpressed")
    });
}

function setButtonEnable(set)
{
    [...controls, ...testers].forEach(c => c.disabled = c.id !== "pitchSlider" && !set);
}

function makeTestArea(testtableid, testtextid)
{   
    const area = makeElement("span", "testarea", "testarea");

    area.append(makeElement("div", null, "header", "Test yourself &mdash; Match the patch"));

    const div = makeElement("div", null, "testcontainer", null);

    const tablehtml = '<tr id="playrow"><td>Play:</td></tr><tr id="checkrow"><td>Check:</td></tr>';
    const text = makeElement("div", testtextid, "testtext", null);
    const table = makeElement("table", testtableid, "testtable", tablehtml);

    div.append(text);
    div.append(table);
    area.append(div);

    const container = document.querySelector(".webaudio");

    container.append(document.createElement("hr"));
    container.append(area);
}

let controls;
let testers;

function setupTesters(controlsid, testtableid, testtextid, testpatches, 
    scorefn, testtext, context)
{
    makeTestArea(testtableid, testtextid);

    let showscope = false;

    controls = document.querySelector("#" + controlsid).getElementsByTagName('*');
    const testtable = document.querySelector("#" + testtableid);
    testers = testtable.getElementsByTagName('*');
    const testtextdiv = document.querySelector("#" + testtextid);
    const pitchSlider = document.querySelector('#pitchSlider');

    function playsample(sampleConfig, note, duration, context)
    {
        let saveconfig, v;
        Scope.setActive(showscope);
        setButtonEnable(false);
        if (context.alt)
            context.alt(sampleConfig, note, true);
        else
        {
            saveconfig = Config.copyConfig(context.config());
            Synth.setToConfig(sampleConfig);
            Synth.allNotesOff(true);
            if (note > 0)
                v = Synth.noteOn(note);
            else if (pitchSlider)
                v = Synth.noteOn(pitchSlider.value / 100);
            else
                v = Synth.noteOn(69);
        }
        setTimeout(() => {
            if (context.alt) 
            {
                context.alt(sampleConfig, note, false);
                Scope.setActive(false);
            }
            else
            {
                Synth.noteOff(v, false, () => {
                    Scope.setActive(false);
                    Synth.setToConfig(saveconfig);
                    fixButtonState();
                    setButtonEnable(true);
                });
            }
        }, duration);
    }
    
    function checksample(sampleConfig, btn)
    {
        btn.innerHTML = "&mdash;";
        setTimeout(() => btn.innerHTML = "", 150);
        setTimeout(() => btn.innerHTML = "&mdash;", 300);
        setTimeout(() => btn.innerHTML = "", 450);
        setTimeout(() => btn.innerHTML = scorefn(sampleConfig), 600);
    }

    if (testtext)
    {
        testtextdiv.innerHTML = testtext;
    }
    else
    {
        testtextdiv.innerHTML = 'Listen to each of the samples below, and try to match the sound ' + 
        'using the above controls. ' +
        'Click the corresponding <span class="emph" style="color:#007065;">Check</span> button to see how close you are.';
    }
    
    const playrow = testtable.rows[0];
    const testrow = testtable.rows[1];
    shuffle(testpatches).forEach((p,i) => {
        let btn = document.createElement("button");
        btn.setAttribute("class", "greenbutton");
        btn.innerHTML = "" + (i + 1);
        btn.addEventListener('click', () => {
            playsample(p.patch, p.note, p.duration, context);
        });
        let cell = document.createElement("td");
        cell.appendChild(btn);
        playrow.append(cell);

        btn = document.createElement("button");
        btn.setAttribute("class", "greenbutton");
        btn.innerHTML = "&mdash;";
        btn.addEventListener('click', () => {
            checksample(p.patch, btn);
        });
        cell = document.createElement("td");
        cell.appendChild(btn);
        testrow.append(cell);
    });

    const cell = document.createElement("td");
    const checkbox = document.createElement('input');
    checkbox.type = "checkbox";
    checkbox.name = "showscope";
    checkbox.value = "show";
    checkbox.id = "showscope";
    checkbox.addEventListener('change', () => showscope = checkbox.checked);
    const label = document.createElement('label')
    label.htmlFor = "showscope";
    label.appendChild(document.createTextNode('Show wave: '));    
    cell.appendChild(label);
    cell.appendChild(checkbox);
    playrow.append(cell);

    testrow.append(document.createElement("td"));
}
