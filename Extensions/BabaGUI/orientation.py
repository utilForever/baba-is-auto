import pyBaba


_ROTATIONS = {
    pyBaba.Direction.RIGHT: 0,
    pyBaba.Direction.UP: 90,
    pyBaba.Direction.LEFT: 180,
    pyBaba.Direction.DOWN: -90,
}


def rotation_for_direction(direction):
    return _ROTATIONS[direction]


def oriented_instances(objects):
    for instance in objects.GetInstances():
        if instance.type == pyBaba.ObjectType.ICON_EMPTY:
            continue

        rotation = (
            rotation_for_direction(instance.direction)
            if instance.type == pyBaba.ObjectType.ICON_KEKE
            else 0
        )

        yield instance, rotation
