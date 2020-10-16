# PLAIN GRAPHICS LIBRARY 3

## Synopsis
This is a vector graphics library that is modeled after the PostScript/PDF
imaging model.

# TODO
- Queued
  - Secure UTF-8
  - Kerning
  - OpenType substitutions
  - Fill & Stroke without overflap
  - Radial fill
- Planned
  - Re-evaluate flatness metric
  - Beveled line joins
  - Miter limit
  - OpenType Windows/Symbol CMAP
  - Unicode glyphs outside of BMP
  - SVG Fonts
- Optimisation
  - FMA for matrix multiplication
  - Find a tighter bound on number of vertexes in OpenGL flattening
  - Share flattened vertexes in OpenGL `_strokefill()`
  - Add a OpenGL 3 geometry shader to flatten curves
  - Possibly use Loop-Blinn to render curves
- PLANNED INCOMPATIBLITY
  - PostScript/PDF
    - No round line caps or round line joins
    - Line dashing
  - TrueType/OpenType
    - "Matching Point" composite glyphs not supported
    - Variable fonts
    - CFF CID-keyed fonts and FDSelect (Source Han Sans - JP)
    - Type2 CharStrings with 2-byte operators (computed and dynamic glyphs)
- ROBUSTNESS / SECURITY
  - Check float to integer conversion
    - `(unsigned) negativefloat` is implementation defined
  - Consistent style of unsigned usage
  - Values are cached such as the size of tables.
    If the underlying file changes, the results could cause exceptions.
