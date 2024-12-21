import vt
import time


class hello_script(vt.Script):
    def has_progress(self: vt.Script) -> bool:
        return True

    def on_run(self: vt.Script) -> None:
        project = vt.current_project()
        if project is None:
            return
        project.tags.clear()

        self.progress_info = "Printing info"
        print("Tags:")
        for tag in project.tags.list:
            print(f"name: {tag.name} color: {hex(tag.color).upper()}")

        print("Videos:")
        for vid in project.videos:
            print(f"id: {vid.id} path: {vid.path} size: {vid.size}")

        print(project.name)
        # help(vt)

        self.progress_info = "Adding tags"
        for i in range(200):
            self.progress = i / 200.0
            time.sleep(0.00125)
            tag = vt.Tag(f"Python Tag {i + 1}", 0xFF00FF + i * 100)
            project.tags.add_tag(tag)
        self.progress_info = ""
