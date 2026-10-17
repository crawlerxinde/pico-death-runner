# LVGL integration notes

Keep the vendor demo's display and input initialization. Implement a small `dr_draw_api_t`
adapter whose `clear`, `rect`, and `line` callbacks draw into a caller-owned RGB565 target
or LVGL canvas. The renderer is deliberately expressed in primitives so the LVGL 8/9 API
difference stays at this boundary.

For the first hardware integration, prefer a partial/strip draw target if the vendor's
flush path supports it. A full 640x172 RGB565 buffer is 220,160 bytes.
