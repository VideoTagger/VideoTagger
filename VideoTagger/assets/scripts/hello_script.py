import vt
# import time


class hello_script(vt.Script):
    def has_progress(self: vt.Script) -> bool:
        return True

    def on_run(self: vt.Script) -> None:
        print(type(vt.timeline.segment_count))

        print(vt.timeline)

        project = vt.current_project()
        if project is not None:
            print(project.name)

        # time.sleep(5)
        help(vt)
