from typing import Any, get_origin


def resolve_genericalias(origin: type, args: tuple[Any, ...]):
    """
    Remap a GenericAlias instance to its underlying __value__ GenericAlias,
    preserving the correct parameter mapping.
    """
    src_parameters = getattr(origin, "__parameters__", ())
    dst_generic = getattr(origin, "__value__", None)
    dst_parameters = getattr(dst_generic, "__parameters__", ())

    if not dst_generic:
        raise TypeError(f"Origin {origin} does not define __value__, cannot resolve.")

    if len(src_parameters) != len(dst_parameters):
        raise ValueError(
            f"Generic type {origin} has {len(src_parameters)} parameters, "
            f"but {len(dst_parameters)} were expected."
        )

    # Fast path: identical parameter order
    if src_parameters == dst_parameters:
        return dst_generic[args]

    # Build mapping: src_param → arg
    param_to_arg = dict(zip(src_parameters, args))

    # Remap args to dst_parameter order
    try:
        reordered_args = tuple(param_to_arg[dst_param] for dst_param in dst_parameters)
    except KeyError as e:
        raise ValueError(
            f"Parameter {e.args[0]} in destination generic is missing in source parameters."
        ) from None

    return dst_generic[reordered_args]


def get_origin_type(cls: type) -> type:
    return get_origin(cls) or cls


__all__ = ("resolve_genericalias", "get_origin_type")
