import videotagger as vt
import time


class test_long(vt.Script):
    def has_progress(self: vt.Script) -> bool:
        return True

    def on_run(self: vt.Script) -> None:
        project = vt.current_project()
        if project is None:
            return
        self.progress_info = "Sleeping"

        time_seconds = 10
        for i in range(time_seconds):
            self.progress = i / float(time_seconds)
            time.sleep(1.0)
        self.progress_info = ""
