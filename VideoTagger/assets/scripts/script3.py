import vt


def on_run():
    tag = vt.tags.add(f"Tag {vt.tags.size + 1}")
    vt.timeline.add_timepoint(tag, vt.player.current_time)
