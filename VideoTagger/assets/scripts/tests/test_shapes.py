import videotagger as vt
import numpy as np


class test_shapes(vt.Script):
    def __init__(self):
        vt.Script.__init__(self)

    def has_progress(self: vt.Script) -> bool:
        return False

    def on_run(self) -> None:
        self.test_circle()
        self.test_rectangle()
        self.test_points()
        self.test_polygon()
        self.test_line()
        self.test_mask()

    def test_circle(self):
        circle = vt.shapes.Circle(vt.Vec2(10, 20), 30)

        assert circle.pos == vt.Vec2(10, 20)
        assert circle.radius == 30

        circle.pos = vt.Vec2(50, 60)
        assert circle.pos == vt.Vec2(50, 60)

        circle.radius = 100
        assert circle.radius == 100

        same = vt.shapes.Circle(vt.Vec2(50, 60), 100)
        different = vt.shapes.Circle(vt.Vec2(0, 0), 10)

        assert circle == same
        assert circle != different

        print(f"Circle [pos: {circle.pos}, radius: {circle.radius}]")

    def test_rectangle(self):
        rectangle = vt.shapes.Rectangle(
            vt.Vec2(10, 20),
            vt.Vec2(100, 200)
        )

        assert rectangle.start == vt.Vec2(10, 20)
        assert rectangle.end == vt.Vec2(100, 200)

        rectangle.start = vt.Vec2(30, 40)
        rectangle.end = vt.Vec2(300, 400)

        assert rectangle.start == vt.Vec2(30, 40)
        assert rectangle.end == vt.Vec2(300, 400)

        same = vt.shapes.Rectangle(
            vt.Vec2(30, 40),
            vt.Vec2(300, 400)
        )

        different = vt.shapes.Rectangle(
            vt.Vec2(0, 0),
            vt.Vec2(100, 100)
        )

        assert rectangle == same
        assert rectangle != different

        print(f"Rectangle [start: {rectangle.start}, end: {rectangle.end}]")

    def test_points(self):
        points = [
            vt.Vec2(10, 20),
            vt.Vec2(30, 40),
            vt.Vec2(50, 60)
        ]

        shape = vt.shapes.Points(points)

        assert shape.points == points

        shape.points = [
            vt.Vec2(100, 200),
            vt.Vec2(300, 400)
        ]

        assert shape.points == [
            vt.Vec2(100, 200),
            vt.Vec2(300, 400)
        ]

        same = vt.shapes.Points(shape.points)
        different = vt.shapes.Points([vt.Vec2(0, 0)])

        assert shape == same
        assert shape != different

        print(f"Points [points: {shape.points}]")

    def test_polygon(self):
        points = [
            vt.Vec2(0, 0),
            vt.Vec2(100, 0),
            vt.Vec2(100, 100),
            vt.Vec2(0, 100)
        ]

        shape = vt.shapes.Polygon(points)

        assert shape.points == points

        shape.points = [
            vt.Vec2(10, 10),
            vt.Vec2(200, 10),
            vt.Vec2(200, 200)
        ]

        assert len(shape.points) == 3

        same = vt.shapes.Polygon(shape.points)
        different = vt.shapes.Polygon([vt.Vec2(0, 0)])

        assert shape == same
        assert shape != different

        print(f"Polygon [points: {shape.points}]")

    def test_line(self):
        points = [
            vt.Vec2(10, 10),
            vt.Vec2(50, 50)
        ]

        shape = vt.shapes.Line(points)

        assert shape.points == points

        shape.points = [
            vt.Vec2(0, 0),
            vt.Vec2(100, 100),
            vt.Vec2(200, 200)
        ]

        assert len(shape.points) == 3

        same = vt.shapes.Line(shape.points)
        different = vt.shapes.Line([vt.Vec2(0, 0)])

        assert shape == same
        assert shape != different

        print(f"Line [points: {shape.points}]")

    def test_mask(self):
        data = np.array([
            [0, 255, 0],
            [255, 255, 0],
            [0, 0, 255]
        ], dtype=np.uint8)

        mask = vt.shapes.Mask(data)
        result = np.asarray(mask, copy=False)

        assert result.shape == (3, 3)
        assert result.dtype == np.uint8
        assert np.array_equal(result, data)

        result[0, 0] = 128
        assert data[0, 0] == 128

        same = vt.shapes.Mask(data.copy())
        assert mask == same

        different_data = np.zeros((3, 3), dtype=np.uint8)
        different = vt.shapes.Mask(different_data)

        assert mask != different

        print(f"Mask [shape: {result.shape}, dtype: {result.dtype}], data: {result}")
