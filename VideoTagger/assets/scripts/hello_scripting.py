import vt
import time


def on_run():
    print(type(vt.timeline.segment_count))
    print(vt.timeline)

    project = vt.current_project()
    if project is not None:
        print(project.name)

    time.sleep(5)
    help(vt)
