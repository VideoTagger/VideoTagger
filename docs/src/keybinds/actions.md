# Keybind Actions

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

![Keybind Actions Preview](./images/preview1.png)