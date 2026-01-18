# FFFT - FFT Library

This directory contains the FFFT library, a free FFT library by Laurent de Soras.

## License

FFFT is released under the WTFPL (Do What The F*ck You Want To Public License).

## Original Source

The library was originally obtained from: http://ldesoras.free.fr/prod.html

## Usage

This is a header-only C++ template library for computing Fast Fourier Transforms.

Include the appropriate headers:

```cpp
#include <ffft/FFTReal.h>
// or for fixed-length FFT:
#include <ffft/FFTRealFixLen.h>
```

## Files

- `FFTReal.h/hpp` - Main FFT class (variable length)
- `FFTRealFixLen.h/hpp` - Fixed-length FFT (compile-time size)
- `Array.h/hpp` - Array utilities
- `DynArray.h/hpp` - Dynamic array utilities
- `OscSinCos.h/hpp` - Sine/cosine oscillator
- Supporting headers for FFT passes and parameter handling
