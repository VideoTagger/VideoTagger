import vt
import time


class hello_script(vt.Script):
    def has_progress(self: vt.Script) -> bool:
        return True

    def on_run(self: vt.Script) -> None:
        project = vt.current_project()
        if project is None:
            return

        print(type(vt.timeline.segment_count))
        print(vt.timeline)

        self.progress = 0.5
        self.progress_info = "Testing progress"
        print(project.name)

        time.sleep(5)
        self.progress = 1.0
        self.progress_info = ""
        help(vt)
