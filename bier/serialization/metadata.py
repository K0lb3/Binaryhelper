from typing import Any

"""
Used for assigning metadata to a specific type node.
Can be added multiple times to a given TypeNode to add multiple metadata entries.
"""


def is_metadata_annotation(annotation: Any) -> bool:
    return isinstance(annotation, dict)


# this is done so you can prepend a space to your metadata values
# to make ruff emit a F722 error, which can be globally suppressed
# instead of the F841 error which might actually be useful
def _process_metadata_value(value: Any):
    if isinstance(value, str):
        return value.lstrip(" ")

    return value


def parse_metadata_annotation(annotation) -> dict[str, Any]:
    return {
        _process_metadata_value(x): _process_metadata_value(y)
        for x, y in annotation.items()
    }


__all__ = (
    "parse_metadata_annotation",
    "is_metadata_annotation",
)
