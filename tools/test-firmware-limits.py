#!/usr/bin/env python3
"""Regression: a physically fitting 61,440-byte image is rejected by UV Tools."""
from firmware_limits import check_image_size
for size in [8192,8193,61432,61438,61439]:
    check_image_size(size)
for size in [0,8191,61440,61441,65536]:
    try:
        check_image_size(size)
    except ValueError:
        pass
    else:
        raise AssertionError(f'accepted incompatible firmware size {size}')
print('PASS updater-size regression: 61,439 accepted; 61,440 rejected before packaging')
