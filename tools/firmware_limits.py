"""Original DP32G030 physical and Egzumer UV Tools image-size limits."""
FLASH_LIMIT = 60 * 1024
# https://github.com/egzumer/uvtools/blob/7148b01f11357cb28e74fff3a9591b12a656ea13/js/tool_patcher.js
# Its size warning, flash button and final flash guard all use 0xEFFF.
UPDATER_LIMIT = 0xEFFF

def check_image_size(size):
    if not 8192 <= size <= UPDATER_LIMIT:
        raise ValueError(f'raw firmware length {size} outside updater range 8192..{UPDATER_LIMIT}')
