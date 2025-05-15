from .options import length_type, custom

# useful aliases

type list_d[TType, LType: length_type] = custom[list[TType], LType]
type str_d[T: length_type] = custom[str, T]
type bytes_d[T: length_type] = custom[bytes, T]

# option aliases
default_length_encoding = length_type
"""
Used for specifying the default type to use when reading a length-providing field.
Must be serializing an 'int' type.
"""

__all__ = (
    # type aliases
    "list_d",
    "str_d",
    "bytes_d",
    # option aliases
    "default_length_encoding",
)
