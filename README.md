# valorant aim assist

hey there this is a modern gui based aim assist tool i built for valorant using c plus plus and imgui it features a clean interface with directx 11 rendering and real time configuration

created by itzcooode

## important please read

look i need to be straight with you this tool is provided as is and im not responsible for what happens if you use it that includes
- getting banned from valorant this is very likely
- account suspensions or permanent bans
- any other consequences from anti cheat detection

this is purely an educational project to demonstrate pixel detection and gui programming in c plus plus if you choose to use it in game thats entirely on you i dont recommend it and im not liable for the results

youve been warned

## what does it do

this tool uses pixel detection to identify specific colors on your screen and automates mouse clicks when it finds them heres whats packed in

- smart color detection finds yellow red purple or custom colors you define
- two engine modes standard quick and vandal delayed timing for more natural behavior
- hold or auto mode activate with a button press or let it run continuously
- crouch automation optional crouch spray control
- movement lock stops movement keys when active
- real time gui adjust everything on the fly with sliders and buttons
- persistent config your settings save automatically

### available color presets

- yellow 0xffff00 default works well for most scenarios
- red 0xff0000 alternative detection color
- purple 0xfa64fa vibrant purple option
- custom set your own hex color for specific needs

## getting started

### quick build easiest way

just double click build gui bat and youre done it handles everything
- downloads imgui if you dont have it
- compiles the release x64 version
- copies the config file
- launches the app

## how to use it

### the gui

everything you need is right in the window
- status indicators show whats active green running yellow hold mode red off
- quick control buttons for instant mode switching
- sliders to tweak all settings in real time
- save load buttons to manage your configurations
- a disclaimer popup greets you on first launch because legal stuff matters

### keyboard shortcuts

- f1 hold mode only works when you press your configured buttons
- f2 auto mode always active no button needed
- f3 standard mode quick instant clicks
- f4 vandal mode delayed timing more human like
- f5 toggle crouch automation on off
- f6 toggle movement lock on off
- down arrow pause everything

## configuration

the app creates a config ini file on first run with sensible defaults you can tweak it directly or use the gui sliders

### what each setting does

```ini
boxSize=2              # How big the search area is (in pixels from center)
colorTolerance=80      # How close colors need to match (0-255, higher = more lenient)
targetColor=yellow     # Which color preset to use
customColor=0xFFFF00   # Your custom hex color (if using custom mode)
minDelay=2             # Minimum random delay in milliseconds
maxDelay=7             # Maximum random delay in milliseconds
sleepDelay=160         # How long to wait after each action
holdButton=5           # First activation button (5 = Mouse4)
holdButton2=18         # Second activation button (18 = Alt)
```

### common button codes

need to change your activation buttons here are the most common ones
- mouse4 side button 5
- mouse5 side button 6
- alt 18
- ctrl 17
- shift 16
- space 32

## gui features breakdown

the interface is designed to be intuitive and responsive

- live status display see exactly whats active at a glance
- one click controls big clear buttons for mode switching
- real time sliders adjust settings and see changes instantly
- color picker dropdown menu for quick color preset selection
- key name display shows friendly names next to vk codes no more guessing
- config management save and load your favorite setups
- no restart needed all changes apply immediately

## engine modes explained

### standard mode f3
quick and responsive sends mouse down and up immediately then handles crouch if enabled good for fast reactions

### vandal mode f4
more human like timing spreads out the mouse down crouch and mouse up actions with small delays between them looks more natural but slightly slower

pick whichever feels better for your playstyle

## system requirements

- windows 10 or 11
- directx 11 compatible gpu pretty much any modern graphics card

## troubleshooting

colors not detecting
- bump up the color tolerance try 100 150
- switch to a different color preset
- increase the box size for a larger search area

hold mode not working
- double check your button vk codes in the config
- make sure youre pressing the right buttons
- try using the gui to see the key names

## license and credits

this project is under the mit license check the license file for the full text

created by itzcooode  

## final words

this is an educational project to demonstrate c++ gui programming pixel detection and directx integration i built it to learn and share knowledge

using this in actual games will likely get you banned anti cheat systems are not a joke and they will catch you this is not a safe cheat its a learning tool

if youre here to learn about gui programming pixel detection or directx awesome dig into the code and see how it works if youre here to cheat in games well youve been warned about the consequences

stay safe code responsibly
