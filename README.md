# BInaryhelpER

A small python package to make binary serialization faster and easier.

modules:

- EndianedBinaryIO - for parsing and writing streamed binary data.
- ComplexStreams (TODO)
  - BlockStream - for more performant handling of e.g. streams consisting of compressed and/or encrypted blocks
  - MultiStream - for handling concatent streams as single streams
- struct (TODO) - extrend struct module with additional features
  - c - null terminated string
  - v - varint
  - () - tuples (groups)
  - x\*y - read an x, then read x times y (e.g. i*s for strings with their lengths described with a prior int)
- Serialization (TODO) - annotation based class serialization  