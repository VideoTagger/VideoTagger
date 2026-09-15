@page keybinds Keybinds

[TOC]

## Keybind Categories

Keybinds are split into two categories:
- `Application Keybinds`
- `Project Keybinds`

The main difference between them is the fact that `Application Keybinds` are shared between projects and `Project Keybinds` are stored in the project file.

## First Keybind

To add a keybind to your project, select `Tools > Options > Project Settings > Keybinds` from the menubar and press the `Add Keybind` button.

When the popup window opens you need to:
- Assign keybind's name
- Press your desired key combination
- Assign an action that should happen when the keybind is pressed

Some keybind actions have additional attributes, to learn more about them head over to [Keybind Actions](#keybind-actions)

<div class="warning">

There is no difference between `LeftAlt` and `RightAlt`, both are treated as a single `Alt` key - the same rule applies to `Ctrl` and `Shift`.

</div>

After configuring your keybind, press the `Save` button which will store the keybind in your project file.

### Keybind Actions

Keybinds support multiple actions, where each one of them can have multiple attributes. Available actions:
- None
- Insert Timestamp
    - `Tag Name`: Timestamp inserted with this action will have this tag name, unless you've selected `Ask Later`. Then a popup will appear prompting you to choose the tag you want to insert at the current timeline marker position, additionaly you can tweak the insertion point
- Start/End Segment
    - `Tag Name`: This attribute has the same behaviour as in `Insert Timestamp` action.
    - `Type`: This attribute represents what should happen when the keybind is pressed. It could be `Auto`, `Start` or `End` where:
        - `Auto` will start the segment if it wasn't started yet, or end it otherwise
        - `Start` will only start the segment if it wasn't started before
        - `End` will only end the segment if it was started before

![Keybind Actions Preview](/images/preview1.png)
