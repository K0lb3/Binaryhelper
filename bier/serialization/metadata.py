from typing import Any, Literal, get_args, Annotated
from ._typing_helpers import get_origin_type


# NOTE: We might be able to port this to the same way custom is defined,
# but as it stands right now this causes issues in type checkers with the literal arguments
class metadata:
    def __class_getitem__(cls, params):
        assert len(params) == 2, "metadata must have exactly two arguments"
        return Annotated[None, "__bier_metadata__", *params]


"""
Used for assigning metadata to a specific type node.
Can be added multiple times to a given TypeNode to add multiple metadata entries.
"""


def is_metadata_annotation(annotation) -> bool:
    return (
        get_origin_type(annotation) is Annotated
        and len(args := get_args(annotation)) > 1
        and args[1] == "__bier_metadata__"
    )


def parse_metadata_annotation(annotation) -> tuple[str, Any]:
    # the metadata class gets lowered to an Annotated class for None with a magic first argument (__bier_metadata__)

    # skip the first arg, which is NoneType, and the second arg, which has the magic string
    metadata_args = get_args(annotation)
    # assert that arg0 is NoneType
    assert metadata_args[1] == "__bier_metadata__", (
        f"invalid metadata magic argument, got {metadata_args[1]}"
    )

    metadata_args = metadata_args[2:]
    assert len(metadata_args) == 2, (
        f"metadata must have exactly two arguments, got {len(metadata_args)}"
    )

    metadata_key, metadata_value = metadata_args
    if get_origin_type(metadata_key) == Literal:
        metadata_key_value = get_args(metadata_key)[0]
    else:
        metadata_key_value = metadata_key

    assert isinstance(metadata_key_value, str), (
        f"First argument of metadata args must be a literal string, got {type(metadata_key_value)}"
    )

    if get_origin_type(metadata_value) == Literal:
        metadata_value_value = get_args(metadata_value)[0]
    else:
        metadata_value_value = metadata_value

    return metadata_key_value, metadata_value_value


__all__ = (
    "metadata",
    "parse_metadata_annotation",
    "is_metadata_annotation",
)
